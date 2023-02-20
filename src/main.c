#include "capture.h"

/* volatile might be necessary depending on the system/implementation in use. 
(see "C11 draft standard n1570: 5.1.2.3") */
volatile sig_atomic_t sigint_received = 0; 

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t frame_generator_mutex;
FILE* logfile_fp;
int primary_log_callback_id;

snd_pcm_t *pcm_handle;


/* Signal Handler for SIGINT */
void sigint_handler(int sig_num) 
{ 
  // Reset handler to catch SIGINT next time. 
  signal(SIGINT, sigint_handler);

  sigint_received = 1;
  fflush(stdout); 
}

/* Signal Handler for SIGUSR1 / LOG_ROTATE_SIGNAL  */
void logging_signal_handler(int sig_num) 
{ 
  // Reset handler to catch LOG_ROTATE_SIGNAL next time. 
  signal(LOG_ROTATE_SIGNAL, logging_signal_handler);
  fflush(stdout);

  pthread_mutex_lock(&log_mutex);
  if (logfile_fp) {

    log_remove_callback_id(primary_log_callback_id);    
    fflush(logfile_fp);
    fclose(logfile_fp);
    logfile_fp = NULL;
  }

  logfile_fp = fopen(DEFAULT_LOGFILE, "a");
  primary_log_callback_id = log_add_fp(logfile_fp, LOGC_INFO);

  pthread_mutex_unlock(&log_mutex);
}

void setup_framesource(FrameSource *framesource)
{
  int ret;
  IMPFSChnAttr *fs_chn_attr = &framesource->imp_fs_attrs;

  fs_chn_attr->pixFmt = framesource->pixel_format;
  fs_chn_attr->outFrmRateNum = framesource->frame_rate_numerator;
  fs_chn_attr->outFrmRateDen = framesource->frame_rate_denominator;
  fs_chn_attr->nrVBs = framesource->buffer_size;
  fs_chn_attr->crop.enable = framesource->crop_enable;  
  fs_chn_attr->crop.top = framesource->crop_top;
  fs_chn_attr->crop.left = framesource->crop_left;
  fs_chn_attr->crop.width = framesource->crop_width;
  fs_chn_attr->crop.height = framesource->crop_height;
  fs_chn_attr->scaler.enable = framesource->scaling_enable;
  fs_chn_attr->scaler.outwidth = framesource->scaling_width;
  fs_chn_attr->scaler.outheight = framesource->scaling_height;
  fs_chn_attr->picWidth = framesource->pic_width;
  fs_chn_attr->picHeight = framesource->pic_height;
  fs_chn_attr->type = framesource->channel_type;


  // Each framesource is identified by a channel ID.
  // Each framesource also has 2 outputs based on the underlying hardware

  ret = IMP_FrameSource_CreateChn(framesource->id, fs_chn_attr);
  if(ret < 0){
    log_error("IMP_FrameSource_CreateChn error for framesource %d.", framesource->id);
    exit(1);
  }
  log_info("Created frame source channel %d", framesource->id);

  ret = IMP_FrameSource_SetChnAttr(framesource->id, fs_chn_attr);
  if (ret < 0) {
    log_error("IMP_FrameSource_SetChnAttr error for channel %d.", framesource->id);
    exit(1);
  }

  log_info("Frame source setup complete");
}


int setup_encoder(EncoderSetting *encoder_setting)
{
  int ret;

  log_info("Encoder channel attributes for channel %d", encoder_setting->channel);
  print_encoder_channel_attributes(&encoder_setting->chn_attr);

  create_encoding_group(encoder_setting->group);

  ret = IMP_Encoder_CreateChn(encoder_setting->channel, &encoder_setting->chn_attr);
  if (ret < 0) {
    log_error("Error creating encoder channel %d", encoder_setting->channel);      
    return -1;
  }

  log_info("Created encoder channel %d", encoder_setting->channel);

  ret = IMP_Encoder_RegisterChn(encoder_setting->group, encoder_setting->channel);
  if (ret < 0) {
    log_error("IMP_Encoder_RegisterChn error.");
    return -1;
  }
  log_info("IMP_Encoder_RegisterChn(Group [%d], Channel [%d])", encoder_setting->group, encoder_setting->channel);

  return 0;
}



