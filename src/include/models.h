#ifndef MODELS_H
#define MODELS_H

typedef struct user {
	int id;
	char email[255];
	char password[255];
} User;

typedef struct setting {
	int id;
	char name[255];
	char description[255];
	int enum_value;
} Setting;

typedef struct ae_strategy {
	int id;
	char name[255];
	char description[255];
	int enum_value;
} AeStrategy;

typedef struct anti_flicker_attr {
	int id;
	char name[255];
	char description[255];
	int enum_value;
} AntiflickerAttr;

typedef struct scene_mode {
	int id;
	char name[255];
	char description[255];
	int enum_value;
} SceneMode;

typedef struct color_fx_mode {
	int id;
	char name[255];
	char description[255];
	int enum_value;
} ColorFxMode;


void get_setting(int id, char* setting_name, Setting* setting);

#endif
