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

int main(int argc, const char *argv[])
{
  int i, ret;
  IMPSensorInfo sensor_info;
  char v4l2_device_path[255];
  char *r;

  if (argc != 2) {
    printf("./videocapture <v4l2 device>\n");
    return -1;
  }

  signal(SIGINT, sigint_handler);

  log_set_level(LOG_INFO);


  r = strcpy(v4l2_device_path, argv[1]);
  if (r == NULL) {
    log_error("Error copying v4l2 device path.");
    return -1;
  }

  initialize_sensor(&sensor_info);

  setup_framesource();

  setup_encoding_engine(1920, 1080, 25);

  // Create a thread that continuously outputs to the v4l2 device
  output_v4l2_frames(v4l2_device_path, 1920, 1080);

  sensor_cleanup(&sensor_info);

  return 0;
}


