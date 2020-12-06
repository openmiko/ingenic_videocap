#include "configparser.h"
#include "streamsettings.h"
#include "log.h"
#include <stdlib.h>

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


    cJSON *channel = cJSON_GetObjectItemCaseSensitive(encoder, "channel");
    settings->encoders[i].channel = channel->valueint;

    cJSON *v4l2_device_path = cJSON_GetObjectItemCaseSensitive(encoder, "v4l2_device_path");
    strcpy(settings->encoders[i].v4l2_device_path, v4l2_device_path->valuestring);

    cJSON *payload_type = cJSON_GetObjectItemCaseSensitive(encoder, "payload_type");
    strcpy(settings->encoders[i].payload_type, payload_type->valuestring);

    cJSON *buffer_size = cJSON_GetObjectItemCaseSensitive(encoder, "buffer_size");
    settings->encoders[i].buffer_size = buffer_size->valueint;

    cJSON *profile = cJSON_GetObjectItemCaseSensitive(encoder, "profile");
    settings->encoders[i].profile = profile->valueint;

    cJSON *mode = cJSON_GetObjectItemCaseSensitive(encoder, "mode");
    strcpy(settings->encoders[i].mode, mode->valuestring);

    cJSON *frame_rate_numerator_enc = cJSON_GetObjectItemCaseSensitive(encoder, "frame_rate_numerator");
    settings->encoders[i].frame_rate_numerator = frame_rate_numerator_enc->valueint;

    cJSON *frame_rate_denominator_enc = cJSON_GetObjectItemCaseSensitive(encoder, "frame_rate_denominator");
    settings->encoders[i].frame_rate_denominator = frame_rate_denominator_enc->valueint;

    cJSON *max_group_of_pictures = cJSON_GetObjectItemCaseSensitive(encoder, "max_group_of_pictures");
    settings->encoders[i].max_group_of_pictures = max_group_of_pictures->valueint;

    cJSON *max_qp = cJSON_GetObjectItemCaseSensitive(encoder, "max_qp");
    settings->encoders[i].max_qp = max_qp->valueint;

    cJSON *min_qp = cJSON_GetObjectItemCaseSensitive(encoder, "min_qp");
    settings->encoders[i].min_qp = min_qp->valueint;

    cJSON *frame_qp_step = cJSON_GetObjectItemCaseSensitive(encoder, "frame_qp_step");
    settings->encoders[i].frame_qp_step = frame_qp_step->valueint;

    cJSON *gop_qp_step = cJSON_GetObjectItemCaseSensitive(encoder, "gop_qp_step");
    settings->encoders[i].gop_qp_step = gop_qp_step->valueint;

  }

  return 0;
}