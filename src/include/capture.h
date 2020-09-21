#ifndef CAPTURE_H /* include guard */
#define CAPTURE_H


#include "log.h"
#include "configparser.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <time.h>

#include <imp_log.h>
#include <imp_common.h>
#include <imp_osd.h>
#include <imp_framesource.h>
#include <imp_isp.h>
#include <imp_system.h>
#include <imp_encoder.h>
#include <linux/videodev2.h>
#include <h264_stream.h>
#include <cJSON.h>


#define SENSOR_NAME				"jxf23"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x40
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 1
#define CROP_EN					1

#define SENSOR_FRAME_RATE_NUM		25
#define SENSOR_FRAME_RATE_DEN		1

#define SENSOR_WIDTH_SECOND			1920
#define SENSOR_HEIGHT_SECOND		1080


#define MAX_STREAMS				2

typedef struct stream_settings {
	char name[255];
	char v4l2_device_path[255];
	char payload_type[255];
	int buffer_size;
	int profile;
	int pic_width;
	int pic_height;
	char mode[255];
	int frame_rate_numerator;
	int frame_rate_denominator;
	int max_group_of_pictures;
	int max_qp;
	int min_qp;
	int statistics_interval;
	int max_bitrate;
	int change_pos;
	int frame_qp_step;
	int gop_qp_step;
	int channel;
	int group;
	char pixel_format[255];
	int video_buffers;
	char channel_type[255];
	int crop_enable;
	int crop_top;
	int crop_left;
	int crop_width;
	int crop_height;
	int scaling_enable;
	int scaling_width;
	int scaling_height;
} StreamSettings;

int initialize_sensor(IMPSensorInfo *sensor_info);
int setup_framesource(StreamSettings* stream_settings);
int setup_encoding_engine(StreamSettings* stream_settings);
int output_v4l2_frames(StreamSettings *stream_settings);
int sensor_cleanup(IMPSensorInfo *sensor_info);
void hexdump(const char * desc, const void * addr, const int len);
void *produce_frames(void *ptr);
void print_stream_settings(StreamSettings *stream_settings);

#endif /* CAPTURE_H */
