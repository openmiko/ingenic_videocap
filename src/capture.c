#include "capture.h"
#include "bgramapinfo.h"

/*

ISP - Ingenic Smart Platform
IMP - Ingenic Multimedia Platform

*/

extern sig_atomic_t sigint_received;
extern snd_pcm_t *pcm_handle;
extern pthread_mutex_t frame_generator_mutex; 

int FrameSourceEnabled[5] = {0,0,0,0,0};
IMPRgnHandle osdRegion;

int initialize_sensor(IMPSensorInfo *sensor_info)
{
  int ret;
  char sensor_name_buffer[SENSOR_NAME_MAX_LENGTH];
  char *sensor_name;
  FILE *sensor_info_proc_file;
  
  log_info("Initializing sensor");

  // Read sensor from /proc filesystem
  // The string will look like "sensor :jxf23\n" so need to parse it
  sensor_info_proc_file = fopen("/proc/jz/sinfo/info","r");

  if( fgets(sensor_name_buffer, SENSOR_NAME_MAX_LENGTH, sensor_info_proc_file) == NULL) {
    log_error("Error getting sensor name from /proc/jz/sinfo/info");
    return -1;
  }

  // Pointer to first occurance of the colon
  sensor_name = strstr(sensor_name_buffer, ":");
  if (sensor_name != NULL) {
    sensor_name = sensor_name + 1;
    // Assume the last character is a newline and remove it
    sensor_name[strlen(sensor_name)-1] = '\0';
  }
  else {
    log_error("Expecting sensor name read from /proc to have a colon.");
    return -1;
  }

  log_info("Determined sensor name: %s", sensor_name);

  memset(sensor_info, 0, sizeof(IMPSensorInfo));
	memcpy(sensor_info->name, sensor_name, strlen(sensor_name)+1);
	sensor_info->cbus_type = SENSOR_CUBS_TYPE;
	memcpy(sensor_info->i2c.type, sensor_name, strlen(sensor_name)+1);
  sensor_info->i2c.addr = SENSOR_I2C_ADDR;

  log_info("IMPSensorInfo details: ");
  log_info("sensor_info->name: %s", sensor_info->name);



	ret = IMP_ISP_Open();
	if(ret < 0){
		log_error("Failed to open ISP");
		return -1;
	}
  else {
    log_info("Opened the ISP module.");
  }

	ret = IMP_ISP_AddSensor(sensor_info);
	if(ret < 0){
		log_error("Failed to register the %s sensor.", sensor_name);
		exit(-1);
	}
  else {
    log_info("Added the %s sensor.", sensor_name);
  }

	ret = IMP_ISP_EnableSensor();
	if(ret < 0){
		log_error("Failed to EnableSensor");
		return -1;
	}


	ret = IMP_System_Init();
	if(ret < 0){
		log_error("IMP_System_Init failed");
		return -1;
	}



	/* enable tuning, to debug graphics */

	ret = IMP_ISP_EnableTuning();
	if(ret < 0){
		log_error("IMP_ISP_EnableTuning failed\n");
		return -1;
	}

  ret = IMP_ISP_Tuning_SetWDRAttr(IMPISP_TUNING_OPS_MODE_DISABLE);
  if(ret < 0){
    log_error("failed to set WDR\n");
    return -1;
  }


	log_info("Sensor succesfully initialized.");

	return 0;
}



int configure_video_tuning_parameters(CameraConfig *camera_config)
{
  log_info("Configuring video tuning parameters");

  // Sharpness
  IMP_ISP_Tuning_SetSharpness(50);

  // Image flipping
  if (camera_config->flip_vertical) {
    log_info("Flipping image vertically");
    IMP_ISP_Tuning_SetISPVflip(IMPISP_TUNING_OPS_MODE_ENABLE);
  }
  else {
    IMP_ISP_Tuning_SetISPVflip(IMPISP_TUNING_OPS_MODE_DISABLE);    
  }

  if (camera_config->flip_horizontal) {
    log_info("Flipping image horizontally");
    IMP_ISP_Tuning_SetISPHflip(IMPISP_TUNING_OPS_MODE_ENABLE);
  }
  else {
    IMP_ISP_Tuning_SetISPHflip(IMPISP_TUNING_OPS_MODE_DISABLE);    
  }

  return 0;
}

