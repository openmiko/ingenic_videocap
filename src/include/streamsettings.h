#ifndef STREAMSETTINGS_H
#define STREAMSETTINGS_H

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

#endif /* STREAMSETTINGS_H */
