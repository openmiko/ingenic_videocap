#include "log.h"

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


typedef struct stream_settings {
	char name[255];
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
} StreamSettings;

int initialize_sensor(IMPSensorInfo *sensor_info);
int setup_framesource(StreamSettings* stream_settings, int channel);
int setup_encoding_engine(StreamSettings* stream_settings);
int output_v4l2_frames(char *v4l2_device_path, int video_width, int video_height);
int sensor_cleanup(IMPSensorInfo *sensor_info);
void hexdump(const char * desc, const void * addr, const int len);
