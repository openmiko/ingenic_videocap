#include "log.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <imp_log.h>
#include <imp_common.h>
#include <imp_osd.h>
#include <imp_framesource.h>
#include <imp_isp.h>
#include <imp_system.h>
#include <imp_encoder.h>
#include <linux/videodev2.h>
#include <h264_stream.h>


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

int initialize_sensor(IMPSensorInfo *sensor_info);
int setup_framesource();
int setup_encoding_engine(int video_width, int video_height, int fps);
int output_v4l2_frames(char *v4l2_device_path, int video_width, int video_height);
int sensor_cleanup(IMPSensorInfo *sensor_info);
void hexdump(const char * desc, const void * addr, const int len);