int load_framesources(cJSON *json, CameraConfig *camera_config)
{
  int i;
  cJSON *json_stream;
  cJSON *json_framesources;

  log_info("Loading frame sources");

  // Parse framesources
  json_framesources = cJSON_GetObjectItemCaseSensitive(json, "frame_sources");
  if (json_framesources == NULL) {
    log_error("Key 'frame_sources' not found in JSON.");
    return -1;
  }
  camera_config->num_framesources = cJSON_GetArraySize(json_framesources);
  log_info("Found %d frame source(s).", camera_config->num_framesources);

  for (i = 0; i < camera_config->num_framesources; ++i) {
    json_stream = cJSON_DetachItemFromArray(json_framesources, 0);
    
    if( populate_framesource(&camera_config->frame_sources[i], json_stream) != 0) {
      log_error("Error parsing frame_sources[%d].", i);
      cJSON_Delete(json_stream);
      return -1;
    }
    print_framesource(&camera_config->frame_sources[i]);
    setup_framesource(&camera_config->frame_sources[i]);

    cJSON_Delete(json_stream);
  }
}

int load_encoders(cJSON *json, CameraConfig *camera_config)
{
  int i;
  cJSON *json_stream;
  cJSON *json_encoders;

  log_info("Loading encoders");

  // Parse encoders
  json_encoders = cJSON_GetObjectItemCaseSensitive(json, "encoders");
  if (json_encoders == NULL) {
    log_error("Key 'encoders' not found in JSON.");
    return -1;
  }
  camera_config->num_encoders = cJSON_GetArraySize(json_encoders);
  log_info("Found %d encoders.", camera_config->num_encoders);

  for (i = 0; i < camera_config->num_encoders; ++i) {
    json_stream = cJSON_DetachItemFromArray(json_encoders, 0);
    
    if( populate_encoder(&camera_config->encoders[i], json_stream) != 0) {
      log_error("Error parsing encoders[%d].", i);
      cJSON_Delete(json_stream);
      return -1;
    }
    print_encoder(&camera_config->encoders[i]);
    setup_encoder(&camera_config->encoders[i]);

    cJSON_Delete(json_stream);
  }

}



/*
  IMP_System_Bind is used to tie together a frame source and encoder.
  There are two framesources available and each of them have two outputs.

  This gives you a total of 4 outputs to work with.

  - Framesource Channel 0 with two outputs (Output ID 0 and Output ID 1)
  - Framesource Channel 1 with two outputs (Output ID 0 and Output ID 1)

  Additional caveats:

  - Each framesource output must be bound in order. You can't bind output ID 1 without first
  binding output ID 0.

  - You can configure the channels individually as needed.

  IMPCell interpretations for framesource and encoder parameters:

  Framesource:
    { DEV_ID_FS, IMP_FrameSource_CreateChn ID, Output ID 0 or 1};
  Encoder:
    { DEV_ID_ENC, Encoding Group (0 to 5), IMP_Encoder_GetStream Channel (0 to 5) };
*/
int setup_binding(Binding *binding)
{
  int ret;

  IMPCell source = { 
    binding->source.device,
    binding->source.group,
    binding->source.output
  };

  IMPCell target = { 
    binding->target.device,
    binding->target.group,
    binding->target.output
  };

  ret = IMP_System_Bind(&source, &target);
  if (ret < 0) {
    log_error("Error binding frame source to encoder group");
    exit(-1);
  }
  log_info("Success binding source to target");

}

int load_bindings(cJSON *json, CameraConfig *camera_config)
{
  int i;
  cJSON *json_stream;
  cJSON *json_bindings;

  log_info("Loading bindings");

  // Parse bindings
  json_bindings = cJSON_GetObjectItemCaseSensitive(json, "bindings");
  if (json_bindings == NULL) {
    log_error("Key 'bindings' not found in JSON.");
    return -1;
  }
  camera_config->num_bindings = cJSON_GetArraySize(json_bindings);
  log_info("Found %d bindings.", camera_config->num_bindings);

  for (i = 0; i < camera_config->num_bindings; ++i) {
    json_stream = cJSON_DetachItemFromArray(json_bindings, 0);
    
    if( populate_binding(&camera_config->bindings[i], json_stream) != 0) {
      log_error("Error parsing num_bindings[%d].", i);
      cJSON_Delete(json_stream);
      return -1;
    }
    print_binding(&camera_config->bindings[i]);
    setup_binding(&camera_config->bindings[i]);
    cJSON_Delete(json_stream);
  }
}

