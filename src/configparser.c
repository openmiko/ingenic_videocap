#include "configparser.h"
#include "streamsettings.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>

int pixel_format_to_int(char* tmp)
{
	if(strcmp(tmp, "PIX_FMT_YUV420P") == 0) {
		return PIX_FMT_YUV420P;
	}
	else if(strcmp(tmp, "PIX_FMT_YUYV422") == 0) {
		return PIX_FMT_YUYV422;
	}
	else if(strcmp(tmp, "PIX_FMT_UYVY422") == 0) {
		return PIX_FMT_UYVY422;
	}
	else if(strcmp(tmp, "PIX_FMT_YUV422P") == 0) {
		return PIX_FMT_YUV422P;
	}
	else if(strcmp(tmp, "PIX_FMT_YUV444P") == 0) {
		return PIX_FMT_YUV444P;
	}
	else if(strcmp(tmp, "PIX_FMT_YUV410P") == 0) {
		return PIX_FMT_YUV410P;
	}
	else if(strcmp(tmp, "PIX_FMT_YUV411P") == 0) {
		return PIX_FMT_YUV411P;
	}
	else if(strcmp(tmp, "PIX_FMT_GRAY8") == 0) {
		return PIX_FMT_GRAY8;
	}
	else if(strcmp(tmp, "PIX_FMT_MONOWHITE") == 0) {
		return PIX_FMT_MONOWHITE;
	}
	else if(strcmp(tmp, "PIX_FMT_MONOBLACK") == 0) {
		return PIX_FMT_MONOBLACK;
	}
	else if(strcmp(tmp, "PIX_FMT_NV12") == 0) {
		return PIX_FMT_NV12;
	}
	else if(strcmp(tmp, "PIX_FMT_NV21") == 0) {
		return PIX_FMT_NV21;
	}
	else if(strcmp(tmp, "PIX_FMT_RGB24") == 0) {
		return PIX_FMT_RGB24;
	}
	else if(strcmp(tmp, "PIX_FMT_BGR24") == 0) {
		return PIX_FMT_BGR24;
	}
	else if(strcmp(tmp, "PIX_FMT_ARGB") == 0) {
		return PIX_FMT_ARGB;
	}
	else if(strcmp(tmp, "PIX_FMT_RGBA") == 0) {
		return PIX_FMT_RGBA;
	}
	else if(strcmp(tmp, "PIX_FMT_ABGR") == 0) {
		return PIX_FMT_ABGR;
	}
	else if(strcmp(tmp, "PIX_FMT_BGRA") == 0) {
		return PIX_FMT_BGRA;
	}
	else if(strcmp(tmp, "PIX_FMT_RGB565BE") == 0) {
		return PIX_FMT_RGB565BE;
	}
	else if(strcmp(tmp, "PIX_FMT_RGB565LE") == 0) {
		return PIX_FMT_RGB565LE;
	}
	else if(strcmp(tmp, "PIX_FMT_RGB555BE") == 0) {
		return PIX_FMT_RGB555BE;
	}
	else if(strcmp(tmp, "PIX_FMT_RGB555LE") == 0) {
		return PIX_FMT_RGB555LE;
	}
	else if(strcmp(tmp, "PIX_FMT_BGR565BE") == 0) {
		return PIX_FMT_BGR565BE;
	}
	else if(strcmp(tmp, "PIX_FMT_BGR565LE") == 0) {
		return PIX_FMT_BGR565LE;
	}
	else if(strcmp(tmp, "PIX_FMT_BGR555BE") == 0) {
		return PIX_FMT_BGR555BE;
	}
	else if(strcmp(tmp, "PIX_FMT_BGR555LE") == 0) {
		return PIX_FMT_BGR555LE;
	}
	else if(strcmp(tmp, "PIX_FMT_0RGB") == 0) {
		return PIX_FMT_0RGB;
	}
	else if(strcmp(tmp, "PIX_FMT_RGB0") == 0) {
		return PIX_FMT_RGB0;
	}
	else if(strcmp(tmp, "PIX_FMT_0BGR") == 0) {
		return PIX_FMT_0BGR;
	}
	else if(strcmp(tmp, "PIX_FMT_BGR0") == 0) {
		return PIX_FMT_BGR0;
	}
	else if(strcmp(tmp, "PIX_FMT_BAYER_BGGR8") == 0) {
		return PIX_FMT_BAYER_BGGR8;
	}
	else if(strcmp(tmp, "PIX_FMT_BAYER_RGGB8") == 0) {
		return PIX_FMT_BAYER_RGGB8;
	}
	else if(strcmp(tmp, "PIX_FMT_BAYER_GBRG8") == 0) {
		return PIX_FMT_BAYER_GBRG8;
	}
	else if(strcmp(tmp, "PIX_FMT_BAYER_GRBG8") == 0) {
		return PIX_FMT_BAYER_GRBG8;
	}

	log_error("Unknown pixel format: %s", tmp);

	return -1;
}

