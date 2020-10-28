#include "capture.h"

/*

ISP - Ingenic Smart Platform
IMP - Ingenic Multimedia Platform

*/

extern sig_atomic_t sigint_received;

extern snd_pcm_t *pcm_handle;

int initialize_sensor(IMPSensorInfo *sensor_info)
{
  int ret;

  log_info("Initializing sensor");

  memset(sensor_info, 0, sizeof(IMPSensorInfo));
	memcpy(sensor_info->name, SENSOR_NAME, sizeof(SENSOR_NAME));
	sensor_info->cbus_type = SENSOR_CUBS_TYPE;
	memcpy(sensor_info->i2c.type, SENSOR_NAME, sizeof(SENSOR_NAME));
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
		log_error("Failed to register the %s sensor.", SENSOR_NAME);
		return -1;
	}
  else {
    log_info("Added the %s sensor.", SENSOR_NAME);    
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
  audio_settings.numPerFrm = 1000;
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



  // ALSA loopback device setup
  // Found good sample code here: https://gist.github.com/ghedo/963382/98f730d61dad5b6fdf0c4edb7a257c5f9700d83b

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

  /* Write parameters */
  if (ret = snd_pcm_hw_params(pcm_handle, pcm_hw_params) < 0) {    
    log_error("ERROR: Can't set harware parameters. %s\n", snd_strerror(ret));
  }



  log_info("PCM name: %s", snd_pcm_name(pcm_handle));
  log_info("PCM state: %s", snd_pcm_state_name(snd_pcm_state(pcm_handle)));

  snd_pcm_hw_params_get_channels(pcm_hw_params, &audio_channels);
  log_info("PCM channels: %d", audio_channels);

  snd_pcm_hw_params_get_rate(pcm_hw_params, &sample_rate, 0);
  log_info("PCM sample rate: %d bps", sample_rate);


  log_info("Audio initialization complete");

  return 0;
}


int setup_framesource(StreamSettings* stream_settings)
{
  int ret;

  IMPFSChnAttr fs_chn_attr = {
    .pixFmt = PIX_FMT_NV12,
    .outFrmRateNum = stream_settings->frame_rate_numerator,
    .outFrmRateDen = stream_settings->frame_rate_denominator,
    .nrVBs = 3,
    .type = FS_PHY_CHANNEL,

    .crop.enable = 0,
    .crop.top = 0,
    .crop.left = 0,
    .crop.width = stream_settings->pic_width,
    .crop.height = stream_settings->pic_height,

    .scaler.enable = 1,
    .scaler.outwidth = stream_settings->pic_width,
    .scaler.outheight = stream_settings->pic_height,
    .picWidth = stream_settings->pic_width,
    .picHeight = stream_settings->pic_height
  };

  log_info("Setting up frame source for stream: %s", stream_settings->name);


  ret = IMP_FrameSource_CreateChn(stream_settings->channel, &fs_chn_attr);
  if(ret < 0){
    log_error("IMP_FrameSource_CreateChn error.");
    return -1;
  }

  ret = IMP_FrameSource_SetChnAttr(stream_settings->channel, &fs_chn_attr);
  if (ret < 0) {
    log_error("IMP_FrameSource_SetChnAttr error.");
    return -1;
  }

  ret = IMP_Encoder_CreateGroup(stream_settings->group);
  if (ret < 0) {
    log_error("IMP_Encoder_CreateGroup error.");
    return -1;
  }

  log_info("Frame source setup complete: %s", stream_settings->name);

  return 0;
}

