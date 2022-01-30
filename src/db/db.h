#include <sqlite3.h>
#include <models.h>

void open_database();
void get_user(char* email, User *user);
void generate_session_token(User *user, char* session_token);
