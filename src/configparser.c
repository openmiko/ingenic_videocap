#include "configparser.h"
#include "streamsettings.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>


int pixel_format_to_int(char* name) {
  int pixel_format = -1;

  if(strcmp(name, "PIX_FMT_YUV420P") == 0) {
    pixel_format = PIX_FMT_YUV420P;
  }
  if(strcmp(name, "PIX_FMT_YUYV422") == 0) {
    pixel_format = PIX_FMT_YUYV422;
  }
  if(strcmp(name, "PIX_FMT_UYVY422") == 0) {
    pixel_format = PIX_FMT_UYVY422;
  }
  if(strcmp(name, "PIX_FMT_YUV422P") == 0) {
    pixel_format = PIX_FMT_YUV422P;
  }
  if(strcmp(name, "PIX_FMT_YUV444P") == 0) {
    pixel_format = PIX_FMT_YUV444P;
  }
  if(strcmp(name, "PIX_FMT_YUV410P") == 0) {
    pixel_format = PIX_FMT_YUV410P;
  }
  if(strcmp(name, "PIX_FMT_YUV411P") == 0) {
    pixel_format = PIX_FMT_YUV411P;
  }
  if(strcmp(name, "PIX_FMT_GRAY8") == 0) {
    pixel_format = PIX_FMT_GRAY8;
  }
  if(strcmp(name, "PIX_FMT_MONOWHITE") == 0) {
    pixel_format = PIX_FMT_MONOWHITE;
  }
  if(strcmp(name, "PIX_FMT_MONOBLACK") == 0) {
    pixel_format = PIX_FMT_MONOBLACK;
  }
  if(strcmp(name, "PIX_FMT_NV12") == 0) {
    pixel_format = PIX_FMT_NV12;
  }
  if(strcmp(name, "PIX_FMT_NV21") == 0) {
    pixel_format = PIX_FMT_NV21;
  }
  if(strcmp(name, "PIX_FMT_RGB24") == 0) {
    pixel_format = PIX_FMT_RGB24;
  }
  if(strcmp(name, "PIX_FMT_BGR24") == 0) {
    pixel_format = PIX_FMT_BGR24;
  }
  if(strcmp(name, "PIX_FMT_ARGB") == 0) {
    pixel_format = PIX_FMT_ARGB;
  }
  if(strcmp(name, "PIX_FMT_RGBA") == 0) {
    pixel_format = PIX_FMT_RGBA;
  }
  if(strcmp(name, "PIX_FMT_ABGR") == 0) {
    pixel_format = PIX_FMT_ABGR;
  }
  if(strcmp(name, "PIX_FMT_BGRA") == 0) {
    pixel_format = PIX_FMT_BGRA;
  }
  if(strcmp(name, "PIX_FMT_RGB565BE") == 0) {
    pixel_format = PIX_FMT_RGB565BE;
  }
  if(strcmp(name, "PIX_FMT_RGB565LE") == 0) {
    pixel_format = PIX_FMT_RGB565LE;
  }
  if(strcmp(name, "PIX_FMT_RGB555BE") == 0) {
    pixel_format = PIX_FMT_RGB555BE;
  }
  if(strcmp(name, "PIX_FMT_RGB555LE") == 0) {
    pixel_format = PIX_FMT_RGB555LE;
  }
  if(strcmp(name, "PIX_FMT_BGR565BE") == 0) {
    pixel_format = PIX_FMT_BGR565BE;
  }
  if(strcmp(name, "PIX_FMT_BGR565LE") == 0) {
    pixel_format = PIX_FMT_BGR565LE;
  }
  if(strcmp(name, "PIX_FMT_BGR555BE") == 0) {
    pixel_format = PIX_FMT_BGR555BE;
  }
  if(strcmp(name, "PIX_FMT_BGR555LE") == 0) {
    pixel_format = PIX_FMT_BGR555LE;
  }
  if(strcmp(name, "PIX_FMT_0RGB") == 0) {
    pixel_format = PIX_FMT_0RGB;
  }
  if(strcmp(name, "PIX_FMT_RGB0") == 0) {
    pixel_format = PIX_FMT_RGB0;
  }
  if(strcmp(name, "PIX_FMT_0BGR") == 0) {
    pixel_format = PIX_FMT_0BGR;
  }
  if(strcmp(name, "PIX_FMT_BGR0") == 0) {
    pixel_format = PIX_FMT_BGR0;
  }
  if(strcmp(name, "PIX_FMT_BAYER_BGGR8") == 0) {
    pixel_format = PIX_FMT_BAYER_BGGR8;
  }
  if(strcmp(name, "PIX_FMT_BAYER_RGGB8") == 0) {
    pixel_format = PIX_FMT_BAYER_RGGB8;
  }
  if(strcmp(name, "PIX_FMT_BAYER_GBRG8") == 0) {
    pixel_format = PIX_FMT_BAYER_GBRG8;
  }
  if(strcmp(name, "PIX_FMT_BAYER_GRBG8") == 0) {
    pixel_format = PIX_FMT_BAYER_GRBG8;
  }

  if (pixel_format < 0) {
    log_error("Unknown pixel format: %s", name);
    return pixel_format;
  }

  return pixel_format;
}