int channel_type_to_int(char* name)
{
	int channel_type = -1;
	
	if(strcmp(name, "FS_PHY_CHANNEL") == 0) {
		channel_type = FS_PHY_CHANNEL;
	}
	else if(strcmp(name, "FS_EXT_CHANNEL") == 0) {
		channel_type = FS_EXT_CHANNEL;
	}
	
	return channel_type;
}

int device_name_to_id(char* name)
{	
	if(strcmp(name, "DEV_ID_FS") == 0) {
		return DEV_ID_FS;
	}
	else if(strcmp(name, "DEV_ID_ENC") == 0) {
		return DEV_ID_ENC;
	}
	else if(strcmp(name, "DEV_ID_OSD") == 0) {
		return DEV_ID_OSD;
	}
	else if(strcmp(name, "DEV_ID_IVS") == 0) {
		return DEV_ID_IVS;
	}
	
	return -1;
}

void device_id_to_string(int device_id, char *dest, int buffer_size)
{
	if (device_id == DEV_ID_FS) {
		snprintf(dest, buffer_size, "DEV_ID_FS");
	}
	else if (device_id == DEV_ID_ENC) {
		snprintf(dest, buffer_size, "DEV_ID_ENC");
	}
	else if (device_id == DEV_ID_OSD) {
		snprintf(dest, buffer_size, "DEV_ID_OSD");
	}
	else if (device_id == DEV_ID_IVS) {
		snprintf(dest, buffer_size, "DEV_ID_IVS");
	}
	else {
		snprintf(dest, buffer_size, "UNKNOWN DEVICE ID");
	}
}

int osd_area_type_to_int(char *tmp)
{
	if (strcmp(tmp, "OSD_REG_INV") == 0) {
		return OSD_REG_INV;
	}
	else if (strcmp(tmp, "OSD_REG_LINE") == 0) {
		return OSD_REG_LINE;
	}
	else if (strcmp(tmp, "OSD_REG_RECT") == 0) {
		return OSD_REG_RECT;
	}
	else if (strcmp(tmp, "OSD_REG_BITMAP") == 0) {
		return OSD_REG_BITMAP;
	}
	else if (strcmp(tmp, "OSD_REG_COVER") == 0) {
		return OSD_REG_COVER;
	}
	else if (strcmp(tmp, "OSD_REG_PIC") == 0) {
		return OSD_REG_PIC;
	}
	
	log_error("Unknown osd area type: %s", tmp);

	return -1;
}

uint32_t get_osd_color_from_string(char *tmp)
{
	// if color format 0x00000000 (argb)
	// not working.. need fix
	/*if (strncmp(tmp, "0x", 2) && strlen(tmp) == 10) {
		// validate string HEX value
		int i;
		for (i = 2; i < 10; i++) {
			if ((tmp[i] < '0' || tmp[i] > '9') ||
				(tmp[i] < 'a' || tmp[i] > 'f') ||
				(tmp[i] < 'A' || tmp[i] > 'F')) {
					//strcpy(tmp, "0xffffc0cb");
					return 0xffd94496; // pink error
			}
		}
		
		return strtoul(tmp, NULL, 16);
	}*/
	
	if(strcmp(tmp, "none") == 0) {
		return 0x00000000;
	}
	else if(strcmp(tmp, "white") == 0) {
		return OSD_WHITE;
	}
	else if(strcmp(tmp, "black") == 0) {
		return OSD_BLACK;
	}
	else if(strcmp(tmp, "red") == 0) {
		return OSD_RED;
	}
	else if(strcmp(tmp, "green") == 0) {
		return OSD_GREEN;
	}
	else if(strcmp(tmp, "blue") == 0) {
		return OSD_BLUE;
	}
	else if(strcmp(tmp, "cyan") == 0) {
		return OSD_GREEN|OSD_BLUE;
	}
	else if(strcmp(tmp, "yellow") == 0) {
		return OSD_RED|OSD_GREEN;
	}
	else if(strcmp(tmp, "magenta") == 0) {
		return OSD_BLUE|OSD_RED;
	}
	
	// spams the console too much if a color is not found
	//log_error("Unknown color: %s", tmp);
	//strcpy(tmp, "0xffffc0cb");
	
	return 0xffd94496; // pink error
}

