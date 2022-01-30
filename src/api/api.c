/*
OpenMiko API
*/

#include "http.h"
#include "fio_cli.h"
#include "db.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "bcrypt.h"
#include "crypt_blowfish/ow-crypt.h"
#include "log.h"

// The general sqlite configuration database
sqlite3 *db;


FIO_LOG_LEVEL = FIO_LOG_LEVEL_DEBUG;

/* *****************************************************************************
Internal Helpers
***************************************************************************** */

/* initialize CLI helper and manage it's default options */
static void cli_init(int argc, char const *argv[]);

/* cleanup any leftovers */
static void cleanup(void);

/* reusable objects */
static FIOBJ HTTP_HEADER_SERVER;
static FIOBJ HTTP_VALUE_SERVER;

static FIOBJ HTTP_HEADER_CORS;
static FIOBJ HTTP_HEADER_CORS_VALUE;

static FIOBJ CORS_ACCESS_CONTROL_ALLOW_HEADERS_KEY;
static FIOBJ CORS_ACCESS_CONTROL_ALLOW_HEADERS_VALUE;


static FIOBJ JSON_KEY;
static FIOBJ JSON_VALUE;

/* *****************************************************************************
Routing
***************************************************************************** */

/* adds a route to our simple router */
static void route_add(char *path, void (*handler)(http_s *));

/* routes a request to the correct handler */
static void route_perform(http_s *);

/* cleanup for our router */
static void route_clear(void);

/* *****************************************************************************
Request handlers
***************************************************************************** */

/* handles JSON requests */
static void on_request_json(http_s *h);

/* handles plain text requests (Hello World) */
static void on_request_plain_text(http_s *h);

static void on_request_settings(http_s *h);
static void on_request_auth(http_s *h);
static void on_request_user(http_s *h);

/* *****************************************************************************
The main function
***************************************************************************** */
int main(int argc, char const *argv[]) {
  /* initialize the CLI helper and options */
  cli_init(argc, argv);


  open_database();

  /* setup routes */
  route_add("/json", on_request_json);
  route_add("/plaintext", on_request_plain_text);
  route_add("/settings", on_request_settings);
  route_add("/auth", on_request_auth);
  route_add("/user", on_request_user);


  /* Server name and header */
  HTTP_HEADER_SERVER = fiobj_str_new("server", 6);
  HTTP_VALUE_SERVER = fiobj_str_new("OpenMiko", 8);

  HTTP_HEADER_CORS = fiobj_str_new("Access-Control-Allow-Origin", 27);
  HTTP_HEADER_CORS_VALUE = fiobj_str_new("*", 1);

  CORS_ACCESS_CONTROL_ALLOW_HEADERS_KEY = fiobj_str_new("Access-Control-Allow-Headers", 28);
  CORS_ACCESS_CONTROL_ALLOW_HEADERS_VALUE = fiobj_str_new("*", 1);


  /* JSON values to be serialized */
  JSON_KEY = fiobj_str_new("message", 7);
  JSON_VALUE = fiobj_str_new("Hello, World!", 13);

  /* Test for static file service */
  const char *public_folder = fio_cli_get("-www");
  if (public_folder) {
    fprintf(stderr, "* serving static files from:%s\n", public_folder);
  }

  fio_cli_set_default("-port", "8000");
  fio_cli_set_default("-address", "0.0.0.0");

  /* listen to HTTP connections */
  http_listen(fio_cli_get("-port"), fio_cli_get("-address"),
              .on_request = route_perform, .public_folder = public_folder,
              .log = fio_cli_get_bool("-log"));

  /* Start the facil.io reactor */
  fio_start(.threads = fio_cli_get_i("-t"), .workers = fio_cli_get_i("-w"));

  /* perform cleanup */
  cleanup();
  return 0;
}

/* *****************************************************************************
Request handlers
***************************************************************************** */

/* handles JSON requests */
static void on_request_json(http_s *h) {
  http_set_header(h, HTTP_HEADER_CONTENT_TYPE, http_mimetype_find("json", 4));
  FIOBJ json;
  /* create a new Hash to be serialized for every request */
  FIOBJ hash = fiobj_hash_new2(1);
  fiobj_hash_set(hash, JSON_KEY, fiobj_dup(JSON_VALUE));
  json = fiobj_obj2json(hash, 0);
  fiobj_free(hash);
  fio_str_info_s tmp = fiobj_obj2cstr(json);
  http_send_body(h, tmp.data, tmp.len);
  fiobj_free(json);
}