int channel_type_to_int(char* name) {
  int channel_type = -1;

  if(strcmp(name, "FS_PHY_CHANNEL") == 0) {
    channel_type = FS_PHY_CHANNEL;
  }
  if(strcmp(name, "FS_EXT_CHANNEL") == 0) {
    channel_type = FS_EXT_CHANNEL;
  }
  return channel_type;
}

int device_name_to_id(char* name) {
  int device_id = -1;

  if(strcmp(name, "DEV_ID_FS") == 0) {
    device_id = DEV_ID_FS;
  }
  if(strcmp(name, "DEV_ID_ENC") == 0) {
    device_id = DEV_ID_ENC;
  }
  if(strcmp(name, "DEV_ID_IVS") == 0) {
    device_id = DEV_ID_IVS;
  }
  if(strcmp(name, "DEV_ID_OSD") == 0) {
    device_id = DEV_ID_OSD;
  }

  return device_id;
}

void device_id_to_string(int device_id, char *dest, int buffer_size)
{
  if (device_id == DEV_ID_FS) {
    snprintf(dest, buffer_size, "DEV_ID_FS");
  }
  else if (device_id == DEV_ID_ENC) {
    snprintf(dest, buffer_size, "DEV_ID_ENC");
  }
  else if (device_id == DEV_ID_IVS) {
    snprintf(dest, buffer_size, "DEV_ID_IVS");
  }
  else if (device_id == DEV_ID_OSD) {
    snprintf(dest, buffer_size, "DEV_ID_OSD");
  }
  else {
    snprintf(dest, buffer_size, "UNKNOWN DEVICE ID");
  }
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

void print_general_settings(CameraConfig *camera_config)
{
  char buffer[2048];

  // Be careful not to add an extra comma on the snprint
  snprintf(buffer, sizeof(buffer), "general_settings: \n\n"
                   "flip_vertical: %d\n"
                   "flip_horizontal: %d\n"
                   "show_timestamp: %d\n"
                   "timestamp_24h: %d\n"
                   "timestamp_location: %d\n"
                   "enable_audio: %d\n",
                    camera_config->flip_vertical,
                    camera_config->flip_horizontal,
                    camera_config->show_timestamp,
                    camera_config->timestamp_24h,
                    camera_config->timestamp_location,
                    camera_config->enable_audio
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

  snprintf(buffer, sizeof(buffer), "FrameSource: \n"
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
  encoder_setting->channel = channel->valueint;

  cJSON *group = cJSON_GetObjectItemCaseSensitive(json, "group");
  encoder_setting->group = group->valueint;

  cJSON *v4l2_device_path = cJSON_GetObjectItemCaseSensitive(json, "v4l2_device_path");
  strcpy(encoder_setting->v4l2_device_path, v4l2_device_path->valuestring);

  cJSON *payload_type = cJSON_GetObjectItemCaseSensitive(json, "payload_type");
  strcpy(encoder_setting->payload_type, payload_type->valuestring);

  cJSON *profile = cJSON_GetObjectItemCaseSensitive(json, "profile");
  encoder_setting->profile = profile->valueint;

  cJSON *mode = cJSON_GetObjectItemCaseSensitive(json, "mode");
  strcpy(encoder_setting->mode, mode->valuestring);

  cJSON *buffer_size = cJSON_GetObjectItemCaseSensitive(json, "buffer_size");
  encoder_setting->buffer_size = buffer_size->valueint;

  cJSON *frame_rate_numerator_enc = cJSON_GetObjectItemCaseSensitive(json, "frame_rate_numerator");
  encoder_setting->frame_rate_numerator = frame_rate_numerator_enc->valueint;

  cJSON *frame_rate_denominator_enc = cJSON_GetObjectItemCaseSensitive(json, "frame_rate_denominator");
  encoder_setting->frame_rate_denominator = frame_rate_denominator_enc->valueint;

  cJSON *max_group_of_pictures = cJSON_GetObjectItemCaseSensitive(json, "max_group_of_pictures");
  encoder_setting->max_group_of_pictures = max_group_of_pictures->valueint;

  cJSON *max_qp = cJSON_GetObjectItemCaseSensitive(json, "max_qp");
  encoder_setting->max_qp = max_qp->valueint;

  cJSON *min_qp = cJSON_GetObjectItemCaseSensitive(json, "min_qp");
  encoder_setting->min_qp = min_qp->valueint;

  cJSON *frame_qp_step = cJSON_GetObjectItemCaseSensitive(json, "frame_qp_step");
  encoder_setting->frame_qp_step = frame_qp_step->valueint;

  cJSON *gop_qp_step = cJSON_GetObjectItemCaseSensitive(json, "gop_qp_step");
  encoder_setting->gop_qp_step = gop_qp_step->valueint;

  cJSON *pic_width = cJSON_GetObjectItemCaseSensitive(json, "pic_width");
  encoder_setting->pic_width = pic_width->valueint;

  cJSON *pic_height = cJSON_GetObjectItemCaseSensitive(json, "pic_height");
  encoder_setting->pic_height = pic_height->valueint;

  IMPEncoderAttr *enc_attr;
  IMPEncoderRcAttr *rc_attr;

  memset(&encoder_setting->chn_attr, 0, sizeof(IMPEncoderCHNAttr));

  log_info("Setting up encoder attributes");
  enc_attr = &encoder_setting->chn_attr.encAttr;
  rc_attr =  &encoder_setting->chn_attr.rcAttr;



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
    rc_attr->attrH264Vbr.staticTime = statistics_interval->valueint;

    cJSON *max_bitrate = cJSON_GetObjectItemCaseSensitive(h264vbr_json, "max_bitrate");    
    rc_attr->attrH264Vbr.maxBitRate = max_bitrate->valueint;

    cJSON *change_pos = cJSON_GetObjectItemCaseSensitive(h264vbr_json, "change_pos");    
    rc_attr->attrH264Vbr.changePos = change_pos->valueint;

    rc_attr->attrH264Vbr.outFrmRate.frmRateNum = encoder_setting->frame_rate_numerator;
    rc_attr->attrH264Vbr.outFrmRate.frmRateDen = encoder_setting->frame_rate_denominator;
    rc_attr->attrH264Vbr.maxGop = encoder_setting->max_group_of_pictures;
    rc_attr->attrH264Vbr.maxQp = encoder_setting->max_qp;
    rc_attr->attrH264Vbr.minQp = encoder_setting->min_qp;  
    rc_attr->attrH264Vbr.FrmQPStep = encoder_setting->frame_qp_step;
    rc_attr->attrH264Vbr.GOPQPStep = encoder_setting->gop_qp_step;
  }

  log_info("Done setting up encoder attributes");

  return 0;
}

void print_encoder(EncoderSetting *encoder_setting)
{
  char buffer[1024];
  snprintf(buffer, sizeof(buffer), "EncoderSetting: \n"
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

  if (!cJSON_IsNumber(pic_width) || !cJSON_IsNumber(pic_height))
  {
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