int populate_isp_settings(ISPSettings *isp_settings, cJSON* json)
{
	int i;
	const char* attribute_names[] = {
		"night_mode",
		"flip_image"
	};

	cJSON *json_attribute;
	cJSON *encoder;

	// Code to check if top level attributes are defined
	for (i = 0; i < sizeof(attribute_names) / sizeof(char *); i++) {
		json_attribute = cJSON_GetObjectItemCaseSensitive(json, attribute_names[i]);

		if (json_attribute == NULL) {
			log_error("Attribute %s must be defined even if not used", attribute_names[i]);
			return -1;
		}
	}

	cJSON *night_mode = cJSON_GetObjectItemCaseSensitive(json, "night_mode");
	cJSON *flip_image = cJSON_GetObjectItemCaseSensitive(json, "flip_image");

	isp_settings->night_mode = night_mode->valueint;
	isp_settings->flip_image = flip_image->valueint;

	isp_settings->night_mode_flag = 1;
	isp_settings->flip_image_flag = 1;

	return 0;
}

int populate_framesource(FrameSource *framesource, cJSON* json)
{
	int i;
	const char* attribute_names[] = {
		"id",
		"group",
		"pixel_format",
		"frame_rate_numerator",
		"frame_rate_denominator",
		"buffer_size",
		"channel_type",
		"crop_enable",
		"crop_top",
		"crop_left",
		"crop_width",
		"crop_height",
		"scaling_enable",
		"scaling_width",
		"scaling_height",
		"pic_width",
		"pic_height"
	};

	cJSON *json_attribute;
	cJSON *encoder;

	// Code to check if top level attributes are defined
	for (i = 0; i < sizeof(attribute_names) / sizeof(char *); i++) {
		json_attribute = cJSON_GetObjectItemCaseSensitive(json, attribute_names[i]);
	
		if (json_attribute == NULL) {
			log_error("Attribute %s must be defined even if not used", attribute_names[i]);
			return -1;
		}
	}

	cJSON *id = cJSON_GetObjectItemCaseSensitive(json, "id");
	cJSON *group = cJSON_GetObjectItemCaseSensitive(json, "group");
	cJSON *pixel_format = cJSON_GetObjectItemCaseSensitive(json, "pixel_format");
	cJSON *frame_rate_numerator = cJSON_GetObjectItemCaseSensitive(json, "frame_rate_numerator");
	cJSON *frame_rate_denominator = cJSON_GetObjectItemCaseSensitive(json, "frame_rate_denominator");
	cJSON *buffer_size = cJSON_GetObjectItemCaseSensitive(json, "buffer_size");
	cJSON *channel_type = cJSON_GetObjectItemCaseSensitive(json, "channel_type");
	cJSON *crop_enable = cJSON_GetObjectItemCaseSensitive(json, "crop_enable");
	cJSON *crop_top = cJSON_GetObjectItemCaseSensitive(json, "crop_top");
	cJSON *crop_left = cJSON_GetObjectItemCaseSensitive(json, "crop_left");
	cJSON *crop_width = cJSON_GetObjectItemCaseSensitive(json, "crop_width");
	cJSON *crop_height = cJSON_GetObjectItemCaseSensitive(json, "crop_height");
	cJSON *scaling_enable = cJSON_GetObjectItemCaseSensitive(json, "scaling_enable");
	cJSON *scaling_width = cJSON_GetObjectItemCaseSensitive(json, "scaling_width");
	cJSON *scaling_height = cJSON_GetObjectItemCaseSensitive(json, "scaling_height");
	cJSON *pic_width = cJSON_GetObjectItemCaseSensitive(json, "pic_width");
	cJSON *pic_height = cJSON_GetObjectItemCaseSensitive(json, "pic_height");

	framesource->id = id->valueint;
	framesource->group = group->valueint;
	framesource->pixel_format = pixel_format_to_int(pixel_format->valuestring);
	strcpy(framesource->pixel_format_name, pixel_format->valuestring);
	framesource->frame_rate_numerator = frame_rate_numerator->valueint;
	framesource->frame_rate_denominator = frame_rate_denominator->valueint;
	framesource->buffer_size = buffer_size->valueint;
	framesource->channel_type = channel_type_to_int(channel_type->valuestring);
	framesource->crop_enable = crop_enable->valueint;
	framesource->crop_top = crop_top->valueint;
	framesource->crop_left = crop_left->valueint;
	framesource->crop_width = crop_width->valueint;
	framesource->crop_height = crop_height->valueint;
	framesource->scaling_enable = scaling_enable->valueint;
	framesource->scaling_width = scaling_width->valueint;
	framesource->scaling_height = scaling_height->valueint;
	framesource->pic_width = pic_width->valueint;
	framesource->pic_height = pic_height->valueint;

	return 0;
}

void print_isp_settings(ISPSettings *isp_settings)
{
	char buffer[1024];
	
	snprintf(buffer, sizeof(buffer), "ISP settings: \n"
		"night_mode: %d\n"
		"flip_image: %d\n",
		isp_settings->night_mode,
		isp_settings->flip_image
	);
	
	log_info("%s", buffer);
}

