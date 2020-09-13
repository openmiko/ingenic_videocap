#include "capture.h"

/*

ISP - Ingenic Smart Platform
IMP - Ingenic Multimedia Platform

*/

extern sig_atomic_t sigint_received;

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

int setup_framesource(StreamSettings* stream_settings, int channel)
{
  int ret;

  IMPFSChnAttr fs_chn_attr = {
    .pixFmt = PIX_FMT_NV12,
    .outFrmRateNum = stream_settings->frame_rate_numerator,
    .outFrmRateDen = stream_settings->frame_rate_denominator,
    .nrVBs = 3,
    .type = FS_PHY_CHANNEL,

    .crop.enable = CROP_EN,
    .crop.top = 0,
    .crop.left = 0,
    .crop.width = SENSOR_WIDTH,
    .crop.height = SENSOR_HEIGHT,

    .scaler.enable = 0,
    .scaler.outwidth = SENSOR_WIDTH_SECOND,
    .scaler.outheight = SENSOR_HEIGHT_SECOND,
    .picWidth = SENSOR_WIDTH,
    .picHeight = SENSOR_HEIGHT
  };

  log_info("Setting up frame source.");

  ret = IMP_FrameSource_CreateChn(channel, &fs_chn_attr);
  if(ret < 0){
    log_error("IMP_FrameSource_CreateChn error.");
    return -1;
  }

  ret = IMP_FrameSource_SetChnAttr(channel, &fs_chn_attr);
  if (ret < 0) {
    log_error("IMP_FrameSource_SetChnAttr error.");
    return -1;
  }

  ret = IMP_Encoder_CreateGroup(channel);
  if (ret < 0) {
    log_error("IMP_Encoder_CreateGroup error.");
    return -1;
  }

  log_info("Frame source setup complete.");

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


  log_info("bufSize %d", enc_attr->bufSize);
  log_info("profile %d", enc_attr->profile);
  log_info("picWidth %d", enc_attr->picWidth);
  log_info("picHeight %d", enc_attr->picHeight);
  log_info("frmRateNum %d", rc_attr->attrH264Vbr.outFrmRate.frmRateNum);
  log_info("frmRateDen %d", rc_attr->attrH264Vbr.outFrmRate.frmRateDen);
  log_info("maxGop %d", rc_attr->attrH264Vbr.maxGop);
  log_info("maxQp %d", rc_attr->attrH264Vbr.maxQp);
  log_info("minQp %d", rc_attr->attrH264Vbr.minQp);
  log_info("staticTime %d", rc_attr->attrH264Vbr.staticTime);
  log_info("maxBitRate %d", rc_attr->attrH264Vbr.maxBitRate);
  log_info("changePos %d", rc_attr->attrH264Vbr.changePos);
  log_info("FrmQPStep %d", rc_attr->attrH264Vbr.FrmQPStep);
  log_info("GOPQPStep %d", rc_attr->attrH264Vbr.GOPQPStep);


  ret = IMP_Encoder_CreateChn(0, &channel_attr);
  if (ret < 0) {
    log_error("IMP_Encoder_CreateChn error.", i);
    return -1;
  }

  ret = IMP_Encoder_RegisterChn(0, 0);
  if (ret < 0) {
    log_error("IMP_Encoder_RegisterChn error.");
    return -1;
  }

  return 0;  

}

int output_v4l2_frames(char *v4l2_device_path, int video_width, int video_height)
{
  int ret;
  int stream_packets;
  int i;
  int total;

  int frames_written = 0;
  float current_fps = 0;
  float elapsed_seconds = 0;
  struct timeval tval_before, tval_after, tval_result;



  IMPCell framesource_chn = { DEV_ID_FS, 0, 0};
  IMPCell imp_encoder = { DEV_ID_ENC, 0, 0};

  struct v4l2_capability vid_caps;
  struct v4l2_format vid_format;

  IMPEncoderStream stream;

  uint8_t *stream_chunk;
  uint8_t *temp_chunk;


  // h264 NAL unit stuff
  h264_stream_t *h = h264_new();
  int nal_start, nal_end;
  uint8_t* buf;
  int len;


  ret = IMP_System_Bind(&framesource_chn, &imp_encoder);
  if (ret < 0) {
    log_error("Erroring binding frame source channel to encoder.");
    return -1;
  }

  ret = IMP_FrameSource_EnableChn(0);
  if (ret < 0) {
    log_error("IMP_FrameSource_EnableChn(0) error: %d", ret);
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
  vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
  vid_format.fmt.pix.sizeimage = 0;
  vid_format.fmt.pix.field = V4L2_FIELD_NONE;
  vid_format.fmt.pix.bytesperline = 0;
  vid_format.fmt.pix.colorspace = V4L2_PIX_FMT_YUV420;

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



  ret = IMP_Encoder_StartRecvPic(0);
  if (ret < 0) {
    log_error("IMP_Encoder_StartRecvPic(0) failed.");
    return -1;
  }

  // Every set number of frames calculate out how many frames per second we are getting
  current_fps = 0;
  frames_written = 0;
  gettimeofday(&tval_before, NULL);

  while(!sigint_received) {

    //usleep(40 * 1000);

    if (frames_written == 200) {
      gettimeofday(&tval_after, NULL);
      timersub(&tval_after, &tval_before, &tval_result);

      elapsed_seconds =  (long int)tval_result.tv_sec + ((long int)tval_result.tv_usec / 1000000);

      current_fps = 200 / elapsed_seconds;
      log_info("Current FPS: %.2f", current_fps);
      frames_written = 0;
      gettimeofday(&tval_before, NULL);
    }


    ret = IMP_Encoder_PollingStream(0, 1000);
    if (ret < 0) {
      log_error("Timeout while polling for stream.");
      continue;
    }

    // Get H264 Stream on channel 0 and enable a blocking call
    ret = IMP_Encoder_GetStream(0, &stream, 1);
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

    IMP_Encoder_ReleaseStream(0, &stream);

    frames_written = frames_written + 1;
  }


  ret = IMP_Encoder_StopRecvPic(0);
  if (ret < 0) {
    log_error("IMP_Encoder_StopRecvPic() failed");
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