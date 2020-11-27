#include "capture.h"

/* volatile might be necessary depending on the system/implementation in use. 
(see "C11 draft standard n1570: 5.1.2.3") */
volatile sig_atomic_t sigint_received = 0; 

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t frame_generator_mutex; 

snd_pcm_t *pcm_handle;

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



void start_frame_producer_threads(StreamSettings stream_settings[], int num_streams)
{
  int ret, i, j;
  pthread_t *thread_ids;
  int total_encoders = 0;
  int current_encoder = 0;

  EncoderThreadParams *encoder_thread_params;



  IMPFSChnAttr fs_chn_attr = {
    .pixFmt = PIX_FMT_NV12,
    .outFrmRateNum = 25,
    .outFrmRateDen = 1,
    .nrVBs = 3,
    .type = FS_PHY_CHANNEL,

    .crop.enable = 0,
    .crop.top = 0,
    .crop.left = 0,
    .crop.width = 0,
    .crop.height = 0,

    .scaler.enable = 0,
    .scaler.outwidth = 0,
    .scaler.outheight = 0,
    .picWidth = 0,
    .picHeight = 0
  };

  // Calculate out the number of threads to create by looping through all the streams
  for (i = 0; i < num_streams; i++) {
    if (stream_settings[i].enabled == 1) {
      total_encoders = total_encoders + stream_settings[i].num_encoders;
    }
  }
  log_info("Found %d encoder definitions across all enabled streams.", total_encoders);


  // Allocate space for the thread IDs and parameters
  thread_ids = (pthread_t *)calloc(total_encoders, sizeof(pthread_t)); 
  encoder_thread_params = (EncoderThreadParams *)calloc(total_encoders, sizeof(EncoderThreadParams)); 



  // Loop through all the streams
  for (i = 0; i < num_streams; i++) {

    if (stream_settings[i].enabled == 0) {
      log_info("Stream %d is not enabled. Skipping.", i);
      continue;
    }

    fs_chn_attr.picWidth = stream_settings[i].pic_width;
    fs_chn_attr.picHeight = stream_settings[i].pic_height;


    ret = IMP_FrameSource_CreateChn(i, &fs_chn_attr);
    if(ret < 0){
      log_error("IMP_FrameSource_CreateChn error for channel %d.", i);
      return;
    }
    log_info("Created frame source channel %d", i);

    ret = IMP_FrameSource_SetChnAttr(i, &fs_chn_attr);
    if (ret < 0) {
      log_error("IMP_FrameSource_SetChnAttr error for channel %d.", i);
      return;
    }
    log_info("Frame source setup complete for channel %d", i);



    for (int j = 0; j < stream_settings[i].num_encoders; ++j) {
      log_info("Creating thread for stream_settings[%d] / encoder[%d]", i, j);

      encoder_thread_params[current_encoder].stream_settings = &stream_settings[i];
      encoder_thread_params[current_encoder].encoder_setting = &stream_settings[i].encoders[j];

      ret = pthread_create(&thread_ids[current_encoder], NULL, produce_frames, &encoder_thread_params[current_encoder]);

      if (ret < 0) {
        log_error("Error creating thread for stream %d: %d", i, ret);
      }

      log_info("Thread %d started.", thread_ids[current_encoder]);

      current_encoder = current_encoder + 1;

    }

  }

  log_info("Created a total of %d threads.", current_encoder);

  for (i = 0; i < total_encoders; i++) {
    log_info("Waiting for thread %d to finish.", thread_ids[i]);
    pthread_join(thread_ids[i], NULL);
  }

  free(thread_ids);
  free(encoder_thread_params);

}



void lock_callback(bool lock, void* udata) {
  pthread_mutex_t *LOCK = (pthread_mutex_t*)(udata);
  if (lock)
    pthread_mutex_lock(LOCK);
  else
    pthread_mutex_unlock(LOCK);
}

// TODO: Possible refactoring methods
// Parsing JSON file



int main(int argc, const char *argv[])
{
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

  cJSON *settings;

  int num_stream_settings;
  StreamSettings stream_settings[MAX_STREAMS];


  if (argc != 2) {
    printf("./videocapture <json config file>\n");
    return -1;
  }

  signal(SIGINT, sigint_handler);


  // Configure logging
  log_set_level(LOG_INFO);
  log_set_lock(lock_callback, &log_mutex);




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


  json_stream_settings = cJSON_GetObjectItemCaseSensitive(json, "stream_settings");
  if (json_stream_settings == NULL) {
    log_error("Key 'stream_settings' not found in JSON.");
    return -1;
  }

  num_stream_settings = cJSON_GetArraySize(json_stream_settings);
  log_info("Found %d stream settings elements in JSON.", num_stream_settings);


  for (i = 0; i < num_stream_settings; ++i) {
    json_stream = cJSON_DetachItemFromArray(json_stream_settings, 0);
    
    if( populate_stream_settings(&stream_settings[i], json_stream) != 0) {
      cJSON_Delete(json_stream);
      return -1;
    }

    cJSON_Delete(json_stream);
  }


  initialize_sensor(&sensor_info);
  initialize_audio();


  if (pthread_mutex_init(&frame_generator_mutex, NULL) != 0) { 
    log_error("Failed to initialize frame_generator_mutex.");
    return -1;
  } 


  // This will suspend the main thread until the streams quit
  start_frame_producer_threads(stream_settings, num_stream_settings);


  // Resume execution
  log_info("All threads completed. Cleaning up.");
  sensor_cleanup(&sensor_info);


err:

  free(file_contents);
  cJSON_Delete(json);
  pthread_mutex_destroy(&frame_generator_mutex); 


  return 0;
}


