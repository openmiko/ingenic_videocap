#define _GNU_SOURCE

#include "log.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>

uint8_t *buffer;

static int xioctl(int fd, int request, void *arg)
{
	int r;

	do r = ioctl (fd, request, arg);
	while (-1 == r && EINTR == errno);

	return r;
}

int print_caps(int fd)
{
	struct v4l2_capability caps = {};
	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &caps)) {
		perror("Querying Capabilities");
		return 1;
	}

	log_info(
		"Driver Caps:\n"
		"\tDriver: \"%s\"\n"
		"\tCard: \"%s\"\n"
		"\tBus: \"%s\"\n"
		"\tVersion: %d.%d\n"
		"\tCapabilities: %08x\n",
		caps.driver,
		caps.card,
		caps.bus_info,
		(caps.version>>16)&&0xff,
		(caps.version>>24)&&0xff,
		caps.capabilities
	);

	char fourcc[5] = {0};

	struct v4l2_format fmt = {0};
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = 1920;
	fmt.fmt.pix.height = 1080;
	
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;
	fmt.fmt.pix.colorspace = V4L2_COLORSPACE_JPEG;

	if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) {
		perror("Setting Pixel Format");
		return 1;
	}

	strncpy(fourcc, (char *)&fmt.fmt.pix.pixelformat, 4);
	log_info(
		"Selected Camera Mode:\n"
		"\tWidth: %d\n"
		"\tHeight: %d\n"
		"\tPixFmt: %s\n"
		"\tField: %d\n",
		fmt.fmt.pix.width,
		fmt.fmt.pix.height,
		fourcc,
		fmt.fmt.pix.field
	);
	
	return 0;
}

int init_mmap(int fd)
{
	struct v4l2_requestbuffers req = {0};
	req.count = 1;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		perror("Requesting Buffer");
		return 1;
	}

	struct v4l2_buffer buf = {0};
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = 0;
	if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf)) {
		perror("Querying Buffer");
		return 1;
	}

	buffer = mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
	log_info("Length: %d\nAddress: %p\n", buf.length, buffer);
	log_info("Image Length: %d\n", buf.bytesused);

	return 0;
}

int capture_image(int fd, char *output_mode)
{
	int ret;

	struct v4l2_buffer buf = {0};
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = 0;

	if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
		log_info("Query Buffer");
		return 1;
	}

	if (-1 == xioctl(fd, VIDIOC_STREAMON, &buf.type)) {
		log_info("Start Capture");
		return 1;
	}

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	struct timeval tv = {0};
	tv.tv_sec = 2;
	int r = select(fd+1, &fds, NULL, NULL, &tv);
	if (-1 == r) {
		log_info("Waiting for Frame");
		return 1;
	}

	if(-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
		log_info("Retrieving Frame");
		return 1;
	}

	// When retrieving the frame it will return the full number of bytes
	// even though it is a JPEG image. So look for the end of the JPEG marker
	// and only write out those bytes

	FILE * fp;
	fp = fopen("/tmp/out.jpg", "wb");

	char eof_jpeg_marker[2] = {0xff, 0xd9};

	char *last_needle = NULL;

	char *eof = (char *)memmem(buffer, buf.bytesused, eof_jpeg_marker, 2);
	if(eof == NULL) {
		log_error("Error: Unable to find end of JPEG marker in retrieved frame.");
		return 1;
	}

	int jpeg_bytes = eof - (char *)buffer;
	log_info("Calculated size of JPEG as %d\n", jpeg_bytes);
	fwrite((void *)buffer, jpeg_bytes+2, 1, fp);
	fclose(fp);

	log_info("Image saved to: /tmp/out.jpg");

	char content_type_mixed_replace[] = "Content-type: multipart/x-mixed-replace;boundary=--boundarydonotcross\r\n";
	char boundary[] = "--boundarydonotcross\r\n";
	char content_type_image_jpg[] = "Content-type: image/jpeg\r\n";
	char content_length_header[255];


	if (output_mode != NULL && strcmp(output_mode, "mjpeghttp") == 0) {
		fwrite(content_type_mixed_replace, strlen(content_type_mixed_replace), 1, stdout);
		fwrite(boundary, strlen(boundary), 1, stdout);
		fwrite(content_type_image_jpg, strlen(content_type_image_jpg), 1, stdout);
		fwrite(content_length_header, strlen(content_length_header), 1, stdout);

		sprintf(content_length_header, "Content-length: %d\r\n\r\n", jpeg_bytes+2);
		fwrite(content_length_header, strlen(content_length_header), 1, stdout);

		fwrite((void *)buffer, jpeg_bytes+2, 1, stdout);

		fwrite("\r\n", 2, 1, stdout);
		fwrite(boundary, strlen(boundary), 1, stdout);			
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int fd;
	int option_index = 0;
	int verbosity_level = 3;
	char *v4l2_device = NULL;
	char *output_dest = NULL;
	char *output_mode = NULL;

	while ((option_index = getopt(argc, argv, "v:d:o:f:")) != -1) {
		switch (option_index) {
			case 'v':
				verbosity_level = atoi(optarg);
				break;
			case 'd':
				v4l2_device = optarg;
				break;
			case 'o':
				output_dest = optarg;
				break;
			case 'f':
				output_mode = optarg;
				break;
			default:
				printf("Unknown option.\n");
				return 1;
		}
	}

	// Configure logging
	switch (verbosity_level) {
		case 0:
			log_set_level(LOGC_FATAL);
		case 1:
			log_set_level(LOGC_ERROR);
			break;
		case 2:
			log_set_level(LOGC_WARN);
			break;
		case 3:
			log_set_level(LOGC_INFO);
			break;
		case 4:
			log_set_level(LOGC_DEBUG);
			break;
		case 5:
			log_set_level(LOGC_TRACE);
			break;
		default:
			log_set_level(LOGC_INFO);
	}	

	log_info("Opening v4l2_device: %s\n", v4l2_device);
	fd = open(v4l2_device, O_RDWR);

	if (fd < 0) {
		perror("Opening video device");
		return EXIT_FAILURE;
	}

	if (print_caps(fd)) {
		return EXIT_FAILURE;
	}

	if (init_mmap(fd)) {
		return EXIT_FAILURE;
	}

	if(capture_image(fd, output_mode)) {
		return EXIT_FAILURE;
	}

	close(fd);
	return EXIT_SUCCESS;
}