void print_framesource(FrameSource *framesource)
{
	char buffer[1024];
	// char type[32];
	
	// switch (framesource->channel_type) {
	//   case FS_PHY_CHANNEL:
	//     strcpy(type, "FS_PHY_CHANNEL");
	//     break;
	//   case FS_EXT_CHANNEL:
	//     strcpy(type, "FS_EXT_CHANNEL");
	//     break;
	//   default:
	//     strcpy(type, "UNKNOWN CHANNEL TYPE");
	// }
	
	snprintf(buffer, sizeof(buffer),
		"FrameSource: \n"
		"id: %d\n"
		"group: %d\n"
		"pixel_format: %d\n"
		"pixel_format_name: %s\n"
		"frame_rate_numerator: %d\n"
		"frame_rate_denominator: %d\n"
		"pic_width: %d\n"
		"pic_height: %d\n",
		framesource->id,
		framesource->group,
		framesource->pixel_format,
		framesource->pixel_format_name,
		framesource->frame_rate_numerator,
		framesource->frame_rate_denominator,
		framesource->pic_width,
		framesource->pic_height
	);
	
	log_info("%s", buffer);
}

int populate_encoder(EncoderSetting *encoder_setting, cJSON* json)
{
	int i;
	const char* attribute_names[] = {
		"channel",
		"group",
		"v4l2_device_path",
		"payload_type",
		"profile",
		"mode",
		"buffer_size",
		"frame_rate_numerator",
		"frame_rate_denominator",
		"max_group_of_pictures",
		"max_qp",
		"min_qp",
		"frame_qp_step",
		"gop_qp_step",
		"pic_width",
		"pic_height"
	};

	cJSON *json_attribute;

	// Code to check if top level attributes are defined
	for (i = 0; i < sizeof(attribute_names) / sizeof(char *); i++) {
		json_attribute = cJSON_GetObjectItemCaseSensitive(json, attribute_names[i]);
		
		if (json_attribute == NULL) {
			log_error("Attribute %s must be defined even if not used", attribute_names[i]);
			return -1;
		}
	}

	cJSON *channel = cJSON_GetObjectItemCaseSensitive(json, "channel");
	cJSON *group = cJSON_GetObjectItemCaseSensitive(json, "group");
	cJSON *v4l2_device_path = cJSON_GetObjectItemCaseSensitive(json, "v4l2_device_path");
	cJSON *payload_type = cJSON_GetObjectItemCaseSensitive(json, "payload_type");
	cJSON *profile = cJSON_GetObjectItemCaseSensitive(json, "profile");
	cJSON *mode = cJSON_GetObjectItemCaseSensitive(json, "mode");
	cJSON *buffer_size = cJSON_GetObjectItemCaseSensitive(json, "buffer_size");
	cJSON *frame_rate_numerator_enc = cJSON_GetObjectItemCaseSensitive(json, "frame_rate_numerator");
	cJSON *frame_rate_denominator_enc = cJSON_GetObjectItemCaseSensitive(json, "frame_rate_denominator");
	cJSON *max_group_of_pictures = cJSON_GetObjectItemCaseSensitive(json, "max_group_of_pictures");
	cJSON *max_qp = cJSON_GetObjectItemCaseSensitive(json, "max_qp");
	cJSON *min_qp = cJSON_GetObjectItemCaseSensitive(json, "min_qp");
	cJSON *frame_qp_step = cJSON_GetObjectItemCaseSensitive(json, "frame_qp_step");
	cJSON *gop_qp_step = cJSON_GetObjectItemCaseSensitive(json, "gop_qp_step");
	cJSON *pic_width = cJSON_GetObjectItemCaseSensitive(json, "pic_width");
	cJSON *pic_height = cJSON_GetObjectItemCaseSensitive(json, "pic_height");

	encoder_setting->channel = channel->valueint;
	encoder_setting->group = group->valueint;
	strcpy(encoder_setting->v4l2_device_path, v4l2_device_path->valuestring);
	strcpy(encoder_setting->payload_type, payload_type->valuestring);
	encoder_setting->profile = profile->valueint;
	strcpy(encoder_setting->mode, mode->valuestring);
	encoder_setting->buffer_size = buffer_size->valueint;
	encoder_setting->frame_rate_numerator = frame_rate_numerator_enc->valueint;
	encoder_setting->frame_rate_denominator = frame_rate_denominator_enc->valueint;
	encoder_setting->max_group_of_pictures = max_group_of_pictures->valueint;
	encoder_setting->max_qp = max_qp->valueint;
	encoder_setting->min_qp = min_qp->valueint;
	encoder_setting->frame_qp_step = frame_qp_step->valueint;
	encoder_setting->gop_qp_step = gop_qp_step->valueint;
	encoder_setting->pic_width = pic_width->valueint;
	encoder_setting->pic_height = pic_height->valueint;

	// ---
	IMPEncoderAttr *enc_attr;
	IMPEncoderRcAttr *rc_attr;
	memset(&encoder_setting->chn_attr, 0, sizeof(IMPEncoderCHNAttr));

	log_info("Setting up encoder attributes");
	enc_attr = &encoder_setting->chn_attr.encAttr;
	rc_attr = &encoder_setting->chn_attr.rcAttr;

	if (strcmp(encoder_setting->payload_type, "PT_H264") == 0) {
		enc_attr->enType = PT_H264;
	}
	else if(strcmp(encoder_setting->payload_type, "PT_JPEG") == 0) {
		enc_attr->enType = PT_JPEG;
	}
	else {
		log_error("Unknown payload type: %s", encoder_setting->payload_type);
		return -1;
	}

	enc_attr->bufSize = encoder_setting->buffer_size;
	enc_attr->profile = encoder_setting->profile;
	enc_attr->picWidth = encoder_setting->pic_width;
	enc_attr->picHeight = encoder_setting->pic_height;

	if (strcmp(encoder_setting->mode, "ENC_RC_MODE_H264VBR") == 0) {
		rc_attr->rcMode = ENC_RC_MODE_H264VBR;
	}
	else if (strcmp(encoder_setting->mode, "MJPEG") == 0) {
		rc_attr->rcMode = 0;
	}
	else {
		log_error("Unknown encoding mode: %s", encoder_setting->mode);
		return -1;
	}
	
	
	cJSON *h264vbr_json = cJSON_GetObjectItemCaseSensitive(json, "h264vbr_settings");
	if (h264vbr_json) {
		cJSON *statistics_interval = cJSON_GetObjectItemCaseSensitive(h264vbr_json, "statistics_interval");
		cJSON *max_bitrate = cJSON_GetObjectItemCaseSensitive(h264vbr_json, "max_bitrate");
		cJSON *change_pos = cJSON_GetObjectItemCaseSensitive(h264vbr_json, "change_pos");

		rc_attr->attrH264Vbr.staticTime = statistics_interval->valueint;
		rc_attr->attrH264Vbr.maxBitRate = max_bitrate->valueint;
		rc_attr->attrH264Vbr.changePos = change_pos->valueint;
		
		rc_attr->attrH264Vbr.outFrmRate.frmRateNum = encoder_setting->frame_rate_numerator;
		rc_attr->attrH264Vbr.outFrmRate.frmRateDen = encoder_setting->frame_rate_denominator;
		rc_attr->attrH264Vbr.maxGop = encoder_setting->max_group_of_pictures;
		rc_attr->attrH264Vbr.maxQp = encoder_setting->max_qp;
		rc_attr->attrH264Vbr.minQp = encoder_setting->min_qp;
		rc_attr->attrH264Vbr.FrmQPStep = encoder_setting->frame_qp_step;
		rc_attr->attrH264Vbr.GOPQPStep = encoder_setting->gop_qp_step;
		
		encoder_setting->h264vbr.statistics_interval = statistics_interval->valueint;
		encoder_setting->h264vbr.max_bitrate = max_bitrate->valueint;
		encoder_setting->h264vbr.change_pos = change_pos->valueint;
	}
	
	log_info("Done setting up encoder attributes");
	
	return 0;
}