int initialize_audio()
{
  int ret;
  int device_id = 1;
  int audio_channel_id = 0;

  IMPAudioIOAttr audio_settings;
  IMPAudioIChnParam audio_channel_params;

  log_info("Initializing audio settings");

  audio_settings.samplerate = AUDIO_SAMPLE_RATE_48000;
  audio_settings.bitwidth = AUDIO_BIT_WIDTH_16;
  audio_settings.soundmode = AUDIO_SOUND_MODE_MONO; 

  // Number of audio frames to cache (max is 50) 
  audio_settings.frmNum = MAX_AUDIO_FRAME_NUM;

  // Number of sampling points per frame
  audio_settings.numPerFrm = 1920;
  audio_settings.chnCnt = 1;



  // ALSA
  snd_pcm_hw_params_t *pcm_hw_params;
  snd_pcm_uframes_t pcm_frames;
  int sample_rate, audio_channels;




  ret = IMP_AI_SetPubAttr(device_id, &audio_settings);

  if(ret < 0){
    log_error("Error in setting attributes for audio encoder\n");
    return -1;
  }

  log_info("Sample rate: %d", audio_settings.samplerate);
  log_info("Bit width: %d", audio_settings.bitwidth);
  log_info("Sound mode: %d", audio_settings.soundmode);
  log_info("Max frames to cache: %d", audio_settings.frmNum);
  log_info("Samples per frame: %d", audio_settings.numPerFrm);

  /* Step 2: enable AI device. */
  ret = IMP_AI_Enable(device_id);
  if(ret != 0) {
    log_error("Error enabling the audio device: %d", device_id);
    return -1;
  }


  // Set audio channel attributes of device

  // Audio frame buffer depth
  audio_channel_params.usrFrmDepth = 20;

  ret = IMP_AI_SetChnParam(device_id, audio_channel_id, &audio_channel_params);
  if(ret != 0) {
    log_error("Error setting the audio channel parameters for device %d", device_id);
    return -1;
  }

  // Step 4: enable AI channel.
  ret = IMP_AI_EnableChn(device_id, audio_channel_id);
  if(ret != 0) {
    log_error("Error enabling audio channel");
    return -1;
  }

  /* Step 5: Set audio channel volume. */
  ret = IMP_AI_SetVol(device_id, audio_channel_id, 70);
  if(ret != 0) {
    log_error("Error setting the audio channel volume");
    return -1;
  }  


  // Enable hardware noise suppression
  // 4 levels to pick from:
  //
  // NS_LOW  
  // NS_MODERATE
  // NS_HIGH
  // NS_VERYHIGH
  ret = IMP_AI_EnableNs(&audio_settings, NS_HIGH);
  if(ret != 0) {
    log_error("Error enable hardware noise suppression.");
    return -1;
  }  

  // ALSA loopback device setup
  // Found good sample code here: https://gist.github.com/ghedo/963382/98f730d61dad5b6fdf0c4edb7a257c5f9700d83b

  // The last argument of 0 indicates I want blocking mode
  ret = snd_pcm_open(&pcm_handle, "hw:0,0", SND_PCM_STREAM_PLAYBACK, 0);
  if(ret != 0) {
    log_error("Error opening ALSA PCM loopback device.");
    return -1;
  }



  /* Allocate parameters object and fill it with default values*/
  snd_pcm_hw_params_alloca(&pcm_hw_params);
  snd_pcm_hw_params_any(pcm_handle, pcm_hw_params);


  audio_channels = 1;
  sample_rate = 48000;


  /* Set parameters */
  if (ret = snd_pcm_hw_params_set_access(pcm_handle, pcm_hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {    
    log_error("ERROR: Can't set interleaved mode. %s\n", snd_strerror(ret));
  }

  if (ret = snd_pcm_hw_params_set_format(pcm_handle, pcm_hw_params, SND_PCM_FORMAT_S16_LE) < 0)  {
    log_error("ERROR: Can't set format. %s\n", snd_strerror(ret));
  }

  if (ret = snd_pcm_hw_params_set_channels(pcm_handle, pcm_hw_params, audio_channels) < 0) {
    log_error("ERROR: Can't set channels number. %s\n", snd_strerror(ret));
  }

  if (ret = snd_pcm_hw_params_set_rate_near(pcm_handle, pcm_hw_params, &sample_rate, 0) < 0) {    
    log_error("ERROR: Can't set rate. %s\n", snd_strerror(ret));  
  }


  // snd_pcm_uframes_t frames = 1920;

  // if (ret = snd_pcm_hw_params_set_period_size(pcm_handle, pcm_hw_params, frames, 0) < 0) {
  //   log_error("ERROR: Can't set period size. %s\n", snd_strerror(ret));
  // }



  /* Set number of periods. Periods used to be called fragments. */ 
  // if (ret = snd_pcm_hw_params_set_periods(pcm_handle, pcm_hw_params, 2, 0) < 0) {
  //   log_error("ERROR: Can't set periods. %s\n", snd_strerror(ret));
  // }

  // /* Set buffer size (in frames). The resulting latency is given by */
  // /* latency = periodsize * periods / (rate * bytes_per_frame)     */
  // if (ret = snd_pcm_hw_params_set_buffer_size(pcm_handle, pcm_hw_params, 960 * 2) < 0) {
  //   log_error("ERROR: Can't set buffer size. %s\n", snd_strerror(ret));  
  // }

  /* Write parameters */
  if (ret = snd_pcm_hw_params(pcm_handle, pcm_hw_params) < 0) {    
    log_error("ERROR: Can't set hardware parameters. %s\n", snd_strerror(ret));
  }



  log_info("PCM name: %s", snd_pcm_name(pcm_handle));
  log_info("PCM state: %s", snd_pcm_state_name(snd_pcm_state(pcm_handle)));

  snd_pcm_hw_params_get_channels(pcm_hw_params, &audio_channels);
  log_info("PCM channels: %d", audio_channels);

  snd_pcm_hw_params_get_rate(pcm_hw_params, &sample_rate, 0);
  log_info("PCM sample rate: %d bps", sample_rate);


  log_info("Audio initialization complete");

  // initialize_capture_side_audio();

  return 0;
}


int create_encoding_group(int group_id)
{
  // One group only supports one resolution, and different resolutions
  // need to start a new group. A Group supports both H264 and JPEG
  // capture formats

  int ret;

  ret = IMP_Encoder_CreateGroup(group_id);
  if (ret < 0) {
    log_warn("IMP_Encoder_CreateGroup(%d) error. Can be ignored if already created.", group_id);
    return -1;
  }
  else {
    log_info("Created encoding group %d", group_id);
  }

  return 0;
}



int setup_encoding_engine(FrameSource* frame_source, EncoderSetting *encoder_setting)
{
  int i, ret;
  IMPEncoderAttr *enc_attr;
  IMPEncoderRcAttr *rc_attr;
  IMPFSChnAttr *imp_chn_attr_tmp;
  IMPEncoderCHNAttr channel_attr;

  // imp_chn_attr_tmp = &chn[i].fs_chn_attr;

  memset(&channel_attr, 0, sizeof(IMPEncoderCHNAttr));
  enc_attr = &channel_attr.encAttr;

  if (strcmp(encoder_setting->payload_type, "PT_H264") == 0) {
    enc_attr->enType = PT_H264;
  }
  else if(strcmp(encoder_setting->payload_type, "PT_JPEG") == 0) {
    enc_attr->enType = PT_JPEG;
  }
  else {
    log_error("Unknown payload type: %s", encoder_setting->payload_type);
    return -1;
  }

  enc_attr->bufSize = encoder_setting->buffer_size;
  enc_attr->profile = encoder_setting->profile;
  enc_attr->picWidth = frame_source->pic_width;
  enc_attr->picHeight = frame_source->pic_height;
  rc_attr = &channel_attr.rcAttr;

  if (strcmp(encoder_setting->mode, "ENC_RC_MODE_H264VBR") == 0) {
    rc_attr->rcMode = ENC_RC_MODE_H264VBR;
  }
  else if (strcmp(encoder_setting->mode, "MJPEG") == 0) {
    rc_attr->rcMode = 0;
  }
  else {
    log_error("Unknown encoding mode: %s", encoder_setting->mode);
  }


  rc_attr->attrH264Vbr.outFrmRate.frmRateNum = encoder_setting->frame_rate_numerator;
  rc_attr->attrH264Vbr.outFrmRate.frmRateDen = encoder_setting->frame_rate_denominator;
  rc_attr->attrH264Vbr.maxGop = encoder_setting->max_group_of_pictures;
  rc_attr->attrH264Vbr.maxQp = encoder_setting->max_qp;
  rc_attr->attrH264Vbr.minQp = encoder_setting->min_qp;
  
  // rc_attr->attrH264Vbr.staticTime = stream_settings->statistics_interval;
  // rc_attr->attrH264Vbr.maxBitRate = stream_settings->max_bitrate;
  // rc_attr->attrH264Vbr.changePos = stream_settings->change_pos;

  rc_attr->attrH264Vbr.staticTime = 1;
  rc_attr->attrH264Vbr.maxBitRate = 500;
  rc_attr->attrH264Vbr.changePos = 50;


  rc_attr->attrH264Vbr.FrmQPStep = encoder_setting->frame_qp_step;
  rc_attr->attrH264Vbr.GOPQPStep = encoder_setting->gop_qp_step;
  rc_attr->attrH264FrmUsed.enable = 1;


  log_info("Encoder channel attributes for channel %d", encoder_setting->channel);
  print_encoder_channel_attributes(&channel_attr);


  ret = IMP_Encoder_CreateChn(encoder_setting->channel, &channel_attr);
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

void print_channel_attributes(IMPFSChnAttr *attr)
{
  char buffer[1024];  
  snprintf(buffer, sizeof(buffer), "IMPFSChnAttr: \n"
                   "picWidth: %d\n"
                   "picHeight: %d\n"
                   "outFrmRateNum: %d\n"
                   "outFrmRateDen: %d\n"
                   "nrVBs: %d\n",
                    attr->picWidth,
                    attr->picHeight,
                    attr->outFrmRateNum,
                    attr->outFrmRateDen,
                    attr->nrVBs
                    );
  log_info("%s", buffer);
}


void print_encoder_channel_attributes(IMPEncoderCHNAttr *attr)
{
  IMPEncoderAttr *enc_attr;
  IMPEncoderRcAttr *rc_attr;  
  char buffer[4096];
  char payload_type[32];

  enc_attr = &attr->encAttr;
  rc_attr = &attr->rcAttr;


  switch(enc_attr->enType) {
    case PT_H264:
      strcpy(payload_type, "PT_H264");
      break;
    case PT_JPEG:
      strcpy(payload_type, "PT_JPEG");
      break;
    default:
      strcpy(payload_type, "ERROR_UNKNOWN");
  }

  snprintf(buffer, sizeof(buffer), "IMPEncoderCHNAttr: \n"
                   "payload_type: %s\n"
                   "bufSize: %u\n"
                   "profile: %u\n"
                   "picWidth: %u\n"
                   "picHeight: %u\n"
                   "attrH264Vbr.outFrmRate.frmRateNum: %u\n"
                   "attrH264Vbr.outFrmRate.frmRateDen: %u\n"
                   "attrH264Vbr.maxGop: %u\n"
                   "attrH264Vbr.maxQp: %u\n"
                   "attrH264Vbr.minQp: %u\n"
                   "attrH264Vbr.staticTime: %u\n"
                   "attrH264Vbr.maxBitRate: %u\n"
                   "attrH264Vbr.changePos: %u\n"
                   "attrH264Vbr.FrmQPStep: %u\n"
                   "attrH264Vbr.GOPQPStep: %u\n",
                    payload_type,
                    enc_attr->bufSize,
                    enc_attr->profile,
                    enc_attr->picWidth,
                    enc_attr->picHeight,
                    rc_attr->attrH264Vbr.outFrmRate.frmRateNum,
                    rc_attr->attrH264Vbr.outFrmRate.frmRateDen,
                    rc_attr->attrH264Vbr.maxGop,
                    rc_attr->attrH264Vbr.maxQp,
                    rc_attr->attrH264Vbr.minQp,
                    rc_attr->attrH264Vbr.staticTime,
                    rc_attr->attrH264Vbr.maxBitRate,
                    rc_attr->attrH264Vbr.changePos,
                    rc_attr->attrH264Vbr.FrmQPStep,
                    rc_attr->attrH264Vbr.GOPQPStep
                    );
  log_info("%s", buffer);
}


void print_stream_settings(StreamSettings *stream_settings)
{
  char buffer[1024];  
  snprintf(buffer, sizeof(buffer), "Stream settings: \n"
                   "name: %s\n"
                   "enabled: %d\n"
                   "pic_width: %d\n"
                   "pic_height: %d\n"
                   "statistics_interval: %d\n"
                   "max_bitrate: %d\n"
                   "change_pos: %d\n"
                   "group: %d\n",
                    stream_settings->name,
                    stream_settings->enabled,
                    stream_settings->pic_width,
                    stream_settings->pic_height,
                    stream_settings->statistics_interval,
                    stream_settings->max_bitrate,
                    stream_settings->change_pos,
                    stream_settings->group
                    );
  log_info("%s", buffer);
}



int initialize_osd()
{
  int ret = 0;
  int osdGroupNumber = 0;
  IMPOSDGrpRgnAttr grAttrFont;
  

  log_info("Initializing on screen display");

  osdRegion = IMP_OSD_CreateRgn(NULL);
  if (osdRegion < 0 ) {
    log_error("IMP_OSD_CreateRgn failed");
    return -1;
  }

  ret = IMP_OSD_RegisterRgn(osdRegion, 0, NULL);
  if (ret < 0) {
    log_error("IMP_OSD_RegisterRgn failed");
    return -1;
  }

  IMPOSDRgnAttr rAttrFont;
  memset(&rAttrFont, 0, sizeof(IMPOSDRgnAttr));
  rAttrFont.type = OSD_REG_PIC;
  rAttrFont.rect.p0.x = 10;
  rAttrFont.rect.p0.y = 10;
  rAttrFont.rect.p1.x = rAttrFont.rect.p0.x + 20 * OSD_REGION_WIDTH - 1;   //p0 is start，and p1 well be epual p0+width(or heigth)-1
  rAttrFont.rect.p1.y = rAttrFont.rect.p0.y + OSD_REGION_HEIGHT - 1;
  rAttrFont.fmt = PIX_FMT_BGRA;
  rAttrFont.data.picData.pData = NULL;

  ret = IMP_OSD_SetRgnAttr(osdRegion, &rAttrFont);
  if (ret < 0) {
    log_error("IMP_OSD_SetRgnAttr failed");
    return -1;
  }


  // if (IMP_OSD_GetGrpRgnAttr(osdRegion, osdGroupNumber, &grAttrFont) < 0) {
  //   log_error("IMP_OSD_GetGrpRgnAttr failed");
  //   return -1;
  // }
  // memset(&grAttrFont, 0, sizeof(IMPOSDGrpRgnAttr));
  // grAttrFont.show = 0;

  // // Disable Font global alpha, only use pixel alpha.
  // grAttrFont.gAlphaEn = 1;
  // grAttrFont.fgAlhpa = 0xff;
  // grAttrFont.layer = 3;
  // if (IMP_OSD_SetGrpRgnAttr(osdRegion, osdGroupNumber, &grAttrFont) < 0) {
  //   log_error("IMP_OSD_SetGrpRgnAttr failed");
  //   return -1;
  // }

  ret = IMP_OSD_Start(osdGroupNumber);
  if (ret < 0) {
    log_error("IMP_OSD_Start failed");
    return -1;
  }

}

// This is the entrypoint for the timestamp OSD thread
void *timestamp_osd_entry_start(void *timestamp_osd_thread_params)
{
  int ret;

  CameraConfig *camera_config = (CameraConfig *)timestamp_osd_thread_params;

  /*generate time*/
  char DateStr[40];
  time_t currTime;
  struct tm *currDate;
  unsigned i = 0, j = 0;
  void *dateData = NULL;
  uint32_t *timeStampData;

  IMPOSDRgnAttrData rAttrData;

  initialize_osd();

  int groupNumber = 0;

  if (camera_config->show_timestamp <= 0) {
    log_info("On screen timestamps not configured.");
    return NULL;
  }

  ret = IMP_OSD_ShowRgn(osdRegion, groupNumber, 1);
  if( ret < 0) {
    log_error("IMP_OSD_ShowRgn failed");
    return NULL;
  }

  timeStampData = malloc(20 * OSD_REGION_HEIGHT * OSD_REGION_WIDTH * 4);


  while(!sigint_received) {
      int penpos_t = 0;
      int fontadv = 0;

      time(&currTime);
      currDate = localtime(&currTime);
      memset(DateStr, 0, 40);
      strftime(DateStr, 40, "%Y-%m-%d %H:%M:%S", currDate);
      for (i = 0; i < 20; i++) {
        switch(DateStr[i]) {
          case '0' ... '9':
            dateData = (void *)gBgramap[DateStr[i] - '0'].pdata;
            fontadv = gBgramap[DateStr[i] - '0'].width;
            penpos_t += gBgramap[DateStr[i] - '0'].width;
            break;
          case '-':
            dateData = (void *)gBgramap[10].pdata;
            fontadv = gBgramap[10].width;
            penpos_t += gBgramap[10].width;
            break;
          case ' ':
            dateData = (void *)gBgramap[11].pdata;
            fontadv = gBgramap[11].width;
            penpos_t += gBgramap[11].width;
            break;
          case ':':
            dateData = (void *)gBgramap[12].pdata;
            fontadv = gBgramap[12].width;
            penpos_t += gBgramap[12].width;
            break;
          default:
            break;
        }

        for (j = 0; j < OSD_REGION_HEIGHT; j++) {
          memcpy((void *)((uint32_t *)timeStampData + j*20*OSD_REGION_WIDTH + penpos_t),
              (void *)((uint32_t *)dateData + j*fontadv), fontadv*4);
        }
      }
      rAttrData.picData.pData = timeStampData;
      IMP_OSD_UpdateRgnAttrData(osdRegion, &rAttrData);
      // log_info("Updated osdRegion to: %s", DateStr);

      sleep(1);
  }

  free(timeStampData);

  return NULL;

}


// This is the entrypoint for the audio thread
void *audio_thread_entry_start(void *audio_thread_params)
{
  int ret;

  // Audio device
  int audio_device_id = 1;
  int audio_channel_id = 0;
  IMPAudioFrame audio_frame;
  int num_samples;
  short pcm_audio_data[1920 * 2];

  while(!sigint_received) {

    ret = IMP_AI_PollingFrame(audio_device_id, audio_channel_id, 1000);
    if (ret < 0) {
      log_error("Error or timeout polling for audio frame");
      pthread_exit(NULL);
    }

    ret = IMP_AI_GetFrame(audio_device_id, audio_channel_id, &audio_frame, BLOCK);
    if (ret < 0) {
      log_error("Error getting audio frame data");
      pthread_exit(NULL);
    }

    num_samples = audio_frame.len / sizeof(short);

    memcpy(pcm_audio_data, (void *)audio_frame.virAddr, audio_frame.len );

    ret = IMP_AI_ReleaseFrame(audio_device_id, audio_channel_id, &audio_frame);
    if(ret != 0) {
      log_error("Error releasing audio frame");
      pthread_exit(NULL);
    }

    ret = snd_pcm_writei(pcm_handle, pcm_audio_data, num_samples);

    if ( ret == -EPIPE) {
      log_error("Buffer underrun when writing to ALSA loopback device");
      snd_pcm_prepare(pcm_handle);
    }

    if (ret < 0) {
      log_error("ERROR. Can't write to PCM device. %s\n", snd_strerror(ret));
    }

  }
}



// This is the entrypoint for the threads
void *produce_frames(void *encoder_thread_params_ptr)
{
  int ret, i;
  EncoderThreadParams *encoder_thread_params = encoder_thread_params_ptr;

  // Unpack the EncoderThreadParams
  EncoderSetting *encoder = encoder_thread_params->encoder;

  log_info("Starting thread for encoder");

  output_v4l2_frames(encoder);

}

int output_v4l2_frames(EncoderSetting *encoder_setting)
{
  int ret;
  int stream_packets;
  int i;
  int total;
  char *v4l2_device_path = encoder_setting->v4l2_device_path;
  int video_width = encoder_setting->pic_width;
  int video_height = encoder_setting->pic_height;

  int frames_written = 0;
  float current_fps = 0;
  float elapsed_seconds = 0;
  struct timeval tval_before, tval_after, tval_result;
  float delay_in_seconds = 0;
  float adjusted_delay_in_seconds = 0;


  struct v4l2_capability vid_caps;
  struct v4l2_format vid_format;

  IMPEncoderStream stream;

  uint8_t *stream_chunk;
  uint8_t *temp_chunk;


  // h264 NAL unit stuff

  // h264_stream_t *h = h264_new();
  // int nal_start, nal_end;
  // uint8_t* buf;
  // int len;




  delay_in_seconds = (1.0 * encoder_setting->frame_rate_denominator) / encoder_setting->frame_rate_numerator;
  log_info("Delay in seconds: %f", delay_in_seconds);






  log_info("Opening V4L2 device: %s ", v4l2_device_path);
  int v4l2_fd = open(v4l2_device_path, O_WRONLY, 0777);

  if (v4l2_fd < 0) {
    log_error("Failed to open V4L2 device: %s", v4l2_device_path);
    return -1;
  }


  // ret = ioctl(v4l2_fd, VIDIOC_QUERYCAP, &vid_caps);

  memset(&vid_format, 0, sizeof(vid_format));
  vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  vid_format.fmt.pix.width = video_width;
  vid_format.fmt.pix.height = video_height;

  if (strcmp(encoder_setting->payload_type, "PT_H264") == 0) {
    vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
    vid_format.fmt.pix.sizeimage = 0;
    vid_format.fmt.pix.field = V4L2_FIELD_NONE;
    vid_format.fmt.pix.bytesperline = 0;
    vid_format.fmt.pix.colorspace = V4L2_PIX_FMT_YUV420;
  }
  else if(strcmp(encoder_setting->payload_type, "PT_JPEG") == 0) {
    vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
    // TODO: Is this correct? Doc says needs to be set to maximum size of image
    vid_format.fmt.pix.sizeimage = 0; 
    vid_format.fmt.pix.field = V4L2_FIELD_NONE;
    vid_format.fmt.pix.bytesperline = 0;
    vid_format.fmt.pix.colorspace = V4L2_COLORSPACE_JPEG;
  }
  else {
    log_error("Unknown payload type: %s", encoder_setting->payload_type);
    return -1;
  }


  ret = ioctl(v4l2_fd, VIDIOC_S_FMT, &vid_format);
  if (ret < 0) {
    log_error("Unable to set V4L2 device video format: %d", ret);
    return -1;
  }

  ret = ioctl(v4l2_fd, VIDIOC_STREAMON, &vid_format);
  if (ret < 0) {
    log_error("Unable to perform VIDIOC_STREAMON: %d", ret);
    return -1;
  }

  log_info("V4L2 device opened and setup complete: VIDIOC_STREAMON");
  

  log_info("Sleeping 2 seconds before starting to send frames...");


  ret = IMP_Encoder_StartRecvPic(encoder_setting->channel);
  if (ret < 0) {
    log_error("IMP_Encoder_StartRecvPic(%d) failed.", encoder_setting->channel);
    return -1;
  }

  // Every set number of frames calculate out how many frames per second we are getting
  current_fps = 0;
  frames_written = 0;
  gettimeofday(&tval_before, NULL);


  // int samples_file;
  // samples_file = fopen("/tmp/samples.pcm", "w");

  clock_t start, end;
  double cpu_time_used;


  while(!sigint_received) {
    start = clock();

    // Video Frames
    if (frames_written == 200) {
      gettimeofday(&tval_after, NULL);
      timersub(&tval_after, &tval_before, &tval_result);

      elapsed_seconds =  (long int)tval_result.tv_sec + ((long int)tval_result.tv_usec / 1000000);

      current_fps = 200 / elapsed_seconds;
      log_info("Current FPS: %.2f / Channel %d", current_fps, encoder_setting->channel);

      // if (strcmp(v4l2_device_path, "/dev/video3") == 0) {
      //   log_info("Obtained %d 16-bit samples from this specific audio frame", num_samples);
      // }

      // IMPEncoderCHNStat encoder_status;

      // IMP_Encoder_Query(encoder_setting->channel, &encoder_status);

      // log_info("Registered: %u", encoder_status.registered);
      // log_info("Work done (0 is running, 1 is not running): %u", encoder_status.work_done);
      // log_info("Number of images to be encoded: %u", encoder_status.leftPics);
      // log_info("Number of bytes remaining in the stream buffer: %u", encoder_status.leftStreamBytes);

      frames_written = 0;
      gettimeofday(&tval_before, NULL);
    }


    ret = IMP_Encoder_PollingStream(encoder_setting->channel, 1000);
    if (ret < 0) {
      log_error("Timeout while polling for stream on channel %d.", encoder_setting->channel);
      continue;
    }


    // Get H264 Stream on channel and enable a blocking call
    ret = IMP_Encoder_GetStream(encoder_setting->channel, &stream, 1);
    if (ret < 0) {
      log_error("IMP_Encoder_GetStream() failed");
      return -1;
    }

    stream_packets = stream.packCount;

    total = 0;
    stream_chunk = malloc(1);
    if (stream_chunk == NULL) {
      log_error("Malloc returned NULL.");
      return -1;
    }

    for (i = 0; i < stream_packets; i++) {
      log_debug("Processing packet %d of size %d.", total, i, stream.pack[i].length);

      temp_chunk = realloc(stream_chunk, total + stream.pack[i].length);

      if (temp_chunk == NULL) {
        log_error("realloc returned NULL for request of size: %d", total);
        return -1;
      }

      log_debug("Allocated an additional %d bytes for packet %d.", total + stream.pack[i].length, i);

      // Allocating worked
      stream_chunk = temp_chunk;
      temp_chunk = NULL;

      memcpy(&stream_chunk[total], (void *)stream.pack[i].virAddr, stream.pack[i].length);
      total = total + stream.pack[i].length;

      log_debug("Total size of chunk after concatenating: %d bytes.", total);
    }

    // Write out to the V4L2 device (for example /dev/video0)
    ret = write(v4l2_fd, (void *)stream_chunk, total);
    if (ret != total) {
      log_error("Stream write error: %s", ret);
      return -1;
    }
  
    free(stream_chunk);

    IMP_Encoder_ReleaseStream(encoder_setting->channel, &stream);

    frames_written = frames_written + 1;

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    adjusted_delay_in_seconds = delay_in_seconds - cpu_time_used;
    usleep(1000 * 1000 * delay_in_seconds);

  }


  ret = IMP_Encoder_StopRecvPic(encoder_setting->channel);
  if (ret < 0) {
    log_error("IMP_Encoder_StopRecvPic(%d) failed", encoder_setting->channel);
    return -1;
  }


}

int sensor_cleanup(IMPSensorInfo *sensor_info)
{
  int ret = 0;

  log_info("Cleaning up sensor.");

  IMP_System_Exit();

  ret = IMP_ISP_DisableSensor();
  if(ret < 0){
    log_error("failed to EnableSensor");
    return -1;
  }

  ret = IMP_ISP_DelSensor(sensor_info);
  if(ret < 0){
    log_error("failed to AddSensor");
    return -1;
  }

  ret = IMP_ISP_DisableTuning();
  if(ret < 0){
    log_error("IMP_ISP_DisableTuning failed");
    return -1;
  }

  if(IMP_ISP_Close()){
    log_error("failed to open ISP");
    return -1;
  }

  log_info("Sensor cleanup success.");

  return 0;
}