int load_general_settings(cJSON *json, CameraConfig *camera_config)
{
  int i;
  cJSON *json_stream;
  cJSON *json_general_settings;

  log_info("Loading general settings");

  // Parse general settings
  json_general_settings = cJSON_GetObjectItemCaseSensitive(json, "general_settings");
  if (json_general_settings == NULL) {
    log_error("Key 'general_settings' not found in JSON.");
    return -1;
  }

  cJSON *flip_vertical = cJSON_GetObjectItemCaseSensitive(json_general_settings, "flip_vertical");
  cJSON *flip_horizontal = cJSON_GetObjectItemCaseSensitive(json_general_settings, "flip_horizontal");
  cJSON *show_timestamp = cJSON_GetObjectItemCaseSensitive(json_general_settings, "show_timestamp");
  cJSON *timestamp_24h = cJSON_GetObjectItemCaseSensitive(json_general_settings, "timestamp_24h");
  cJSON *timestamp_location = cJSON_GetObjectItemCaseSensitive(json_general_settings, "timestamp_location");
  cJSON *enable_audio = cJSON_GetObjectItemCaseSensitive(json_general_settings, "enable_audio");
  cJSON *enable_logging = cJSON_GetObjectItemCaseSensitive(json_general_settings, "enable_logging");

  camera_config->flip_vertical = flip_vertical->valueint;
  camera_config->flip_horizontal = flip_horizontal->valueint;
  camera_config->show_timestamp = show_timestamp->valueint;

  camera_config->timestamp_24h = 0;
  if (timestamp_24h) {
    camera_config->timestamp_24h = timestamp_24h->valueint;
  }

  camera_config->timestamp_location = 0;
  if (timestamp_location) {
    camera_config->timestamp_location = timestamp_location->valueint;
  }

  camera_config->enable_audio = 0;
  if (enable_audio) {
    camera_config->enable_audio = enable_audio->valueint;
  }


  print_general_settings(camera_config);

  return 0;
}


void load_configuration(cJSON *json, CameraConfig *camera_config)
{
  load_general_settings(json, camera_config);
  load_framesources(json, camera_config);
  load_encoders(json, camera_config);
  load_bindings(json, camera_config);
}



int find_framesource_by_id(FrameSource frame_sources[], int num_framesources, int id)
{
  int i;
  for (i = 0; i < num_framesources; i++) {
    if (frame_sources[i].id == id) {
      return i;
    }
  }
  return -1;
}

void start_frame_producer_threads(CameraConfig *camera_config)
{
  int ret, i;
  pthread_t thread_ids[MAX_ENCODERS];
  EncoderThreadParams encoder_thread_params[MAX_ENCODERS];

  pthread_t audio_thread_id;
  pthread_t timestamp_osd_thread_id;
  pthread_t night_vision_thread_id;
  pthread_t real_time_configuration_thread_id;


  if(camera_config->enable_audio) {
    log_info("Starting audio thread");
    ret = pthread_create(&audio_thread_id, NULL, audio_thread_entry_start, NULL);
    if (ret < 0) {
      log_error("Error creating audio thread");
    }
  }

  log_info("Starting timestamp OSD thread");
  ret = pthread_create(&timestamp_osd_thread_id, NULL, timestamp_osd_entry_start, camera_config);
  if (ret < 0) {
    log_error("Error creating timestamp OSD thread");
  }

  log_info("Starting night vision thread");
  ret = pthread_create(&night_vision_thread_id, NULL, night_vision_entry_start, camera_config);
  if (ret < 0) {
    log_error("Error creating night vision thread");
  }

  log_info("Starting real time configuration thread");
  ret = pthread_create(&real_time_configuration_thread_id, NULL, real_time_configuration_start, camera_config);
  if (ret < 0) {
    log_error("Error creating real time configuration thread");
  }


  log_info("Starting frame producer threads for each encoder");

  for (i = 0; i < camera_config->num_encoders; i++) {
    log_info("Creating thread for encoder[%d]", i);

    encoder_thread_params[i].encoder  = &camera_config->encoders[i];

    ret = pthread_create(&thread_ids[i], NULL, produce_frames, &encoder_thread_params[i]);

    if (ret < 0) {
      log_error("Error creating thread for encoder[%d].", i);
    }

    log_info("Thread %d started.", thread_ids[i]);
    sleep(2);
  }

  for (i = 0; i < camera_config->num_encoders; i++) {
    log_info("Waiting for encoder thread_id %d to finish.", thread_ids[i]);
    pthread_join(thread_ids[i], NULL);
  }

  log_info("Waiting for audio thread %d to finish.", audio_thread_id);
  pthread_join(audio_thread_id, NULL);

  log_info("Waiting for OSD timestamp thread %d to finish.", timestamp_osd_thread_id);
  pthread_join(timestamp_osd_thread_id, NULL);

}

