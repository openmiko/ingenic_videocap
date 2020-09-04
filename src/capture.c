#include "capture.h"

/*

ISP - Ingenic Smart Platform
IMP - Ingenic Multimedia Platform

*/


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

int setup_framesource()
{
  int ret;

  IMPFSChnAttr fs_chn_attr = {
    .pixFmt = PIX_FMT_NV12,
    .outFrmRateNum = SENSOR_FRAME_RATE_NUM,
    .outFrmRateDen = SENSOR_FRAME_RATE_DEN,
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
  IMPCell framesource_chn = { DEV_ID_FS, 0, 0};
  IMPCell imp_encoder = { DEV_ID_ENC, 0, 0};

  log_info("Setting up frame source.");

  ret = IMP_FrameSource_CreateChn(0, &fs_chn_attr);
  if(ret < 0){
    log_error("IMP_FrameSource_CreateChn error.");
    return -1;
  }

  ret = IMP_FrameSource_SetChnAttr(0, &fs_chn_attr);
  if (ret < 0) {
    log_error("IMP_FrameSource_SetChnAttr error.");
    return -1;
  }

  ret = IMP_Encoder_CreateGroup(0);
  if (ret < 0) {
    log_error("IMP_Encoder_CreateGroup error.");
    return -1;
  }

  log_info("Frame source setup complete.");

  return 0;
}

int setup_encoding_engine(int video_width, int video_height, int fps)
{
  int i, ret;
  IMPEncoderAttr *enc_attr;
  IMPEncoderRcAttr *rc_attr;
  IMPFSChnAttr *imp_chn_attr_tmp;
  IMPEncoderCHNAttr channel_attr;

  // imp_chn_attr_tmp = &chn[i].fs_chn_attr;


  memset(&channel_attr, 0, sizeof(IMPEncoderCHNAttr));
  enc_attr = &channel_attr.encAttr;
  enc_attr->enType = PT_H264;
  enc_attr->bufSize = 0;
  enc_attr->profile = 0;
  enc_attr->picWidth = video_width;
  enc_attr->picHeight = video_height;
  rc_attr = &channel_attr.rcAttr;


  rc_attr->rcMode = ENC_RC_MODE_H264VBR;
  rc_attr->attrH264Vbr.outFrmRate.frmRateNum = fps;
  rc_attr->attrH264Vbr.outFrmRate.frmRateDen = 1;
  rc_attr->attrH264Vbr.maxGop = 10;
  rc_attr->attrH264Vbr.maxQp = 45; // 38 before
  rc_attr->attrH264Vbr.minQp = 25; // 15 before
  rc_attr->attrH264Vbr.staticTime = 1;
  rc_attr->attrH264Vbr.maxBitRate = 500;
  rc_attr->attrH264Vbr.changePos = 50;
  rc_attr->attrH264Vbr.FrmQPStep = 3;
  rc_attr->attrH264Vbr.GOPQPStep = 15;
  rc_attr->attrH264FrmUsed.enable = 1;


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