int setup_encoding_engine(StreamSettings* stream_settings)
{
  int i, ret;
  IMPEncoderAttr *enc_attr;
  IMPEncoderRcAttr *rc_attr;
  IMPFSChnAttr *imp_chn_attr_tmp;
  IMPEncoderCHNAttr channel_attr;

  // imp_chn_attr_tmp = &chn[i].fs_chn_attr;


  memset(&channel_attr, 0, sizeof(IMPEncoderCHNAttr));
  enc_attr = &channel_attr.encAttr;

  if (strcmp(stream_settings->payload_type, "PT_H264") == 0) {
    enc_attr->enType = PT_H264;
  }
  else if(strcmp(stream_settings->payload_type, "PT_JPEG") == 0) {
    enc_attr->enType = PT_JPEG;
  }
  else {
    log_error("Unknown payload type: %s", stream_settings->payload_type);
    return -1;
  }

  enc_attr->bufSize = stream_settings->buffer_size;
  enc_attr->profile = stream_settings->profile;
  enc_attr->picWidth = stream_settings->pic_width;
  enc_attr->picHeight = stream_settings->pic_height;
  rc_attr = &channel_attr.rcAttr;



  if (strcmp(stream_settings->mode, "ENC_RC_MODE_H264VBR") == 0) {
    rc_attr->rcMode = ENC_RC_MODE_H264VBR;
  }
  else if (strcmp(stream_settings->mode, "MJPEG") == 0) {
    rc_attr->rcMode = 0;
  }
  else {
    log_error("Unknown encoding mode: %s", stream_settings->mode);
  }


  rc_attr->attrH264Vbr.outFrmRate.frmRateNum = stream_settings->frame_rate_numerator;
  rc_attr->attrH264Vbr.outFrmRate.frmRateDen = stream_settings->frame_rate_denominator;
  rc_attr->attrH264Vbr.maxGop = stream_settings->max_group_of_pictures;
  rc_attr->attrH264Vbr.maxQp = stream_settings->max_qp;
  rc_attr->attrH264Vbr.minQp = stream_settings->min_qp;
  rc_attr->attrH264Vbr.staticTime = stream_settings->statistics_interval;
  rc_attr->attrH264Vbr.maxBitRate = stream_settings->max_bitrate;
  rc_attr->attrH264Vbr.changePos = stream_settings->change_pos;
  rc_attr->attrH264Vbr.FrmQPStep = stream_settings->frame_qp_step;
  rc_attr->attrH264Vbr.GOPQPStep = stream_settings->gop_qp_step;
  rc_attr->attrH264FrmUsed.enable = 1;


  ret = IMP_Encoder_CreateChn(stream_settings->channel, &channel_attr);
  if (ret < 0) {
    log_error("IMP_Encoder_CreateChn error for stream %s, channel %d",
      stream_settings->name, stream_settings->channel);      
    return -1;
  }

  log_info("IMP_Encoder_CreateChn success for stream %s, channel %d",
    stream_settings->name, stream_settings->channel);

  ret = IMP_Encoder_RegisterChn(stream_settings->group, stream_settings->channel);
  if (ret < 0) {
    log_error("IMP_Encoder_RegisterChn error.");
    return -1;
  }

  return 0;

}

void print_stream_settings(StreamSettings *stream_settings)
{
  char buffer[1024];  
  snprintf(buffer, sizeof(buffer), "Stream settings: \n"
                   "name: %s\n"
                   "v4l2_device_path: %s\n"
                   "payload_type: %s\n"
                   "buffer_size: %d\n"
                   "profile: %d\n"
                   "pic_width: %d\n"
                   "pic_height: %d\n"
                   "mode: %s\n"
                   "frame_rate_numerator: %d\n"
                   "frame_rate_denominator: %d\n"
                   "max_group_of_pictures: %d\n"
                   "max_qp: %d\n"
                   "min_qp: %d\n"
                   "statistics_interval: %d\n"
                   "max_bitrate: %d\n"
                   "change_pos: %d\n"
                   "frame_qp_step: %d\n"
                   "gop_qp_step: %d\n"
                   "channel: %d\n"
                   "group: %d\n",
                    stream_settings->name,
                    stream_settings->v4l2_device_path,
                    stream_settings->payload_type,
                    stream_settings->buffer_size,
                    stream_settings->profile,
                    stream_settings->pic_width,
                    stream_settings->pic_height,
                    stream_settings->mode,
                    stream_settings->frame_rate_numerator,
                    stream_settings->frame_rate_denominator,
                    stream_settings->max_group_of_pictures,
                    stream_settings->max_qp,
                    stream_settings->min_qp,
                    stream_settings->statistics_interval,
                    stream_settings->max_bitrate,
                    stream_settings->change_pos,
                    stream_settings->frame_qp_step,
                    stream_settings->gop_qp_step,
                    stream_settings->channel,
                    stream_settings->group
                    );
  log_info("%s", buffer);
}


void *produce_frames(void *ptr)
{
  // ptr is a pointer to StreamSettings
  StreamSettings *stream_settings = ptr;
  print_stream_settings(stream_settings);

  setup_framesource(stream_settings);

  setup_encoding_engine(stream_settings);

  output_v4l2_frames(stream_settings);

}

