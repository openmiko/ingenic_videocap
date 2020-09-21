#include "configparser.h"


int populate_stream_settings(StreamSettings *settings, cJSON *json)
{
  int i;
  const char* attribute_names[] = {
    "name",
    "v4l2_device_path",
    "payload_type",
    "buffer_size",
    "profile",
    "pic_width",
    "pic_height",
    "mode",
    "frame_rate_numerator",
    "frame_rate_denominator",
    "max_group_of_pictures",
    "max_qp",
    "min_qp",
    "statistics_interval",
    "max_bitrate",
    "change_pos",
    "frame_qp_step",
    "gop_qp_step",
    "channel",
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
    "scaling_height"
  };
  cJSON *json_attribute;

  // Code to check if attribute is defined
  for (i = 0; i < sizeof(attribute_names) / sizeof(char *); i++) {
    json_attribute = cJSON_GetObjectItemCaseSensitive(json, attribute_names[i]);

    if (json_attribute == NULL) {
      log_error("Attribute %s must be defined even if not used");
      return -1;
    }
  }

  cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "name");
  cJSON *v4l2_device_path = cJSON_GetObjectItemCaseSensitive(json, "v4l2_device_path");
  cJSON *payload_type = cJSON_GetObjectItemCaseSensitive(json, "payload_type");
  cJSON *buffer_size = cJSON_GetObjectItemCaseSensitive(json, "buffer_size");
  cJSON *profile = cJSON_GetObjectItemCaseSensitive(json, "profile");
  cJSON *pic_width = cJSON_GetObjectItemCaseSensitive(json, "pic_width");
  cJSON *pic_height = cJSON_GetObjectItemCaseSensitive(json, "pic_height");
  cJSON *mode = cJSON_GetObjectItemCaseSensitive(json, "mode");
  cJSON *frame_rate_numerator = cJSON_GetObjectItemCaseSensitive(json, "frame_rate_numerator");
  cJSON *frame_rate_denominator = cJSON_GetObjectItemCaseSensitive(json, "frame_rate_denominator");
  cJSON *max_group_of_pictures = cJSON_GetObjectItemCaseSensitive(json, "max_group_of_pictures");
  cJSON *max_qp = cJSON_GetObjectItemCaseSensitive(json, "max_qp");
  cJSON *min_qp = cJSON_GetObjectItemCaseSensitive(json, "min_qp");
  cJSON *statistics_interval = cJSON_GetObjectItemCaseSensitive(json, "statistics_interval");
  cJSON *max_bitrate = cJSON_GetObjectItemCaseSensitive(json, "max_bitrate");
  cJSON *change_pos = cJSON_GetObjectItemCaseSensitive(json, "change_pos");
  cJSON *frame_qp_step = cJSON_GetObjectItemCaseSensitive(json, "frame_qp_step");
  cJSON *gop_qp_step = cJSON_GetObjectItemCaseSensitive(json, "gop_qp_step");
  cJSON *channel = cJSON_GetObjectItemCaseSensitive(json, "channel");
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

  if (!cJSON_IsNumber(pic_width) || !cJSON_IsNumber(pic_height))
  {
    log_error("pic_width or pic_height must be a number");
    return -1;
  }

  strcpy(settings->name, name->valuestring);
  strcpy(settings->v4l2_device_path, v4l2_device_path->valuestring);
  strcpy(settings->payload_type, payload_type->valuestring);
  settings->buffer_size = buffer_size->valueint;
  settings->profile = profile->valueint;
  settings->pic_width = pic_width->valueint;
  settings->pic_height = pic_height->valueint;
  strcpy(settings->mode, mode->valuestring);
  settings->frame_rate_numerator = frame_rate_numerator->valueint;
  settings->frame_rate_denominator = frame_rate_denominator->valueint;
  settings->max_group_of_pictures = max_group_of_pictures->valueint;
  settings->max_qp = max_qp->valueint;
  settings->min_qp = min_qp->valueint;
  settings->statistics_interval = statistics_interval->valueint;
  settings->max_bitrate = max_bitrate->valueint;
  settings->change_pos = change_pos->valueint;
  settings->frame_qp_step = frame_qp_step->valueint;
  settings->gop_qp_step = gop_qp_step->valueint;
  settings->channel = channel->valueint;
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

}