void print_encoder(EncoderSetting *encoder_setting)
{
	char buffer[1024];
	
	snprintf(buffer, sizeof(buffer),
		"EncoderSetting: \n"
		"channel: %d\n"
		"group: %d\n"
		"v4l2_device_path: %s\n"
		"payload_type: %s\n"
		"profile: %d\n"
		"mode: %s\n"
		"frame_rate_numerator: %d\n"
		"frame_rate_denominator: %d\n",
		encoder_setting->channel,
		encoder_setting->group,
		encoder_setting->v4l2_device_path,
		encoder_setting->payload_type,
		encoder_setting->profile,
		encoder_setting->mode,
		encoder_setting->frame_rate_numerator,
		encoder_setting->frame_rate_denominator
	);
	
	log_info("%s", buffer);
}

int populate_binding(Binding *binding, cJSON* json)
{
	int i;
	const char* attribute_names[] = {
	"source",
	"target"
	};
	cJSON *json_attribute;
	
	// Code to check if top level attributes are defined
	for (i = 0; i < sizeof(attribute_names) / sizeof(char *); i++) {
		json_attribute = cJSON_GetObjectItemCaseSensitive(json, attribute_names[i]);
		
		if (json_attribute == NULL) {
			log_error("Attribute %s must be defined", attribute_names[i]);
			return -1;
		}
	}
	
	cJSON *source = cJSON_GetObjectItemCaseSensitive(json, "source");
	cJSON *source_device = cJSON_GetObjectItemCaseSensitive(source, "device");
	cJSON *source_group = cJSON_GetObjectItemCaseSensitive(source, "group");
	cJSON *source_output = cJSON_GetObjectItemCaseSensitive(source, "output");
	
	
	cJSON *target = cJSON_GetObjectItemCaseSensitive(json, "target");
	cJSON *target_device = cJSON_GetObjectItemCaseSensitive(target, "device");
	cJSON *target_group = cJSON_GetObjectItemCaseSensitive(target, "group");
	cJSON *target_output = cJSON_GetObjectItemCaseSensitive(target, "output");
	
	binding->source.device = device_name_to_id(source_device->valuestring);
	binding->source.group = source_group->valueint;
	binding->source.output = source_output->valueint;
	
	binding->target.device = device_name_to_id(target_device->valuestring);
	binding->target.group = target_group->valueint;
	binding->target.output = target_output->valueint;
	
	return 0;
}

