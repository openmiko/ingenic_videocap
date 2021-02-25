#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <cJSON.h>
#include "streamsettings.h"
#include "imp_common.h"
#include "imp_framesource.h"

int populate_stream_settings(StreamSettings *settings, cJSON *json);
int populate_isp_settings(ISPSettings *isp_settings, cJSON* json);
int populate_framesource(FrameSource *framesource, cJSON* json);
int populate_encoder(EncoderSetting *encoder_setting, cJSON* json);
int populate_binding(Binding *binding, cJSON* json);
int populate_osd_group(OsdGroup *osdGroup, cJSON* json);

void print_isp_settings(ISPSettings *isp_settings);
void print_framesource(FrameSource *framesource);
void print_encoder(EncoderSetting *encoder_setting);
void print_binding(Binding *binding);
void print_osd_group(OsdGroup *osdGroup);

int pixel_format_to_int(char* tmp);
int osd_area_type_to_int(char *tmp);
uint32_t get_osd_color_from_string(char *tmp);

#endif /* CONFIGPARSER_H */
