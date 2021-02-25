#ifndef STREAMSETTINGS_H
#define STREAMSETTINGS_H

#include <stdint.h>
#include <imp_osd.h>
#include <imp_framesource.h>
#include <imp_encoder.h>

// Shared memory
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define FTOKDIR "/usr"
#define FTOKPROJID 1

#define MAX_FRAMESOURCES 4
#define MAX_ENCODERS 10
#define MAX_BINDINGS 10
#define MAX_OSDGROUPS 4
#define MAX_OSDITEMS 16

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

typedef struct encoder_h264vbr_setting {
	int statistics_interval;
	int max_bitrate;
	int change_pos;
} EncoderH264VBRSetting;

typedef struct encoder_h264cbr_setting {
	int bitrate;
	int max_fps;
	int min_fps;
	int ibiaslvl;
	int adaptive_mode;
	int gop_relation;
} EncoderH264CBRSetting;

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
	
	EncoderH264VBRSetting h264vbr;
	EncoderH264CBRSetting h264cbr;
	
	IMPEncoderCHNAttr chn_attr;
	
	pthread_t thread_id;
	pthread_mutex_t mutex;
	int reload_flag;

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


typedef struct osd_item {
	int osd_id;
	char osd_type[32];
	IMPRgnHandle rgn_hander;
	
	int show;
	char type[32];
	char format[32];
	int pos_x, pos_y;
	int size_x, size_y;
	
	int layer;
	int g_alpha_en;
	char fg_alhpa[5];
	char bg_alhpa[5];
	
	char primary_color[64];
	char secondary_color[64];
	int line_width;
	
	char image[256];

	char text[128];
	char font[64];
	int extra_space_char_size;
	int line_spacing;
	int letter_spacing;
	int left_right_padding;
	int top_bottom_padding;
	int fixed_font_width;

	int update_interval;
	
	pthread_t thread_id;
	pthread_mutex_t mutex;
	int reload_flag;
} OsdItem;

typedef struct osd_group {
	int group_id;

	int osd_list_size;
	OsdItem osd_list[MAX_OSDITEMS];
} OsdGroup;

typedef struct osd_thread_data {
	OsdGroup *osd_group;
	OsdItem *osd_item;
} OsdThreadData;


typedef struct isp_settings {
	int night_mode;
	int night_mode_flag;
	int flip_image;
	int flip_image_flag;

	pthread_t thread_id;
	sem_t semaphore;
} ISPSettings;


typedef struct {
	ISPSettings isp_settings;
	
	FrameSource frame_sources[MAX_FRAMESOURCES];
	uint32_t num_framesources;

	EncoderSetting encoders[MAX_ENCODERS];
	uint32_t num_encoders;

	Binding bindings[MAX_BINDINGS];
	uint32_t num_bindings;
	
	OsdGroup osd_groups[MAX_OSDGROUPS];
	uint32_t num_osd_groups;

} CameraConfig;



#endif /* STREAMSETTINGS_H */
