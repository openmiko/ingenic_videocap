#ifndef STREAMSETTINGS_H
#define STREAMSETTINGS_H

#include <stdint.h>
#include <imp_framesource.h>
#include <imp_encoder.h>

#define MAX_FRAMESOURCES		4
#define MAX_ENCODERS			10
#define MAX_BINDINGS			10

typedef struct frame_source {
	int id;
	int group;
	int pixel_format;
	char pixel_format_name[255];
	int frame_rate_numerator;
	int frame_rate_denominator;
	int buffer_size;
	int channel_type;
	int crop_enable;
	int crop_top;
	int crop_left;
	int crop_width;
	int crop_height;
	int scaling_enable;
	int scaling_width;
	int scaling_height;
	int pic_width;
	int pic_height;

	IMPFSChnAttr imp_fs_attrs;

} FrameSource;

typedef struct encoder_setting {
	int channel;
	int group;
	char v4l2_device_path[255];
	char payload_type[255];
	int buffer_size;
	int profile;
	char mode[255];
	int frame_rate_numerator;
	int frame_rate_denominator;
	int max_group_of_pictures;
	int max_qp;
	int min_qp;
	int frame_qp_step;
	int gop_qp_step;
	int pic_width;
	int pic_height;
	
	IMPEncoderCHNAttr chn_attr;

} EncoderSetting;

typedef struct binding_parameter {
	int device;
	int group;
	int output;
} BindingParameter;

typedef struct binding {
	BindingParameter source;
	BindingParameter target;
} Binding;

typedef struct stream_settings {
	char name[255];
	int enabled;
	int pic_width;
	int pic_height;
	int frame_rate_numerator;
	int frame_rate_denominator;
	int statistics_interval;
	int max_bitrate;
	int change_pos;
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
	EncoderSetting *encoders;
	int num_encoders;
} StreamSettings;


// Used to pass data to the encoding threads 
typedef struct encoder_thread_params {
	EncoderSetting *encoder;
} EncoderThreadParams;


typedef struct {
	FrameSource frame_sources[MAX_FRAMESOURCES];
	uint32_t num_framesources;

	EncoderSetting encoders[MAX_ENCODERS];
	uint32_t num_encoders;

	Binding bindings[MAX_BINDINGS];
	uint32_t num_bindings;

	uint32_t flip_vertical;
	uint32_t flip_horizontal;
	uint32_t show_timestamp;

} CameraConfig;



#endif /* STREAMSETTINGS_H */