int output_v4l2_frames(StreamSettings *stream_settings)
{
  int ret;
  int stream_packets;
  int i;
  int total;
  char *v4l2_device_path = stream_settings->v4l2_device_path;
  int video_width = stream_settings->pic_width;
  int video_height = stream_settings->pic_height;

  int frames_written = 0;
  float current_fps = 0;
  float elapsed_seconds = 0;
  struct timeval tval_before, tval_after, tval_result;


  IMPCell framesource_chn = { DEV_ID_FS, stream_settings->group, 0};
  IMPCell imp_encoder = { DEV_ID_ENC, stream_settings->group, 0};

  struct v4l2_capability vid_caps;
  struct v4l2_format vid_format;

  IMPEncoderStream stream;

  uint8_t *stream_chunk;
  uint8_t *temp_chunk;

  // Audio device
  int audio_device_id = 1;
  int audio_channel_id = 0;
  IMPAudioFrame audio_frame;
  int num_samples;
  short pcm_audio_data[1024];

  // h264 NAL unit stuff
  h264_stream_t *h = h264_new();
  int nal_start, nal_end;
  uint8_t* buf;
  int len;


  ret = IMP_System_Bind(&framesource_chn, &imp_encoder);
  if (ret < 0) {
    log_error("Error binding frame source to encoder for stream %s", stream_settings->name);
    return -1;
  }

  ret = IMP_FrameSource_EnableChn(stream_settings->channel);
  if (ret < 0) {
    log_error("IMP_FrameSource_EnableChn(%d) error: %d", stream_settings->channel, ret);
    return -1;
  }


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

  if (strcmp(stream_settings->payload_type, "PT_H264") == 0) {
    vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
    vid_format.fmt.pix.sizeimage = 0;
    vid_format.fmt.pix.field = V4L2_FIELD_NONE;
    vid_format.fmt.pix.bytesperline = 0;
    vid_format.fmt.pix.colorspace = V4L2_PIX_FMT_YUV420;
  }
  else if(strcmp(stream_settings->payload_type, "PT_JPEG") == 0) {
    vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
    vid_format.fmt.pix.sizeimage = 0;
    vid_format.fmt.pix.field = V4L2_FIELD_NONE;
    vid_format.fmt.pix.bytesperline = 0;
    vid_format.fmt.pix.colorspace = V4L2_COLORSPACE_JPEG;
  }
  else {
    log_error("Unknown payload type: %s", stream_settings->payload_type);
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
  sleep(2);



  ret = IMP_Encoder_StartRecvPic(stream_settings->channel);
  if (ret < 0) {
    log_error("IMP_Encoder_StartRecvPic(%d) failed.", stream_settings->channel);
    return -1;
  }

  // Every set number of frames calculate out how many frames per second we are getting
  current_fps = 0;
  frames_written = 0;
  gettimeofday(&tval_before, NULL);

  while(!sigint_received) {

    // Audio Frames

    int ret = IMP_AI_PollingFrame(audio_device_id, audio_channel_id, 1000);
    if (ret < 0) {
      log_error("Error polling for audio frame");
      return -1;
    }

    ret = IMP_AI_GetFrame(audio_device_id, audio_channel_id, &audio_frame, BLOCK);
    if (ret < 0) {
      log_error("Error getting audio frame data");
      return -1;
    }

    num_samples = audio_frame.len / sizeof(short);



    memcpy(pcm_audio_data, (void *)audio_frame.virAddr, audio_frame.len);


    ret = IMP_AI_ReleaseFrame(audio_device_id, audio_channel_id, &audio_frame);
    if(ret != 0) {
      log_error("Error releasing audio frame");
      return -1;
    }

    log_debug("Obtained %d 16-bit samples from audio frame", num_samples);


    if (ret = snd_pcm_writei(pcm_handle, pcm_audio_data, num_samples) == -EPIPE) {
      // log_error("Buffer overrun when writing to ALSA loopback device");
      snd_pcm_prepare(pcm_handle);
    } else if (ret < 0) {
      log_error("ERROR. Can't write to PCM device. %s\n", snd_strerror(ret));
    }







    // Video Frames



    if (frames_written == 200) {
      gettimeofday(&tval_after, NULL);
      timersub(&tval_after, &tval_before, &tval_result);

      elapsed_seconds =  (long int)tval_result.tv_sec + ((long int)tval_result.tv_usec / 1000000);

      current_fps = 200 / elapsed_seconds;
      log_info("Current FPS: %.2f", current_fps);
      frames_written = 0;
      gettimeofday(&tval_before, NULL);
    }

    ret = IMP_Encoder_PollingStream(stream_settings->channel, 1000);
    if (ret < 0) {
      log_error("Timeout while polling for stream.");
      continue;
    }

    // Get H264 Stream on channel 0 and enable a blocking call
    ret = IMP_Encoder_GetStream(stream_settings->channel, &stream, 1);
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

    // hexdump("NAL Packet", stream.pack[i].virAddr, 10);

    /*
    ret = find_nal_unit(stream_chunk, total, &nal_start, &nal_end);

    if (ret > 0) {
      log_debug("Found a NAL unit: %d", ret);
      read_nal_unit(h, &stream_chunk[nal_start], nal_end - nal_start);
      debug_nal(h, h->nal);
    }
    */

    // Write out to the V4L2 device (for example /dev/video0)
    ret = write(v4l2_fd, (void *)stream_chunk, total);
    if (ret != total) {
      log_error("Stream write error: %s", ret);
      return -1;
    }


    free(stream_chunk);

    IMP_Encoder_ReleaseStream(stream_settings->channel, &stream);

    frames_written = frames_written + 1;
  }


  ret = IMP_Encoder_StopRecvPic(stream_settings->channel);
  if (ret < 0) {
    log_error("IMP_Encoder_StopRecvPic(%d) failed", stream_settings->channel);
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