#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <cJSON.h>
#include "streamsettings.h"

int populate_stream_settings(StreamSettings *settings, cJSON *json);

#endif /* CONFIGPARSER_H */
