#include "capture.h"

/* volatile might be necessary depending on the system/implementation in use. 
(see "C11 draft standard n1570: 5.1.2.3") */
volatile sig_atomic_t sigint_received = 0; 

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

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
  int ret, i;
  pthread_t thread_ids[num_streams];

  for (i = 0; i < num_streams; i++) {
    ret = pthread_create(&thread_ids[i], NULL, produce_frames, &stream_settings[i]);
    if (ret < 0) {
      log_error("Error creating thread for stream %d: %d", i, ret);
    }
    log_info("Created frame producer thread id: %d", thread_ids[i]);
  }

  for (i = 0; i < num_streams; i++) {
    log_info("Thread id %d go.", thread_ids[i]);
    pthread_join(thread_ids[i], NULL);
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
  cJSON *json_v4l2_device_path;

  char v4l2_device_path[255];

  StreamSettings stream_settings[MAX_STREAMS];


  if (argc != 2) {
    printf("./videocapture <json config file>\n");
    return -1;
  }

  signal(SIGINT, sigint_handler);

  log_set_level(LOG_INFO);
  log_set_lock(lock_callback, &log_mutex);


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


  json_stream_settings = cJSON_GetObjectItemCaseSensitive(json, "stream_settings");



  for (i = 0; i < MAX_STREAMS; ++i) {
    // TODO: Will leak memory here because I lose the pointer to json_stream
    json_stream = cJSON_DetachItemFromArray(json_stream_settings, 0);
    populate_stream_settings(&stream_settings[i], json_stream);
  }


  initialize_sensor(&sensor_info);
  initialize_audio();


  // This will suspend the main thread until the streams quit
  start_frame_producer_threads(stream_settings, MAX_STREAMS);


  // Resume execution
  log_info("All threads completed. Cleaning up.");
  sensor_cleanup(&sensor_info);


err:

  free(file_contents);
  cJSON_Delete(json);


  return 0;
}