/* handles plain text requests (Hello World) */
static void on_request_plain_text(http_s *h) {
  http_set_header(h, HTTP_HEADER_CONTENT_TYPE, http_mimetype_find("txt", 3));
  http_send_body(h, "Hello, World!", 13);
}

static void on_request_settings(http_s *h) {
  FIOBJ json;

  // authentication_required();


  http_set_header(h, HTTP_HEADER_CONTENT_TYPE, http_mimetype_find("json", 4));

  FIOBJ key = fiobj_str_new("settings", 8);
  FIOBJ value = fiobj_str_new("test", 4);

  /* create a new Hash to be serialized for every request */
  FIOBJ hash = fiobj_hash_new2(1);
  fiobj_hash_set(hash, key, fiobj_dup(value));

  json = fiobj_obj2json(hash, 0);

  fiobj_free(hash);

  fio_str_info_s tmp = fiobj_obj2cstr(json);

  http_send_body(h, tmp.data, tmp.len);

  fiobj_free(key);
  fiobj_free(value);
  fiobj_free(json);
}

static void on_request_user(http_s *h) {
  if (strcmp(fiobj_obj2cstr(h->method).data, "POST") == 0) {
    http_parse_body(h);
    FIOBJ params = h->params;
  }
}

static void on_request_auth(http_s *h) {
  FIOBJ json;

  FIO_LOG_INFO("In auth request. Method: %s", fiobj_obj2cstr(h->method).data);

  if (strcmp(fiobj_obj2cstr(h->method).data, "POST") == 0) {
    // Extract email and password
    http_parse_body(h);
    FIOBJ params = h->params;
    

    FIOBJ key_email = fiobj_str_new("email", 5);
    FIOBJ email = fiobj_hash_get(params, key_email);

    FIOBJ key_password = fiobj_str_new("password", 8);
    FIOBJ password = fiobj_hash_get(params, key_password);


    // char bcrypt_salt[BCRYPT_HASHSIZE];
    // char bcrypt_hash[BCRYPT_HASHSIZE];
    // int ret;    

    // ret = bcrypt_gensalt(12, bcrypt_salt);
    // printf("Generated bcrypt salt: %s\n", bcrypt_salt);

    // ret = bcrypt_hashpw(fiobj_obj2cstr(password).data, bcrypt_salt, bcrypt_hash);
    // printf("Hashed password: %s\n", bcrypt_hash);


    // Check if the username and password are valid


    // Retrieve the user
    User user;
    get_user(fiobj_obj2cstr(email).data, &user);

    int is_authenticated = 0;
    is_authenticated = (bcrypt_checkpw(fiobj_obj2cstr(password).data, user.password) == 0);

    char session_token[255];


    fiobj_free(params);
    fiobj_free(key_email);
    fiobj_free(email);


    if (is_authenticated) {
      log_info("User %s is authenticated", user.email);
      generate_session_token(&user, session_token);

      http_set_header(h, HTTP_HEADER_CONTENT_TYPE, http_mimetype_find("json", 4));

      FIOBJ key = fiobj_str_new("token", 5);
      FIOBJ value = fiobj_str_new(session_token, strlen(session_token));

      /* create a new Hash to be serialized for every request */
      FIOBJ hash = fiobj_hash_new2(1);
      fiobj_hash_set(hash, key, fiobj_dup(value));

      json = fiobj_obj2json(hash, 0);

      fiobj_free(hash);

      fio_str_info_s tmp = fiobj_obj2cstr(json);

      http_send_body(h, tmp.data, tmp.len);

      fiobj_free(key);
      fiobj_free(value);
      fiobj_free(json);
      return;
    }

    http_send_error(h, 401);
  }
}



/* *****************************************************************************
CLI
***************************************************************************** */

