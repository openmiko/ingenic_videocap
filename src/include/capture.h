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

#include <imp_audio.h>
#include <imp_log.h>
#include <imp_common.h>
#include <imp_osd.h>
#include <imp_framesource.h>
#include <imp_isp.h>
#include <imp_system.h>
#include <imp_encoder.h>
#include <imp/imp_ivs.h>
#include <imp/imp_ivs_move.h>
#include <imp/imp_utils.h>

#include <linux/videodev2.h>
#include <h264_stream.h>
#include <cJSON.h>
#include <alsa/asoundlib.h>

//#define SENSOR_NAME "jxf22"
#define SENSOR_CUBS_TYPE TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR 0x40
#define SENSOR_WIDTH 1920
#define SENSOR_HEIGHT 1080

#define CHN0_EN 1
#define CHN1_EN 1
#define CROP_EN 1
#define SENSOR_FRAME_RATE_NUM 25
#define SENSOR_FRAME_RATE_DEN 1
#define SENSOR_WIDTH_SECOND 1920
#define SENSOR_HEIGHT_SECOND 1080
#define MAX_STREAMS 2
#define SENSOR_NAME_MAX_LENGTH 50

#define MILISEC 1000
#define MICROSEC 1000000
#define NANOSEC 1000000000

int initialize_sensor(IMPSensorInfo *sensor_info);
int initialize_audio();
int create_encoding_group(int group_id);
int setup_encoding_engine(FrameSource* frame_source, EncoderSetting* encoder_setting);
int output_v4l2_frames(EncoderSetting *encoder_setting);
int sensor_cleanup(IMPSensorInfo* sensor_info);
void hexdump(const char * desc, const void * addr, const int len);
void *produce_frames(void *ptr);
void print_stream_settings(StreamSettings *stream_settings);
void print_channel_attributes(IMPFSChnAttr *attr);
void print_encoder_channel_attributes(IMPEncoderCHNAttr *attr);

void *isp_settings_thread(void *isp_settings_);
void start_osd_update_threads(CameraConfig *camera_config);
void reload_night_vision(ISPSettings *isp_settings);
void reload_flip_image(ISPSettings *isp_settings);
void reload_encoder_config(EncoderSetting *encoder_setting);

// don't know why it doesn't work normally with the library....
char* itoa(int val, int base);

#endif /* CAPTURE_H */
