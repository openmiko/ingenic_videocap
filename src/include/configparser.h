#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <cJSON.h>
#include "streamsettings.h"
#include "imp_common.h"
#include "imp_framesource.h"

int populate_stream_settings(StreamSettings *settings, cJSON *json);
int populate_framesource(FrameSource *framesource, cJSON* json);
int populate_encoder(EncoderSetting *encoder_setting, cJSON* json);
int populate_binding(Binding *binding, cJSON* json);

void print_general_settings(CameraConfig *camera_config);
void print_framesource(FrameSource *framesource);
void print_encoder(EncoderSetting *encoder_setting);
void print_binding(Binding *binding);


#endif /* CONFIGPARSER_H */