/* initialize CLI helper and manage it's default options */
static void cli_init(int argc, char const *argv[]) {
  fio_cli_start(argc, argv, 0, 0,
                "OpenMiko HTTP based API.\n"
                "\nThe following arguments are supported:",
                FIO_CLI_PRINT_HEADER("Concurrency:"),
                FIO_CLI_INT("-threads -t The number of threads to use. "
                            "System dependent default."),
                FIO_CLI_INT("-workers -w The number of processes to use. "
                            "System dependent default."),
                FIO_CLI_PRINT_HEADER("Address Binding:"),
                FIO_CLI_INT("-port -p The port number to listen to "
                            "(set to 0 for Unix Sockets."),
                FIO_CLI_STRING("-address -b The address to bind to."),
                FIO_CLI_PRINT_HEADER("HTTP Settings:"),
                FIO_CLI_STRING("-public -www A public folder for serve an HTTP "
                               "static file service."),
                FIO_CLI_BOOL("-log -v Turns logging on (logs to terminal)."),
                FIO_CLI_PRINT_HEADER("Misc:"),
                FIO_CLI_STRING("-database -db The database adrress (URL)."));

  /* setup default port */
  if (!fio_cli_get("-p")) {
    fio_cli_set("-p", "8000");
    fio_cli_set("-port", "8000");
  }

  /* setup database address */
  if (!fio_cli_get("-db")) {
    char *database = getenv("DBHOST");
    if (!database)
      database = "localhost";
    fio_cli_set("-db", database);
    fio_cli_set("-database", database);
  }
}

/* *****************************************************************************
Routing
***************************************************************************** */

typedef void (*fio_router_handler_fn)(http_s *);
#define FIO_SET_NAME fio_router
#define FIO_SET_OBJ_TYPE fio_router_handler_fn
#define FIO_SET_KEY_TYPE fio_str_s
#define FIO_SET_KEY_COPY(dest, obj) fio_str_concat(&(dest), &(obj))
#define FIO_SET_KEY_DESTROY(obj) fio_str_free(&(obj))
#define FIO_SET_KEY_COMPARE(k1, k2) fio_str_iseq(&(k1), &k2)
#define FIO_INCLUDE_STR
#define FIO_STR_NO_REF
#include <fio.h>
/* the router is a simple hash map */
static fio_router_s routes;

/* adds a route to our simple router */
static void route_add(char *path, void (*handler)(http_s *)) {
  /* add handler to the hash map */
  fio_str_s tmp = FIO_STR_INIT_STATIC(path);
  /* fio hash maps support up to 96 full collisions, we can use len as hash */
  fio_router_insert(&routes, fio_str_len(&tmp), tmp, handler, NULL);
}

/* routes a request to the correct handler */
static void route_perform(http_s *h) {
  /* add required Serevr header */
  http_set_header(h, HTTP_HEADER_SERVER, fiobj_dup(HTTP_VALUE_SERVER));
  http_set_header(h, HTTP_HEADER_CORS, fiobj_dup(HTTP_HEADER_CORS_VALUE));
  http_set_header(h, CORS_ACCESS_CONTROL_ALLOW_HEADERS_KEY, fiobj_dup(CORS_ACCESS_CONTROL_ALLOW_HEADERS_VALUE));


  /* collect path from hash map */
  fio_str_info_s tmp_i = fiobj_obj2cstr(h->path);
  fio_str_s tmp = FIO_STR_INIT_EXISTING(tmp_i.data, tmp_i.len, 0);
  fio_router_handler_fn handler = fio_router_find(&routes, tmp_i.len, tmp);
  /* forward request or send error */
  if (handler) {
    handler(h);
    return;
  }
  http_send_error(h, 404);
}

/* cleanup for our router */
static void route_clear(void) { fio_router_free(&routes); }

/* *****************************************************************************
Cleanup
***************************************************************************** */

/* cleanup any leftovers */
static void cleanup(void) {
  fio_cli_end();
  fiobj_free(HTTP_HEADER_SERVER);
  fiobj_free(HTTP_VALUE_SERVER);

  fiobj_free(HTTP_HEADER_CORS);
  fiobj_free(HTTP_HEADER_CORS_VALUE);

  fiobj_free(CORS_ACCESS_CONTROL_ALLOW_HEADERS_KEY);
  fiobj_free(CORS_ACCESS_CONTROL_ALLOW_HEADERS_VALUE);


  fiobj_free(JSON_KEY);
  fiobj_free(JSON_VALUE);

  route_clear();
}