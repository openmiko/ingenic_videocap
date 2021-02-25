#include "capture.h"
#include "fontmap.h"
#include "fontmapbig.h"

/*
	ISP - Ingenic Smart Platform
	IMP - Ingenic Multimedia Platform
*/

extern sig_atomic_t sigint_received;
extern snd_pcm_t *pcm_handle;
//extern pthread_mutex_t frame_generator_mutex;

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

	if (fgets(sensor_name_buffer, SENSOR_NAME_MAX_LENGTH, sensor_info_proc_file) == NULL) {
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
	if (ret < 0) {
		log_error("Failed to open ISP");
		return -1;
	}
	else {
		log_info("Opened the ISP module.");
	}

	ret = IMP_ISP_AddSensor(sensor_info);
	if (ret < 0) {
		log_error("Failed to register the %s sensor.", sensor_name);
		exit(-1);
	}
	else {
		log_info("Added the %s sensor.", sensor_name);
	}

	ret = IMP_ISP_EnableSensor();
	if (ret < 0) {
		log_error("Failed to EnableSensor");
		return -1;
	}

	ret = IMP_System_Init();
	if (ret < 0) {
		log_error("IMP_System_Init failed");
		return -1;
	}

	/* enable tuning, to debug graphics */
	ret = IMP_ISP_EnableTuning();
	if (ret < 0) {
		log_error("IMP_ISP_EnableTuning failed\n");
		return -1;
	}
	
	ret = IMP_ISP_Tuning_SetWDRAttr(IMPISP_TUNING_OPS_MODE_DISABLE);
	if (ret < 0) {
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
	audio_settings.numPerFrm = 960;
	audio_settings.chnCnt = 1;

	// ALSA
	snd_pcm_hw_params_t *pcm_hw_params;
	snd_pcm_uframes_t pcm_frames;
	int sample_rate, audio_channels;
	
	ret = IMP_AI_SetPubAttr(device_id, &audio_settings);
	if (ret < 0) {
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
	if (ret != 0) {
		log_error("Error enabling the audio device: %d", device_id);
		return -1;
	}

	// Set audio channel attributes of device
	
	// Audio frame buffer depth
	audio_channel_params.usrFrmDepth = 20;

	ret = IMP_AI_SetChnParam(device_id, audio_channel_id, &audio_channel_params);
	if (ret != 0) {
		log_error("Error setting the audio channel parameters for device %d", device_id);
		return -1;
	}

	// Step 4: enable AI channel.
	ret = IMP_AI_EnableChn(device_id, audio_channel_id);
	if (ret != 0) {
		log_error("Error enabling audio channel");
		return -1;
	}

	/* Step 5: Set audio channel volume. */
	ret = IMP_AI_SetVol(device_id, audio_channel_id, 70);
	if (ret != 0) {
		log_error("Error setting the audio channel volume");
		return -1;
	}	

	// ALSA loopback device setup
	// Found good sample code here: https://gist.github.com/ghedo/963382/98f730d61dad5b6fdf0c4edb7a257c5f9700d83b
	
	ret = snd_pcm_open(&pcm_handle, "hw:0,0", SND_PCM_STREAM_PLAYBACK, 0);
	if (ret != 0) {
		log_error("Error opening ALSA PCM loopback device.");
		return -1;
	}

	/* Allocate parameters object and fill it with default values*/
	snd_pcm_hw_params_alloca(&pcm_hw_params);
	snd_pcm_hw_params_any(pcm_handle, pcm_hw_params);

	audio_channels = 1;
	sample_rate = 48000;

	/* Set parameters */
	ret = snd_pcm_hw_params_set_access(pcm_handle, pcm_hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret < 0) {
		log_error("ERROR: Can't set interleaved mode. %s\n", snd_strerror(ret));
	}

	ret = snd_pcm_hw_params_set_format(pcm_handle, pcm_hw_params, SND_PCM_FORMAT_S16_LE);
	if (ret < 0) {
		log_error("ERROR: Can't set format. %s\n", snd_strerror(ret));
	}

	ret = snd_pcm_hw_params_set_channels(pcm_handle, pcm_hw_params, audio_channels);
	if (ret < 0) {
		log_error("ERROR: Can't set channels number. %s\n", snd_strerror(ret));
	}

	ret = snd_pcm_hw_params_set_rate_near(pcm_handle, pcm_hw_params, &sample_rate, 0);
	if (ret < 0) {
		log_error("ERROR: Can't set rate. %s\n", snd_strerror(ret));
	}

	/* Write parameters */
	ret = snd_pcm_hw_params(pcm_handle, pcm_hw_params);
	if (ret < 0) {
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
	else if (strcmp(encoder_setting->payload_type, "PT_JPEG") == 0) {
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
	
	// use IMP struct instead
	rc_attr->attrH264Vbr.staticTime = encoder_setting->h264vbr.statistics_interval;
	rc_attr->attrH264Vbr.maxBitRate = encoder_setting->h264vbr.max_bitrate;
	rc_attr->attrH264Vbr.changePos = encoder_setting->h264vbr.change_pos;

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

// OSD
void *osd_update_thread(void *p)
{
	OsdThreadData *osdThreadData = (OsdThreadData*) p;
	
	OsdGroup *osd_group = osdThreadData->osd_group;
	OsdItem *osd_item = osdThreadData->osd_item;
	
	uint32_t *pixel_matrix = (uint32_t *) malloc(osd_item->size_y * osd_item->size_x * 4);
	
	IMPOSDRgnAttrData r_attr_data;
	int ret, i, u, left, top;
	time_t curr_time;
	struct tm *curr_date;
	FILE *f;
	
	uint32_t *pixel;
	char text[128] = "";
	char osd_text_param[64] = "";
	bitmapinfo_t *fontmap;
	int font_height = 0;
	int space_char_size = 0;
	//int max_font_width = 0;
	bitmapinfo_t *font_char_data;
	void *font_char;
	int font_width, font_width_bytes;
	int offset_left = 0;
	int offset_top = 0;
	int font_char_offset = 0;
	int fix_smearing;
	
	// 'fps' counter (vars used to reset the counter back to 1 when the date 'seconds' digit changes)
	int osd_num_counter = 0;
	int setting_value = 0;
	char datetime_str[64] = "";
	char datetime_str2[64] = "";
	time_t curr_time_counter;
	struct tm *curr_date_counter;
	
	time(&curr_time_counter);
	curr_date_counter = localtime(&curr_time_counter);
	
	pthread_mutexattr_t mutex_attr;
	pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
	ret = pthread_mutex_init(&osd_item->mutex, &mutex_attr);
	if (ret != 0) {
		log_error("pthread_mutex_init error (osd_id %d)", osd_item->osd_id);
		return NULL;
	}

	// Flag at 1 allow to startup without having the same functions elsewhere
	osd_item->reload_flag = 1;
	
	while (!sigint_received) {
		pthread_mutex_lock(&osd_item->mutex);

		if (osd_item->reload_flag == 1) {
			IMPOSDRgnAttr rAttrTmp;
			memset(&rAttrTmp, 0, sizeof(IMPOSDRgnAttr));
			rAttrTmp.type = osd_area_type_to_int(osd_item->type);
			rAttrTmp.rect.p0.x = osd_item->pos_x;
			rAttrTmp.rect.p0.y = osd_item->pos_y;
			rAttrTmp.rect.p1.x = rAttrTmp.rect.p0.x + osd_item->size_x - 1;
			rAttrTmp.rect.p1.y = rAttrTmp.rect.p0.y + osd_item->size_y - 1;
			rAttrTmp.fmt = pixel_format_to_int(osd_item->format);
			rAttrTmp.data.picData.pData = NULL;
			
			if (rAttrTmp.type == OSD_REG_COVER) {
				rAttrTmp.data.coverData.color = get_osd_color_from_string(osd_item->primary_color);
			}
			else if (rAttrTmp.type == OSD_REG_RECT) {
				rAttrTmp.data.lineRectData.color = get_osd_color_from_string(osd_item->primary_color);
				rAttrTmp.data.lineRectData.linewidth = osd_item->line_width;
			}
			
			ret = IMP_OSD_SetRgnAttr(osd_item->rgn_hander, &rAttrTmp);
			if (ret < 0) {
				log_error("IMP_OSD_SetRgnAttr error! (group_id: %d, osd_id: %d)", osd_group->group_id, osd_item->osd_id);
			}
			
			// ---
			
			IMPOSDGrpRgnAttr grAttrTmp;
			if (IMP_OSD_GetGrpRgnAttr(osd_item->rgn_hander, osd_group->group_id, &grAttrTmp) < 0) {
				log_error("IMP_OSD_GetGrpRgnAttr error! (group_id: %d, osd_id: %d)", osd_group->group_id, osd_item->osd_id);
				return NULL;
			}
			
			memset(&grAttrTmp, 0, sizeof(IMPOSDGrpRgnAttr));
			grAttrTmp.show = osd_item->show;
			grAttrTmp.layer = osd_item->layer;
			grAttrTmp.gAlphaEn = osd_item->g_alpha_en;
			grAttrTmp.fgAlhpa = (int) strtol(osd_item->fg_alhpa, NULL, 0);
			grAttrTmp.bgAlhpa = (int) strtol(osd_item->bg_alhpa, NULL, 0);
			
			if (rAttrTmp.type == OSD_REG_RECT) {
				grAttrTmp.scalex = 1;
				grAttrTmp.scaley = 1;
			}
			
			if (IMP_OSD_SetGrpRgnAttr(osd_item->rgn_hander, osd_group->group_id, &grAttrTmp) < 0) {
				log_error("IMP_OSD_SetGrpRgnAttr error! (group_id: %d, osd_id: %d)", osd_group->group_id, osd_item->osd_id);
				return NULL;
			}
			
			// ---
			
			// show/hide osd... already done above
			/*ret = IMP_OSD_ShowRgn(osd_item->osd_id, osd_group->group_id, osd_item->show);
			if (ret != 0) {
				log_error("IMP_OSD_ShowRgn() error! (group_id: %d, osd_id: %d)", osd_group->group_id, osd_item->osd_id);
				return NULL;
			}*/
			
			if (strcmp(osd_item->font, "default") == 0) {
				fontmap = gBgramap;
				font_height = CHARHEIGHT;
			} else if (strcmp(osd_item->font, "default_big") == 0) {
				fontmap = gBgramapBig;
				font_height = CHARHEIGHT_BIG;
			} //else ...(add fonts here...)
				
			space_char_size = SPACELENGHT * 4;
			//max_font_width = fontmap['W' - STARTCHAR].width; // if needed
		}
		
		if (strcmp(osd_item->osd_type, "text") == 0) {
			if (strncmp(osd_item->text, "#DATETIME=", 10) == 0) {
				time(&curr_time);
				curr_date = localtime(&curr_time);
				
				for (i = 10; i < 64+10 && osd_item->text[i] != '#'; i++) {
					osd_text_param[i-10] = osd_item->text[i];
				}
				osd_text_param[i-10] = '\0';
				
				strftime(text, 32, osd_text_param, curr_date);
			}
			else if (strncmp(osd_item->text, "#COUNTER=", 8) == 0) {
				osd_num_counter++;
				
				for (i = 9; i < 64+9 && osd_item->text[i] != '#'; i++) {
					osd_text_param[i-9] = osd_item->text[i];
				}
				osd_text_param[i-9] = '\0';
				
				setting_value = atoi(osd_text_param);
				
				strftime(datetime_str, 32, "%S", curr_date_counter);
				time(&curr_time_counter);
				curr_date_counter = localtime(&curr_time_counter);
				strftime(datetime_str2, 32, "%S", curr_date_counter);
				
				if (osd_num_counter > setting_value || strcmp(datetime_str, datetime_str2) != 0) {
					osd_num_counter = 1;
				}
				
				snprintf(text, sizeof(text), "%d", osd_num_counter);
			}
			else if (strcmp(osd_item->text, "#HOSTNAME#") == 0) {				
				ret = gethostname(text, sizeof(text));
				if (ret != 0) {
					strcpy(text, "ERROR");
				}
				
				/*f = fopen("/etc/hostname", "r");
				if (f != NULL) {
					fscanf(f, "%s", text);
					fclose(f);
				}*/
			}
			else if (strcmp(text, osd_item->text) != 0) {
				strcpy(text, osd_item->text);
			}
			
			text[127] = '\0';
			
			offset_left = osd_item->left_right_padding;
			offset_top = osd_item->top_bottom_padding;
			font_char_offset = 0;
			
			if (strcmp(osd_item->secondary_color, "none") == 0) {
				// set all pixels to 0 (transparent) **
				memset(pixel_matrix, 0, osd_item->size_y * osd_item->size_x * 4);
			}
			else {
				// or set background color **
				for (i = 0; i < osd_item->size_y * osd_item->size_x; i++) {
					((uint32_t *) pixel_matrix)[i] = get_osd_color_from_string(osd_item->secondary_color);
				}
			}
			
			for (i = 0; text[i] != '\0' && i < sizeof(text); i++) {
				if (text[i] == ' ') {
					offset_left += space_char_size + osd_item->extra_space_char_size;
					continue;
				}
				else if (text[i] == '\n') {
					offset_top += font_height + osd_item->line_spacing;
					offset_left = osd_item->left_right_padding;
					continue;
				}
				else if (text[i] == '\t') {
					offset_left += (space_char_size + osd_item->extra_space_char_size) * 4;
					continue;
				}
				// check if character exists in font
				else if (text[i] < STARTCHAR || text[i] > ENDCHAR)
					text[i] = '?';
				
				// get font character data and details
 				font_char_data = &fontmap[text[i] - STARTCHAR];
				font_char = (void *) font_char_data->pdata;
				font_width = font_char_data->width;
				font_width_bytes = font_char_data->widthInByte * 8;
				
				// line break if new character doesn't fit on the right end
				if (offset_left + font_width + osd_item->left_right_padding > osd_item->size_x) {
					offset_top += font_height + osd_item->line_spacing;
					offset_left = osd_item->left_right_padding;
				}
				
				// calculate offset to center character
				if (osd_item->fixed_font_width != 0) {
					font_char_offset = (int) ((osd_item->fixed_font_width - font_width) / 2);
				}
				
				// go through font character bytes horizontaly
				for (left = 0; left < font_width; left++) {
					// useless..
					//if (left + offsetLeft + osd_item->leftRightPadding >= osd_item->size_x) break;
					
					// go through font character bytes verticaly
					for (top = 0; top < font_height; top++) {
						// cut character verticaly if it doesn't fit
						if (top + offset_top + osd_item->top_bottom_padding >= osd_item->size_y) break;
						
						// if osd_item->size_x is an odd number some smearing occurs..
						// couldn't find the problem source, so....
						fix_smearing = 0;
						if (osd_item->size_x % 2 != 0 && (top + offset_top) % 2 != 0)
							fix_smearing = 1;
						
						// up/down: (top + offsetTop) * osd_item->size_x
						// left/right: left + offsetLeft + fontCharOffset - fixSmearing
						pixel = &pixel_matrix[(top + offset_top) * osd_item->size_x + left + offset_left + font_char_offset - fix_smearing];
						
						// if pixel belongs to the font character
						if (((uint32_t *) font_char)[(top * font_width_bytes) + left]) {
							// set pixel color
							*pixel = get_osd_color_from_string(osd_item->primary_color);
						}
						//else {
							// set pixel transparent
							// sometimes it doesn't clear old text, the memset/forloop above clears everything **
							//*pixel = get_osd_color_from_string(osd_item->secondary_color);
						//}
					}
				}
				
				if (osd_item->fixed_font_width == 0)
					offset_left += font_width + osd_item->letter_spacing;
				else
					offset_left += osd_item->fixed_font_width + osd_item->letter_spacing;
			}
			
			r_attr_data.picData.pData = pixel_matrix;
			IMP_OSD_UpdateRgnAttrData(osd_item->osd_id, &r_attr_data);
		}
		else if (strcmp(osd_item->osd_type, "shape") == 0 || strcmp(osd_item->osd_type, "image") == 0) {
			// convert png's to bgra
			// eg: https://www.onlineconvert.com/
			// maybe change everything to rgba or argb in the future...
			
			if (osd_item->reload_flag == 1) {
				FILE *f = fopen(osd_item->image, "rb");
				if (f != NULL) {
					uint8_t *pixel_matrix_8bit = malloc(osd_item->size_x * osd_item->size_y * 4);
					int image_loc = 0;
					long f_lenght;
					uint32_t primary_color, secondary_color;
					primary_color = get_osd_color_from_string(osd_item->primary_color);
					secondary_color = get_osd_color_from_string(osd_item->secondary_color);
					
					fseek(f, 0, SEEK_END);
					f_lenght = ftell(f);
					rewind(f);
					
					for (i = 0; i < f_lenght && i < osd_item->size_x * osd_item->size_y * 4; i++) {
						fread(&pixel_matrix_8bit[image_loc++], 1, 1, f);
					}

					fclose(f);
					
					if (strcmp(osd_item->osd_type, "shape") == 0) {
						i = 0;
						do {
							/*	change shape color based on transparency
							 	afterthought: ...this 'if' is useless because everything transparent won't show up, dah
								only needed to change the background color
								but i'm not using it because if the shape has opacity arround it
								it won't have background there and it i'll create an halo effect around it
							*/
							//if (pixel_matrix_8bit[i+3] != 0x00) {
								pixel_matrix_8bit[i] = (uint8_t)primary_color;
								pixel_matrix_8bit[i+1] = (uint8_t)(primary_color >> 8);
								pixel_matrix_8bit[i+2] = (uint8_t)(primary_color >> 16);
							//} else {
								//pixel_matrix_8bit[i] = (uint8_t)secondary_color;
								//pixel_matrix_8bit[i+1] = (uint8_t)(secondary_color >> 8);
								//pixel_matrix_8bit[i+2] = (uint8_t)(secondary_color >> 16);
								//pixel_matrix_8bit[i+3] = 0xff;
							//}
							
							i += 4;
						} while (i < osd_item->size_x * osd_item->size_y * 4);
					}
					
					r_attr_data.picData.pData = pixel_matrix_8bit;
					IMP_OSD_UpdateRgnAttrData(osd_item->osd_id, &r_attr_data);
				}
				else {
					log_error("Could not open OSD image file '%s'! (group_id: %d, osd_id: %d)", osd_item->image, osd_group->group_id, osd_item->osd_id);
				}
			}
		}
		
		if (osd_item->reload_flag == 1) osd_item->reload_flag = 0;
		
		pthread_mutex_unlock(&osd_item->mutex);
		
		if (osd_item->update_interval > 1)
			usleep(1000 * osd_item->update_interval);
		else
			sleep(1);
	}
	
	free(pixel_matrix);
	
	pthread_mutex_destroy(&osd_item->mutex);

	return NULL;
}

void start_osd_update_threads(CameraConfig *camera_config)
{
	int ret;
	OsdThreadData osd_thread_data[MAX_OSDGROUPS][MAX_OSDITEMS];

	for (int igrp = 0; igrp < camera_config->num_osd_groups; igrp++) {
		OsdGroup *osd_group = &camera_config->osd_groups[igrp];
		
		for (int iosd = 0; iosd < osd_group->osd_list_size; iosd++) {
			OsdItem *osd_item = &osd_group->osd_list[iosd];
			
			// ...change this to pointers?
			osd_thread_data[igrp][iosd].osd_group = osd_group;
			osd_thread_data[igrp][iosd].osd_item = osd_item;

			ret = pthread_create(&osd_item->thread_id, NULL, osd_update_thread, (void*) &osd_thread_data[igrp][iosd]);
			if (ret) {
				log_error("OSD: thread create error!");
				return;
			}
			
			log_info("OSD thread %d started.", osd_item->thread_id);
		}
	}
	
	sleep(2);
}

void print_channel_attributes(IMPFSChnAttr *attr)
{
	char buffer[1024];

	snprintf(buffer, sizeof(buffer),
		"IMPFSChnAttr: \n"
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

	snprintf(buffer, sizeof(buffer),
		"IMPEncoderCHNAttr: \n"
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

	snprintf(buffer, sizeof(buffer),
		"Stream settings: \n"
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
	unsigned long delay_in_microseconds = 0;

	encoder_setting->reload_flag = 0;
	
	pthread_mutexattr_t mutex_attr;
	pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
	ret = pthread_mutex_init(&encoder_setting->mutex, &mutex_attr);
	if (ret != 0) {
		log_error("pthread_mutex_init error (encoder channel %d)", encoder_setting->channel);
		return -1;
	}

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

	// h264_stream_t *h = h264_new();
	// int nal_start, nal_end;
	// uint8_t* buf;
	// int len;

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
	
	delay_in_microseconds = (unsigned long) (1000 * 1000 * encoder_setting->frame_rate_denominator) / encoder_setting->frame_rate_numerator;

	// Every set number of frames calculate out how many frames per second we are getting
	current_fps = 0;
	frames_written = 0;
	gettimeofday(&tval_before, NULL);

	while(!sigint_received) {
		pthread_mutex_lock(&encoder_setting->mutex);
	
		if (encoder_setting->reload_flag == 1) {
			reload_encoder_config(encoder_setting);
			
			delay_in_microseconds = (unsigned long) (1000 * 1000 * encoder_setting->frame_rate_denominator) / encoder_setting->frame_rate_numerator;
			
			encoder_setting->reload_flag = 0;
		}

		// Audio Frames

		// int ret = IMP_AI_PollingFrame(audio_device_id, audio_channel_id, 1000);
		// if (ret < 0) {
		//   log_error("Error polling for audio frame");
		//   return -1;
		// }

		// ret = IMP_AI_GetFrame(audio_device_id, audio_channel_id, &audio_frame, BLOCK);
		// if (ret < 0) {
		//   log_error("Error getting audio frame data");
		//   return -1;
		// }

		// num_samples = audio_frame.len / sizeof(short);


		// memcpy(pcm_audio_data, (void *)audio_frame.virAddr, audio_frame.len);


		// ret = IMP_AI_ReleaseFrame(audio_device_id, audio_channel_id, &audio_frame);
		// if(ret != 0) {
		//   log_error("Error releasing audio frame");
		//   return -1;
		// }

		// if (ret = snd_pcm_writei(pcm_handle, pcm_audio_data, num_samples) == -EPIPE) {
		//   // log_error("Buffer overrun when writing to ALSA loopback device");
		//   snd_pcm_prepare(pcm_handle);
		// } else if (ret < 0) {
		//   log_error("ERROR. Can't write to PCM device. %s\n", snd_strerror(ret));
		// }

		// Video Frames

		if (frames_written == 200) {
			gettimeofday(&tval_after, NULL);
			timersub(&tval_after, &tval_before, &tval_result);

			elapsed_seconds = (long int)tval_result.tv_sec + ((long int)tval_result.tv_usec / 1000000);

			current_fps = 200 / elapsed_seconds;
			log_info("Current FPS: %.2f / Channel %d", current_fps, encoder_setting->channel);
			//log_info("Obtained %d 16-bit samples from this specific audio frame", num_samples);

			// IMPEncoderCHNStat encoder_status;

			// IMP_Encoder_Query(encoder_setting->channel, &encoder_status);

			// log_info("Registered: %u", encoder_status.registered);
			// log_info("Work done (0 is running, 1 is not running): %u", encoder_status.work_done);
			// log_info("Number of images to be encoded: %u", encoder_status.leftPics);
			// log_info("Number of bytes remaining in the stream buffer: %u", encoder_status.leftStreamBytes);

			frames_written = 0;
			gettimeofday(&tval_before, NULL);
		}

		// obs: This function times the frames
		ret = IMP_Encoder_PollingStream(encoder_setting->channel, 1050);
		if (ret < 0) {
			log_error("Timeout while polling for stream on channel %d.", encoder_setting->channel);
			pthread_mutex_destroy(&encoder_setting->mutex);
			continue;
		}
		
		// PT_JPEG doesn't time its frames, so we have to sleep for a bit
		if(strcmp(encoder_setting->payload_type, "PT_JPEG") == 0)
			usleep(delay_in_microseconds);

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
		
		pthread_mutex_unlock(&encoder_setting->mutex);
	}

	pthread_mutex_destroy(&encoder_setting->mutex);

	ret = IMP_Encoder_StopRecvPic(encoder_setting->channel);
	if (ret < 0) {
		log_error("IMP_Encoder_StopRecvPic(%d) failed", encoder_setting->channel);
		return -1;
	}
}

void *isp_settings_thread(void *isp_settings_)
{
	ISPSettings *isp_settings = (ISPSettings*) isp_settings_;
	
	int ret;
	ret = sem_init(&isp_settings->semaphore, 1, 0);
	if (ret < 0) {
		log_error("sem_init failed on isp settings");
		return NULL;
	}

	while(!sigint_received) {
		if (isp_settings->night_mode_flag == 1) {
			reload_night_vision(isp_settings);
			isp_settings->night_mode_flag = 0;
		}
		
		if (isp_settings->flip_image_flag == 1) {
			reload_flip_image(isp_settings);
			isp_settings->flip_image_flag = 0;
		}
		
		usleep(1000 * 100); // can be removed, it's just a precaution

		sem_wait(&isp_settings->semaphore);
	}

	sem_destroy(&isp_settings->semaphore);
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

void reload_night_vision(ISPSettings *isp_settings)
{
	int ret;
	IMPISPRunningMode isprunningmode;
	IMPISPSceneMode sceneMode;
	IMPISPColorfxMode colormode;
	
	if (isp_settings->night_mode == 1) {
		isprunningmode = IMPISP_RUNNING_MODE_NIGHT;
		sceneMode = IMPISP_SCENE_MODE_NIGHT;
		colormode = IMPISP_COLORFX_MODE_BW;
	}
	else {
		isprunningmode = IMPISP_RUNNING_MODE_DAY;
		sceneMode = IMPISP_SCENE_MODE_AUTO;
		colormode = IMPISP_COLORFX_MODE_AUTO;
	}
	
	ret = IMP_ISP_Tuning_SetISPRunningMode(isprunningmode);
	if (ret) {
		log_error("IMP_ISP_Tuning_SetISPRunningMode failed");
		return;
	}
	
	ret = IMP_ISP_Tuning_SetSceneMode(sceneMode);
	if (ret) {
		log_error("IMP_ISP_Tuning_SetSceneMode failed");
		return;
	}
	
	ret = IMP_ISP_Tuning_SetColorfxMode(colormode);
	if (ret) {
		log_error("IMP_ISP_Tuning_SetColorfxMode failed");
		return;
	}
	
	log_info("Night mode updated. (%d)", isp_settings->night_mode);
}

void reload_flip_image(ISPSettings *isp_settings) {
	int ret;

	IMPISPTuningOpsMode tuning_ops_mode;
	tuning_ops_mode = IMPISP_TUNING_OPS_MODE_DISABLE;

	if (isp_settings->flip_image == 1)
		tuning_ops_mode = IMPISP_TUNING_OPS_MODE_ENABLE;

	ret = IMP_ISP_Tuning_SetISPVflip(tuning_ops_mode);
	if (ret) {
		log_error("IMP_ISP_Tuning_SetISPVflip failed");
	}

	ret = IMP_ISP_Tuning_SetISPHflip(tuning_ops_mode);
	if (ret) {
		log_error("IMP_ISP_Tuning_SetISPHflip failed");
	}

	log_info("Flip image mode updated. (%d)", isp_settings->flip_image);
}

void reload_encoder_config(EncoderSetting *encoder_setting) {
	IMPEncoderRcAttr rc_attr;
	
	IMP_Encoder_GetChnRcAttr(encoder_setting->channel, &rc_attr);
	
	rc_attr.attrH264Vbr.outFrmRate.frmRateNum = encoder_setting->frame_rate_numerator;
	rc_attr.attrH264Vbr.outFrmRate.frmRateDen = encoder_setting->frame_rate_denominator;
	rc_attr.attrH264Vbr.maxGop = encoder_setting->max_group_of_pictures;
	rc_attr.attrH264Vbr.maxQp = encoder_setting->max_qp;
	rc_attr.attrH264Vbr.minQp = encoder_setting->min_qp;
	
	rc_attr.attrH264Vbr.staticTime = encoder_setting->h264vbr.statistics_interval;
	rc_attr.attrH264Vbr.maxBitRate = encoder_setting->h264vbr.max_bitrate;
	rc_attr.attrH264Vbr.changePos = encoder_setting->h264vbr.change_pos;
	
	rc_attr.attrH264Vbr.FrmQPStep = encoder_setting->frame_qp_step;
	rc_attr.attrH264Vbr.GOPQPStep = encoder_setting->gop_qp_step;
	
	//rc_attr.attrH264FrmUsed.enable = 1;
	
	IMP_Encoder_SetChnRcAttr(encoder_setting->channel, &rc_attr);
	
	log_info("Encoder attributes for channel %d changed.", encoder_setting->channel);
}

char* itoa(int val, int base) {
	static char buf[32] = {0};
	int i = 30;

	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];

	return &buf[i+1];
}