int enable_framesources(CameraConfig *camera_config)
{
  int ret, i;

  for (i = 0; i < camera_config->num_framesources; ++i) {

    ret = IMP_FrameSource_EnableChn(camera_config->frame_sources[i].id);

    if (ret < 0) {
      log_error("IMP_FrameSource_EnableChn(%d) error: %d", camera_config->frame_sources[i].id, ret);
      exit(-1);
    }
    log_info("Enabled FrameSource with id %d", camera_config->frame_sources[i].id);
  }

}

void lock_callback(bool lock, void* udata) {
  pthread_mutex_t *LOCK = (pthread_mutex_t*)(udata);
  if (lock)
    pthread_mutex_lock(LOCK);
  else
    pthread_mutex_unlock(LOCK);
}


int main(int argc, const char *argv[])
{
  CameraConfig camera_config;

  int i, ret;
  char *r;
  IMPSensorInfo sensor_info;


  // Variables related to parsing the JSON config file
  char filename[255];
  struct stat filestatus;
  FILE *fp;
  int file_size;
  char* file_contents;
  cJSON *json;

  cJSON *json_stream_settings;
  cJSON *json_stream;
  cJSON *json_framesources;
  cJSON *json_encoders;


  cJSON *settings;


  int num_framesources;
  FrameSource frame_sources[MAX_FRAMESOURCES];
  IMPFSChnAttr frame_source_attributes[MAX_FRAMESOURCES];

  int num_encoders;
  EncoderSetting encoders[MAX_ENCODERS];



  if (argc != 2) {
    printf("./videocapture <json config file>\n");
    return -1;
  }

  signal(SIGINT, sigint_handler);

  // Configure logging
  if(camera_config.enable_logging) {
    signal(LOG_ROTATE_SIGNAL, logging_signal_handler);

    log_set_level(LOGC_INFO);
    log_set_lock(lock_callback, &log_mutex);
    logfile_fp = fopen(DEFAULT_LOGFILE, "w+");
    primary_log_callback_id = log_add_fp(logfile_fp, LOGC_INFO);
  }
  

  // Reading the JSON file into memory  
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

  initialize_sensor(&sensor_info);

  if(camera_config.enable_audio) {
    initialize_audio();
  }

  // Parsing the JSON file
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

  ret = IMP_IVS_CreateGroup(0);
  if (ret < 0) {
    log_error("IMP_IVS_CreateGroup failed.");
    return -1;
  }

  ret = IMP_OSD_CreateGroup(0);
  if (ret < 0) {
    log_error("IMP_OSD_CreateGroup(0) failed");
    return -1;
  }
  
  load_configuration(json, &camera_config);
  configure_video_tuning_parameters(&camera_config);
  enable_framesources(&camera_config);


  if (pthread_mutex_init(&frame_generator_mutex, NULL) != 0) { 
    log_error("Failed to initialize frame_generator_mutex.");
    return -1;
  } 

  
  // This will suspend the main thread until the streams quit
  start_frame_producer_threads(&camera_config);


  // Resume execution
  log_info("All threads completed. Cleaning up.");
  sensor_cleanup(&sensor_info);


err:

  free(file_contents);
  cJSON_Delete(json);
  pthread_mutex_destroy(&frame_generator_mutex); 


  return 0;
}
