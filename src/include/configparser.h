#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include "capture.h"

int populate_stream_settings(StreamSettings *settings, cJSON *json);

#endif /* CONFIGPARSER_H */
