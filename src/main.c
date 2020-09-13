#include "capture.h"


volatile sig_atomic_t sigint_received = 0; /* volatile might be necessary depending on 
                                              the system/implementation in use. 
                                              (see "C11 draft standard n1570: 5.1.2.3")*/

/* Signal Handler for SIGINT */
void sigint_handler(int sig_num) 
{ 
  /* Reset handler to catch SIGINT next time. 
     Refer http://en.cppreference.com/w/c/program/signal */
  signal(SIGINT, sigint_handler);

  sigint_received = 1;
  printf("\nSIGINT received. Shutting down.\n"); 
  fflush(stdout); 
}


int populate_stream_settings(StreamSettings *settings, cJSON *json)
{

  cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "name");
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


  if (!cJSON_IsNumber(pic_width) || !cJSON_IsNumber(pic_height))
  {
    log_error("pic_width or pic_height must be a number");
    return -1;
  }

  strcpy(settings->name, name->valuestring);
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
}


int main(int argc, const char *argv[])
{
  int i, ret;
  IMPSensorInfo sensor_info;
  char *r;

  // Variables related to parsing the JSON config file
  char filename[255];
  struct stat filestatus;
  FILE *fp;
  int file_size;
  char* file_contents;
  cJSON *json;

  cJSON *stream_settings;
  cJSON *s1_json;
  cJSON *s2_json;

  cJSON *settings;
  cJSON *json_v4l2_device_path;

  char v4l2_device_path[255];

  // Stream settings
  StreamSettings s1_settings;
  StreamSettings s2_settings;


  if (argc != 2) {
    printf("./videocapture <json config file>\n");
    return -1;
  }

  signal(SIGINT, sigint_handler);

  log_set_level(LOG_INFO);


  r = strcpy(filename, argv[1]);
  if (r == NULL) {
    log_error("Error copying json config path.");
    return -1;
  }

  if ( stat(filename, &filestatus) != 0) {
    log_error("File %s not found\n", filename);
    return -1;
  }

  file_size = filestatus.st_size;
  file_contents = (char *)malloc(filestatus.st_size);
  if ( file_contents == NULL) {
    log_error("Memory error: unable to allocate %d bytes\n", file_size);
    return -1;
  }

  fp = fopen(filename, "rt");

  if (fp == NULL) {
    log_error("Unable to open %s", filename);
    fclose(fp);
    free(file_contents);
    return -1;
  }

  if ( fread(file_contents, file_size, 1, fp) != 1 ) {
    log_error("Unable to read contents of %s", filename);
    fclose(fp);
    free(file_contents);
    return -1;
  }
  fclose(fp);

  json = cJSON_ParseWithLength(file_contents, file_size);
  if (json == NULL) {
    log_error("Unable to parse JSON data");
    free(file_contents);

    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL)
    {
      log_error("Error before: %s", error_ptr);
    }
    cJSON_Delete(json);
    return -1;
  }

  json_v4l2_device_path = cJSON_GetObjectItemCaseSensitive(json, "v4l2_device_path");
  strcpy(v4l2_device_path, json_v4l2_device_path->valuestring);


  stream_settings = cJSON_GetObjectItemCaseSensitive(json, "stream_settings");


  s1_json = cJSON_DetachItemFromArray(stream_settings, 0);
  s2_json = cJSON_DetachItemFromArray(stream_settings, 1);


  populate_stream_settings(&s1_settings, s1_json);



  initialize_sensor(&sensor_info);

  setup_framesource(&s1_settings, 0);

  setup_encoding_engine(&s1_settings);

  // Create a thread that continuously outputs to the v4l2 device
  output_v4l2_frames(v4l2_device_path, s1_settings.pic_width, s1_settings.pic_height);

  sensor_cleanup(&sensor_info);


err:

  free(file_contents);
  cJSON_Delete(json);


  return 0;
}