int populate_osd_group(OsdGroup *osd_group, cJSON* json)
{
	int i;
	const char* attribute_names[] = {
		"group_id",
		"osd_items",
	};
	
	cJSON *json_attribute;

	for (i = 0; i < sizeof(attribute_names) / sizeof(char *); i++) {
		json_attribute = cJSON_GetObjectItemCaseSensitive(json, attribute_names[i]);

		if (json_attribute == NULL) {
			log_error("Attribute '%s' must be defined", attribute_names[i]);
			return -1;
		}
	}
	
	cJSON *json_group_id = cJSON_GetObjectItemCaseSensitive(json, attribute_names[0]); // group id
	osd_group->group_id = json_group_id->valueint;
	
	const char* attribute_names2[] = {
		"osd_id",
		"osd_type",
		"show",
		"type",
		"format",
		"pos_x",
		"pos_y",
		"size_x",
		"size_y",
		"layer",
		"g_alpha_en",
		"fg_alhpa",
		"bg_alhpa",
		"primary_color",
		"secondary_color",
		"line_width",
		"image",
		"font",
		"text",
		"extra_space_char_size",
		"line_spacing",
		"letter_spacing",
		"left_right_padding",
		"top_bottom_padding",
		"fixed_font_width",
		"update_interval"
	};
	
	cJSON *json_osd_items = cJSON_GetObjectItemCaseSensitive(json, attribute_names[1]); // osd_items
	
	osd_group->osd_list_size = cJSON_GetArraySize(json_osd_items);

	for (i = 0; i < osd_group->osd_list_size; i++) {
		OsdItem *osd_item = &osd_group->osd_list[i];
		cJSON *json_osd_item = cJSON_GetArrayItem(json_osd_items, i);
		
		// chech if all defined
		for (int u = 0; u < sizeof(attribute_names2) / sizeof(char *); u++) {
			json_attribute = cJSON_GetObjectItemCaseSensitive(json_osd_item, attribute_names2[u]);

			if (json_attribute == NULL) {
				log_error("Attribute '%s' must be defined", attribute_names2[u]);
				return -1;
			}
		}
		
		cJSON *json_osd_id = cJSON_GetObjectItemCaseSensitive(json_osd_item, "osd_id");
		cJSON *json_osd_type = cJSON_GetObjectItemCaseSensitive(json_osd_item, "osd_type");
		cJSON *json_show = cJSON_GetObjectItemCaseSensitive(json_osd_item, "show");
		cJSON *json_type = cJSON_GetObjectItemCaseSensitive(json_osd_item, "type");
		cJSON *json_format = cJSON_GetObjectItemCaseSensitive(json_osd_item, "format");
		cJSON *json_pos_x = cJSON_GetObjectItemCaseSensitive(json_osd_item, "pos_x");
		cJSON *json_pos_y = cJSON_GetObjectItemCaseSensitive(json_osd_item, "pos_y");
		cJSON *json_size_x = cJSON_GetObjectItemCaseSensitive(json_osd_item, "size_x");
		cJSON *json_size_y = cJSON_GetObjectItemCaseSensitive(json_osd_item, "size_y");
		cJSON *json_layer = cJSON_GetObjectItemCaseSensitive(json_osd_item, "layer");
		cJSON *json_g_alpha_en = cJSON_GetObjectItemCaseSensitive(json_osd_item, "g_alpha_en");
		cJSON *json_fg_alhpa = cJSON_GetObjectItemCaseSensitive(json_osd_item, "fg_alhpa");
		cJSON *json_bg_alhpa = cJSON_GetObjectItemCaseSensitive(json_osd_item, "bg_alhpa");
		cJSON *json_primary_color = cJSON_GetObjectItemCaseSensitive(json_osd_item, "primary_color");
		cJSON *json_secondary_color = cJSON_GetObjectItemCaseSensitive(json_osd_item, "secondary_color");
		cJSON *json_line_width = cJSON_GetObjectItemCaseSensitive(json_osd_item, "line_width");
		cJSON *json_image = cJSON_GetObjectItemCaseSensitive(json_osd_item, "image");
		cJSON *json_text = cJSON_GetObjectItemCaseSensitive(json_osd_item, "text");
		cJSON *json_font = cJSON_GetObjectItemCaseSensitive(json_osd_item, "font");
		cJSON *json_extra_space_char_size = cJSON_GetObjectItemCaseSensitive(json_osd_item, "extra_space_char_size");
		cJSON *json_line_spacing = cJSON_GetObjectItemCaseSensitive(json_osd_item, "line_spacing");
		cJSON *json_letter_spacing = cJSON_GetObjectItemCaseSensitive(json_osd_item, "letter_spacing");
		cJSON *json_left_right_padding = cJSON_GetObjectItemCaseSensitive(json_osd_item, "left_right_padding");
		cJSON *json_top_bottom_padding = cJSON_GetObjectItemCaseSensitive(json_osd_item, "top_bottom_padding");
		cJSON *json_fixed_font_width = cJSON_GetObjectItemCaseSensitive(json_osd_item, "fixed_font_width");
		cJSON *json_update_interval = cJSON_GetObjectItemCaseSensitive(json_osd_item, "update_interval");
		
		osd_item->osd_id = json_osd_id->valueint;
		strcpy(osd_item->osd_type, json_osd_type->valuestring);
		osd_item->show = json_show->valueint;
		strcpy(osd_item->type, json_type->valuestring);
		strcpy(osd_item->format, json_format->valuestring);
		osd_item->pos_x = json_pos_x->valueint;
		osd_item->pos_y = json_pos_y->valueint;
		osd_item->size_x = json_size_x->valueint;
		osd_item->size_y = json_size_y->valueint;
		osd_item->layer = json_layer->valueint;
		osd_item->g_alpha_en = json_g_alpha_en->valueint;
		strcpy(osd_item->fg_alhpa, json_fg_alhpa->valuestring);
		strcpy(osd_item->bg_alhpa, json_bg_alhpa->valuestring);
		strcpy(osd_item->primary_color, json_primary_color->valuestring);
		strcpy(osd_item->secondary_color, json_secondary_color->valuestring);
		osd_item->line_width = json_line_width->valueint;
		strcpy(osd_item->image, json_image->valuestring);
		strcpy(osd_item->text, json_text->valuestring);
		strcpy(osd_item->font, json_font->valuestring);
		osd_item->extra_space_char_size = json_extra_space_char_size->valueint;
		osd_item->line_spacing = json_line_spacing->valueint;
		osd_item->letter_spacing = json_letter_spacing->valueint;
		osd_item->left_right_padding = json_left_right_padding->valueint;
		osd_item->top_bottom_padding = json_top_bottom_padding->valueint;
		osd_item->fixed_font_width = json_fixed_font_width->valueint;
		osd_item->update_interval = json_update_interval->valueint;
	}

	return 0;
}

