#include "db.h"
#include "models.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"

sqlite3 *db;

void open_database()
{
  int rc = sqlite3_open("/config/openmiko.db", &db);

  if (rc != SQLITE_OK) {
    log_error("Cannot open database: %s", sqlite3_errmsg(db));
    sqlite3_close(db);
  }

}


void get_session_token(char* token)
{
  int ret;
  char *err_msg;

  const char *sql = "SELECT lower(hex(randomblob(16)));";
  sqlite3_stmt *stmt;

  ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (ret != SQLITE_OK) {
    log_error("Prepare failed: %s", sqlite3_errmsg(db));
  }

  ret = sqlite3_step(stmt);

  if (ret != SQLITE_ROW) {
    log_error("Error generating randomblob");
    return;
  }

  log_info("Generated session token: %s", sqlite3_column_text(stmt, 0));

  strcpy(token, sqlite3_column_text(stmt, 0));
  sqlite3_finalize(stmt);
}

void generate_session_token(User *user, char* session_token)
{
  int ret;
  char *err_msg;

  get_session_token(session_token);

  const char *sql = "INSERT INTO openmiko_session (session_key, session_data, expire_date, user_id) "
                    "VALUES (?, ?, datetime('now','+7 day'), ?)";
  sqlite3_stmt *stmt;

  ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (ret != SQLITE_OK) {
    log_error("Prepare failed: %s", sqlite3_errmsg(db));
  }

  sqlite3_bind_text(stmt, 1, session_token, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, "{}", -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt, 3, user->id);

  ret = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
}



void get_user(char* email, User *user)
{
  int ret;
  char *err_msg;

  const char *sql = "SELECT id, email, password FROM openmiko_user where email = ?";
  sqlite3_stmt *stmt;

  ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (ret != SQLITE_OK) {
    log_error("Prepare failed: %s", sqlite3_errmsg(db));
  }

  sqlite3_bind_text(stmt, 1, email, -1, SQLITE_TRANSIENT);

  ret = sqlite3_step(stmt);

  if (ret != SQLITE_ROW) {
    log_error("User not found");
    return;
  }

  user->id = sqlite3_column_int (stmt, 0);
  strcpy(user->email, sqlite3_column_text(stmt, 1));
  strcpy(user->password, sqlite3_column_text(stmt, 2));
  sqlite3_finalize(stmt);
}
