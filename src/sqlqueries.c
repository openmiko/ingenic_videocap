#include "capture.h"
#include "models.h"

extern sqlite3 *db;

int get_setting_callback(void *shared_data, int count, char **data, char **columns)
{
  int ret;
  char *err_msg;
  Setting *setting = (Setting *)shared_data;

  if (count == 0) {
    log_error("Expected at least one row for setting.");
    exit(1);
  }

  for (int i = 0; i < count; i++) {
    log_info("%s = %s", columns[i], data[i] ? data[i] : "NULL");
  }

  setting->id = strtol(data[0], NULL, 10);  
  strcpy(setting->name, data[1]);
  strcpy(setting->description, data[2]);
  setting->enum_value = strtol(data[3], NULL, 10);

  return 0;
}

void get_setting(int id, char* setting_name, Setting* setting) {
  int ret;
  char sql[200];
  char *err_msg;

  ret = snprintf(sql, 200, "SELECT id, name, description, enum FROM openmiko_%s where id = %d", setting_name,id);

  if(ret <= 0) {
    log_error("Unable to create SQL string.");
    exit(1);
  }

  log_info("[ExecSQL] %s", sql);
  ret = sqlite3_exec(db, sql, get_setting_callback, setting, &err_msg);
}
