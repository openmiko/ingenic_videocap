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


int initialize_sensor(IMPSensorInfo *sensor_info);
int setup_framesource(StreamSettings* stream_settings);
int setup_encoding_engine(StreamSettings* stream_settings);
int output_v4l2_frames(StreamSettings *stream_settings);
int sensor_cleanup(IMPSensorInfo *sensor_info);
void hexdump(const char * desc, const void * addr, const int len);
void *produce_frames(void *ptr);
void print_stream_settings(StreamSettings *stream_settings);

#endif /* CAPTURE_H */
