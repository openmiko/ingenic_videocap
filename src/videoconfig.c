#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#include "streamsettings.h"

// dafang trick
#define SETGETCONFIGINT(INT) if (opt_set) { INT = atoi(opt_value); } printf("%d\n", INT);
#define SETGETCONFIGSTRING(STR) if (opt_set) { strcpy(STR, opt_value); } printf("%s\n", STR);
#define SETGETCONFIGBOOL(INT) if (opt_set) { INT = str_to_bool(opt_value); } printf("%d\n", INT);

int str_to_bool(char *str);
void usage(char *command);

/*
	-h -> Help
	-o -> Output
	-s -> isp Settings
	-e -> Encoder
	-g -> osd Group
	-i -> osd Item
	-k -> Key
	-v -> Value
*/

#define USAGE "usage: %s [-h] [-o] [-s]/[-e encoder]/([-g osd_group] [-i osd_item]) [-k key] [-v value]\n"

extern char *optarg;
extern int opterr, optind;

int main(int argc, char *argv[]) {
	int i;
	int opt;
	int opt_set = 1;
	int opt_isp_settings = -1;
	int opt_encoder = -1;
	int opt_osd_group = -1;
	int opt_osd_item = -1;
	char opt_key[64] = "";
	char opt_value[64] = "";

	while ((opt = getopt(argc, argv, "hse:g:i:k:v:o")) != EOF) {
		switch(opt) {
			case 'h':
				usage(argv[0]);
				return EXIT_FAILURE;
				break;
			case 's':
				opt_isp_settings = 1;
				break;
			case 'e':
				opt_encoder = atoi(optarg);
				break;
			case 'g':
				opt_osd_group = atoi(optarg);
				break;
			case 'i':
				opt_osd_item = atoi(optarg);
				break;
			case 'k':
				strcpy(opt_key, optarg);
				break;
			case 'v':
				strcpy(opt_value, optarg);
				break;
			case 'o':
				opt_set = 0;
				break;
			case '?':
				if (optopt == 'e' || optopt == 'g' || optopt == 'i' || optopt == 'k' || optopt == 'v')
					printf("Option -%c requires an argument!\n", optopt); //, strerror(errno));
				else if (isprint(optopt))
					printf("Unknown option '-%c'.\n", optopt); //, strerror(errno));
				else
					printf("Unknown option character '\\x%x'.\n", optopt); //, strerror(errno));
				
				return EXIT_FAILURE;
			default:
				printf("Option '%c' not supported!\n", opt);
				return EXIT_FAILURE;
		}
	}
	
	/*
	for (i = optind; i < argc; i++)
		printf ("Non-option argument %s\n", argv[i]);
	*/
	
	// check if videocapture is running
	char vbuf[2] = "";
	FILE *cmd_pipe = popen("pidof videocapture", "r");
	fgets(vbuf, sizeof(vbuf), cmd_pipe);
	pclose(cmd_pipe);
		
	if (vbuf[0] == '\0') {
		printf("videocapture process not running!\n");
		return -1;
	}
	
	// Basic verifications
	if (opt_isp_settings == -1 && opt_encoder == -1 && opt_osd_group == -1 && opt_osd_item == -1) {
		printf(USAGE, argv[0]);
		return EXIT_FAILURE;
	}
	
	if (opt_osd_group >= 0 && opt_osd_item < 0) {
		printf("OSD group id option [-g] set but missing OSD item id [-i]!\n");
		return EXIT_FAILURE;
	}
	else if (opt_osd_item >= 0 && opt_osd_group < 0) {
		printf("OSD item id option [-i] set but missing OSD group id [-g]!\n");
		return EXIT_FAILURE;
	}
	
	if (opt_key[0] == '\0') {
		printf("Key [-k] option needs to be set!\n");
		return EXIT_FAILURE;
	}
	else if (opt_value[0] == '\0' && opt_set == 1) {
		printf("Value [-v] option needs to be set!\n");
		return EXIT_FAILURE;
	}
	
	// Shared Memory (configs)
	int shmid;
	key_t key;
	CameraConfig *camera_config;
	int shmflag = 0666;
	
	// Generate key for shared memory
	if ((key = ftok(FTOKDIR, FTOKPROJID)) == (key_t)-1) {
		printf("Error: shared memory (ftok): %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	// Create or open the shared memory segment
	if ((shmid = shmget(key, sizeof(CameraConfig), shmflag)) < 0) {
		printf("Error: shared memory (shmget): %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	// Attach the segment to our data space
	if ((camera_config = shmat(shmid, NULL, 0)) == (CameraConfig*)-1) {
		printf("Error: shared memory (shmat): %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	if (opt_isp_settings != -1) {
		// ISP settings
		ISPSettings *isp_settings = &camera_config->isp_settings;
		
		if (strcmp(opt_key, "night_mode") == 0) {
			SETGETCONFIGBOOL(isp_settings->night_mode);
			
			if (opt_set == 1)
				isp_settings->night_mode_flag = 1;
		}
		else if (strcmp(opt_key, "flip_image") == 0) {
			SETGETCONFIGBOOL(isp_settings->flip_image);
			
			if (opt_set == 1)
				isp_settings->flip_image_flag = 1;
		}
		else {
			printf("'%s' invalid as an ISP setting!\n", opt_key);
			goto err;
		}
		
		if (opt_set == 1)
			sem_post(&isp_settings->semaphore);
	}
	else if (opt_encoder != -1) {
		// Encoders settings
		
		// Search encoder by channel id
		EncoderSetting *encoder = NULL;
		for (i = 0; i < camera_config->num_encoders; i++) {
			if (camera_config->encoders[i].channel == opt_encoder) {
				encoder = &camera_config->encoders[i];
				break;
			}
		}
		if (encoder == NULL) {
			printf("Encoder with channel '%d' not found!", opt_encoder);
			goto err;
		}
		
		pthread_mutex_lock(&encoder->mutex);
		
		if (strcmp(opt_key, "frame_rate_numerator") == 0) {
			SETGETCONFIGINT(encoder->frame_rate_numerator);
		}
		else if (strcmp(opt_key, "frame_rate_denominator") == 0) {
			SETGETCONFIGINT(encoder->frame_rate_denominator);
		}
		else if (strcmp(opt_key, "max_group_of_pictures") == 0) {
			SETGETCONFIGINT(encoder->max_group_of_pictures);
		}
		else if (strcmp(opt_key, "max_qp") == 0) {
			SETGETCONFIGINT(encoder->max_qp);
		}
		else if (strcmp(opt_key, "min_qp") == 0) {
			SETGETCONFIGINT(encoder->min_qp);
		}
		else if (strcmp(opt_key, "h264vbr_statistics_interval") == 0) {
			SETGETCONFIGINT(encoder->h264vbr.statistics_interval);
		}
		else if (strcmp(opt_key, "h264vbr_max_bitrate") == 0) {
			SETGETCONFIGINT(encoder->h264vbr.max_bitrate);
		}
		else if (strcmp(opt_key, "h264vbr_change_pos") == 0) {
			SETGETCONFIGINT(encoder->h264vbr.change_pos);
		}
		else if (strcmp(opt_key, "frame_qp_step") == 0) {
			SETGETCONFIGINT(encoder->frame_qp_step);
		}
		else if (strcmp(opt_key, "gop_qp_step") == 0) {
			SETGETCONFIGINT(encoder->gop_qp_step);
		}
		else {
			printf("'%s' invalid as an Encoder setting!\n", opt_key);
		}
		
		encoder->reload_flag = 1;
		pthread_mutex_unlock(&encoder->mutex);
	}	
	else if (opt_osd_group != -1 && opt_osd_item != -1) {
		// OSD settings
		
		// Search osd group by channel id
		OsdGroup *osd_group = NULL;
		for (i = 0; i < camera_config->num_osd_groups; i++) {
			if (camera_config->osd_groups[i].group_id == opt_osd_group) {
				osd_group = &camera_config->osd_groups[i];
				break;
			}
		}
		if (osd_group == NULL) {
			printf("OSD group with group_id '%d' not found!", opt_osd_group);
			goto err;
		}
		
		
		// Search osd item by channel id
		OsdItem *osd_item = NULL;
		for (i = 0; i < osd_group->osd_list_size; i++) {
			if (osd_group->osd_list[i].osd_id == opt_osd_item) {
				osd_item = &osd_group->osd_list[i];
				break;
			}
		}
		if (osd_item == NULL) {
			printf("OSD item with osd_id '%d' not found!", opt_osd_item);
			goto err;
		}
		
		pthread_mutex_lock(&osd_item->mutex);
		
		if (strcmp(opt_key, "show") == 0) {
			SETGETCONFIGBOOL(osd_item->show);
		}
		else if (strcmp(opt_key, "pos_x") == 0) {
			SETGETCONFIGINT(osd_item->pos_x);
		}
		else if (strcmp(opt_key, "pos_y") == 0) {
			SETGETCONFIGINT(osd_item->pos_y);
		}
		else if (strcmp(opt_key, "size_x") == 0) {
			SETGETCONFIGINT(osd_item->size_x);
		}
		else if (strcmp(opt_key, "size_y") == 0) {
			SETGETCONFIGINT(osd_item->size_y);
		}
		else if (strcmp(opt_key, "layer") == 0) {
			SETGETCONFIGINT(osd_item->layer);
		}
		else if (strcmp(opt_key, "g_alpha_en") == 0) {
			SETGETCONFIGBOOL(osd_item->g_alpha_en);
		}
		else if (strcmp(opt_key, "fg_alhpa") == 0) {
			SETGETCONFIGSTRING(osd_item->fg_alhpa);
		}
		else if (strcmp(opt_key, "bg_alhpa") == 0) {
			SETGETCONFIGSTRING(osd_item->bg_alhpa);
		}
		else if (strcmp(opt_key, "primary_color") == 0) {
			SETGETCONFIGSTRING(osd_item->primary_color);
		}
		else if (strcmp(opt_key, "secondary_color") == 0) {
			SETGETCONFIGSTRING(osd_item->secondary_color);
		}
		else if (strcmp(opt_key, "line_width") == 0) {
			SETGETCONFIGINT(osd_item->line_width);
		}
		else if (strcmp(opt_key, "image") == 0) {
			SETGETCONFIGSTRING(osd_item->image);
		}
		else if (strcmp(opt_key, "text") == 0) {
			SETGETCONFIGSTRING(osd_item->text);
		}
		else if (strcmp(opt_key, "font") == 0) {
			SETGETCONFIGSTRING(osd_item->font);
		}
		else if (strcmp(opt_key, "extra_space_char_size") == 0) {
			SETGETCONFIGINT(osd_item->extra_space_char_size);
		}
		else if (strcmp(opt_key, "line_spacing") == 0) {
			SETGETCONFIGINT(osd_item->line_spacing);
		}
		else if (strcmp(opt_key, "letter_spacing") == 0) {
			SETGETCONFIGINT(osd_item->letter_spacing);
		}
		else if (strcmp(opt_key, "left_right_padding") == 0) {
			SETGETCONFIGINT(osd_item->left_right_padding);
		}
		else if (strcmp(opt_key, "top_bottom_padding") == 0) {
			SETGETCONFIGINT(osd_item->top_bottom_padding);
		}
		else if (strcmp(opt_key, "fixed_font_width") == 0) {
			SETGETCONFIGINT(osd_item->fixed_font_width);
		}
		else if (strcmp(opt_key, "update_interval") == 0) {
			SETGETCONFIGINT(osd_item->update_interval);
		}
		else {
			printf("'%s' invalid as an OSD setting!\n", opt_key);
		}
		
		osd_item->reload_flag = 1;
		pthread_mutex_unlock(&osd_item->mutex);
	}

	return EXIT_SUCCESS;
	
err:
	shmdt(camera_config);
	
	return EXIT_FAILURE;
}

int str_to_bool(char *str) {
	if (strcasecmp(str, "true") == 0) return 1;
	if (strcasecmp(str, "on") == 0) return 1;
	if (strcasecmp(str, "false") == 0) return 0;
	if (strcasecmp(str, "off") == 0) return 0;
	if (atoi(str) == 1) return 1;
	return 0;
}

void usage(char *command) {
	fprintf(stderr, USAGE, command); // "usage: %s [-h] [-s]/[-e encoder]/([-g osd_group] [-i osd_item]) [-k key] [-v value] [-o]\n"
	fprintf(stderr, "In order to change videocapture configurations, videocapture needs to be running first.\n");
	fprintf(stderr, "Changing configurations here won't save them in the configuration file.\n");
	fprintf(stderr, "The '-o' option can be used to Output the desired key value from the running configuration. In that case '-v' is not required.\n");
	fprintf(stderr, "Examples:\n\tSet night mode on (black-white image): %s -s -k night_mode -v 1\n\tGet night mode state: %s -s -k night_mode -o\n", command, command);
	fprintf(stderr, "Here is a list of all supported/required options:\n");
	
	fprintf(stderr, "\nISP Settings [-s]: (-k 'key' -v 'value')\n");
	fprintf(stderr, "\tnight_mode int[0/1]\n");
	fprintf(stderr, "\tflip_image int[0/1]\n");
	
	fprintf(stderr, "\nEncoder [-e encoder_channel]: (-k 'key' -v 'value')\n");
	fprintf(stderr, "\tframe_rate_numerator int\n");
	fprintf(stderr, "\tframe_rate_denominator int\n");
	fprintf(stderr, "\tmax_group_of_pictures int\n");
	fprintf(stderr, "\tmax_qp int[1/51]\n");
	fprintf(stderr, "\tmin_qp int[1/51]\n");
	fprintf(stderr, "\th264vbr_statistics_interval int\n");
	fprintf(stderr, "\th264vbr_max_bitrate int\n");
	fprintf(stderr, "\th264vbr_change_pos int\n");
	fprintf(stderr, "\tframe_qp_step int\n");
	fprintf(stderr, "\tgop_qp_step int\n");

	fprintf(stderr, "\nOSD(on-screen display) [-g group_id -i osd_id]: (-k 'key' -v 'value')\n");
	fprintf(stderr, "\tshow int[0/1]\n");
	fprintf(stderr, "\tpos_x int\n");
	fprintf(stderr, "\tpos_y int\n");
	fprintf(stderr, "\tsize_x int\n");
	fprintf(stderr, "\tsize_y int\n");
	fprintf(stderr, "\tlayer int\n");
	fprintf(stderr, "\tg_alpha_en int[0/1]\n");
	fprintf(stderr, "\tfg_alhpa string[2bytes, eg: '0x00']\n");
	fprintf(stderr, "\tbg_alhpa string[2bytes, eg: '0x00']\n");
	fprintf(stderr, "\tprimary_color string\n");
	fprintf(stderr, "\tsecondary_color string\n");
	fprintf(stderr, "\tline_width int\n");
	fprintf(stderr, "\timage string[dir to bgra format image/shape]\n");
	fprintf(stderr, "\ttext string\n");
	fprintf(stderr, "\tfont string\n");
	fprintf(stderr, "\textra_space_char_size int\n");
	fprintf(stderr, "\tline_spacing int\n");
	fprintf(stderr, "\tletter_spacing int\n");
	fprintf(stderr, "\tleft_right_padding int\n");
	fprintf(stderr, "\ttop_bottom_padding int\n");
	fprintf(stderr, "\tfixed_font_width int\n");
	fprintf(stderr, "\tupdate_interval int[milliseconds]\n");
	
	fprintf(stderr, "\n");
}
