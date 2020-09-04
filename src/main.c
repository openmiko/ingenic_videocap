#include "capture.h"



int main(int argc, const char *argv[])
{
  int i, ret;
  IMPSensorInfo sensor_info;

  initialize_sensor(&sensor_info);

  setup_framesource();

  setup_encoding_engine(1920, 1080, 25);

  sensor_cleanup(&sensor_info);

  return 0;
}