void print_binding(Binding *binding)
{
	char buffer[1024];
	char source_device[20];
	char target_device[20];

	device_id_to_string(binding->source.device, source_device, 20);
	device_id_to_string(binding->target.device, target_device, 20);

	snprintf(buffer, sizeof(buffer), "Binding: \n"
		"source_device: %s\n"
		"source_group: %d\n"
		"source_output: %d\n"
		"target_device: %s\n"
		"target_group: %d\n"
		"target_output: %d\n",
		source_device,
		binding->source.group,
		binding->source.output,
		target_device,
		binding->target.group,
		binding->target.output
	);
	
	log_info("%s", buffer);
}

void print_osd_group(OsdGroup *osd_group)
{
	int i;
	char buffer[4096];
	
	log_info("OSD group %d:", osd_group->group_id);
	
	for (i = 0; i < osd_group->osd_list_size; i++) {
		OsdItem *osd = &osd_group->osd_list[i];

		snprintf(buffer, sizeof(buffer),
			"OSD item: \n"
			"osd_id: %d\n"
			"osd_type: %s\n"
			"show: %d\n"
			"type: %s\n"
			"format: %s\n"
			"pos_x: %d\n"
			"pos_y: %d\n"
			"size_x: %d\n"
			"size_y: %d\n"
			"layer: %d\n"
			"g_alpha_en: %d\n"
			"fg_alhpa: 0x%x\n"
			"bg_alhpa: 0x%x\n"
			"primary_color: %s\n"
			"secondary_color: %s\n"
			"line_width: %d\n"
			"image: %s\n"
			"text: %s\n"
			"font: %s\n"
			"extra_space_char_size: %d\n"
			"line_spacing: %d\n"
			"letter_spacing: %d\n"
			"left_right_padding: %d\n"
			"top_bottom_padding: %d\n"
			"fixed_font_width: %d\n"
			"update_interval: %d\n",
			osd->osd_id,
			osd->osd_type,
			osd->show,
			osd->type,
			osd->format,
			osd->pos_x,
			osd->pos_y,
			osd->size_x,
			osd->size_y,
			osd->layer,
			osd->g_alpha_en,
			osd->fg_alhpa,
			osd->bg_alhpa,
			osd->primary_color,
			osd->secondary_color,
			osd->line_width,
			osd->image,
			osd->text,
			osd->font,
			osd->extra_space_char_size,
			osd->line_spacing,
			osd->letter_spacing,
			osd->left_right_padding,
			osd->top_bottom_padding,
			osd->fixed_font_width,
			osd->update_interval
		);
		
		log_info("%s", buffer);
	}
}

int populate_stream_settings(StreamSettings *settings, cJSON *json)
{
	int i;
	int num_encoders;

	const char* attribute_names[] = {
		"name",
		"enabled",
		"pic_width",
		"pic_height",
		"group",
		"pixel_format",
		"video_buffers",
		"channel_type",
		"crop_enable",
		"crop_top",
		"crop_left",
		"crop_width",
		"crop_height",
		"scaling_enable",
		"scaling_width",
		"scaling_height",
		"frame_rate_numerator",
		"frame_rate_denominator"
	};
	
	cJSON *json_attribute;
	cJSON *encoder;

	// Code to check if top level attributes are defined
	for (i = 0; i < sizeof(attribute_names) / sizeof(char *); i++) {
		json_attribute = cJSON_GetObjectItemCaseSensitive(json, attribute_names[i]);
		
		if (json_attribute == NULL) {
			log_error("Attribute %s must be defined even if not used", attribute_names[i]);
			return -1;
		}
	}
	
	cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "name");
	cJSON *enabled = cJSON_GetObjectItemCaseSensitive(json, "enabled");
	cJSON *pic_width = cJSON_GetObjectItemCaseSensitive(json, "pic_width");
	cJSON *pic_height = cJSON_GetObjectItemCaseSensitive(json, "pic_height");
	cJSON *group = cJSON_GetObjectItemCaseSensitive(json, "group");
	cJSON *pixel_format = cJSON_GetObjectItemCaseSensitive(json, "pixel_format");
	cJSON *video_buffers = cJSON_GetObjectItemCaseSensitive(json, "video_buffers");
	cJSON *channel_type = cJSON_GetObjectItemCaseSensitive(json, "channel_type");
	cJSON *crop_enable = cJSON_GetObjectItemCaseSensitive(json, "crop_enable");
	cJSON *crop_top = cJSON_GetObjectItemCaseSensitive(json, "crop_top");
	cJSON *crop_left = cJSON_GetObjectItemCaseSensitive(json, "crop_left");
	cJSON *crop_width = cJSON_GetObjectItemCaseSensitive(json, "crop_width");
	cJSON *crop_height = cJSON_GetObjectItemCaseSensitive(json, "crop_height");
	cJSON *scaling_enable = cJSON_GetObjectItemCaseSensitive(json, "scaling_enable");
	cJSON *scaling_width = cJSON_GetObjectItemCaseSensitive(json, "scaling_width");
	cJSON *scaling_height = cJSON_GetObjectItemCaseSensitive(json, "scaling_height");
	cJSON *frame_rate_numerator = cJSON_GetObjectItemCaseSensitive(json, "frame_rate_numerator");
	cJSON *frame_rate_denominator = cJSON_GetObjectItemCaseSensitive(json, "frame_rate_denominator");
	
	cJSON *encoders = cJSON_GetObjectItemCaseSensitive(json, "encoders");
	
	if (!cJSON_IsNumber(pic_width) || !cJSON_IsNumber(pic_height)) {
		log_error("pic_width or pic_height must be a number");
		return -1;
	}
	
	strcpy(settings->name, name->valuestring);
	settings->enabled = enabled->valueint;
	settings->pic_width = pic_width->valueint;
	settings->pic_height = pic_height->valueint;
	settings->group = group->valueint;
	
	strcpy(settings->pixel_format, pixel_format->valuestring);
	settings->video_buffers = video_buffers->valueint;
	strcpy(settings->channel_type,channel_type->valuestring);
	settings->crop_enable = crop_enable->valueint;
	settings->crop_top = crop_top->valueint;
	settings->crop_left = crop_left->valueint;
	settings->crop_width = crop_width->valueint;
	settings->crop_height = crop_height->valueint;
	settings->scaling_enable = scaling_enable->valueint;
	settings->scaling_width = scaling_width->valueint;
	settings->scaling_height = scaling_height->valueint;
	settings->frame_rate_numerator = frame_rate_numerator->valueint;
	settings->frame_rate_denominator = frame_rate_denominator->valueint;
	
	
	// Dynamically allocate an array to store encoder structs
	num_encoders = cJSON_GetArraySize(encoders);
	settings->num_encoders = num_encoders;
	settings->encoders = malloc(num_encoders * sizeof(EncoderSetting));
	
	
	for (i = 0; i < num_encoders; ++i) {
		encoder = cJSON_GetArrayItem(encoders, i);
	}
	
	return 0;
}
