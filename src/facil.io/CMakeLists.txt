project(facil.io C)
cmake_minimum_required(VERSION 2.8)

find_package(Threads REQUIRED)

set(facil.io_SOURCES
  lib/facil/fio.c
  lib/facil/tls/fio_tls_missing.c
  lib/facil/tls/fio_tls_openssl.c
  lib/facil/fiobj/fio_siphash.c
  lib/facil/fiobj/fiobj_ary.c
  lib/facil/fiobj/fiobj_data.c
  lib/facil/fiobj/fiobj_hash.c
  lib/facil/fiobj/fiobj_json.c
  lib/facil/fiobj/fiobj_mustache.c
  lib/facil/fiobj/fiobj_numbers.c
  lib/facil/fiobj/fiobj_str.c
  lib/facil/fiobj/fiobject.c
  lib/facil/cli/fio_cli.c
  lib/facil/http/http.c
  lib/facil/http/http1.c
  lib/facil/http/http_internal.c
  lib/facil/http/websockets.c
#  lib/facil/redis/redis_engine.c
)


set(facil.io_HEADERS
  lib/facil/fio.h
  lib/facil/tls/fio_tls.h
  lib/facil/legacy/fio_mem.h
  lib/facil/cli/fio_cli.h
  lib/facil/fiobj/fiobj_str.h
  lib/facil/fiobj/fiobj_ary.h
  lib/facil/fiobj/fio_tmpfile.h
  lib/facil/fiobj/fiobj4fio.h
  lib/facil/fiobj/fiobj_data.h
  lib/facil/fiobj/mustache_parser.h
  lib/facil/fiobj/fiobj_mustache.h
  lib/facil/fiobj/fiobj.h
  lib/facil/fiobj/fio_siphash.h
  lib/facil/fiobj/fiobj_hash.h
  lib/facil/fiobj/fiobj_json.h
  lib/facil/fiobj/fio_json_parser.h
  lib/facil/fiobj/fiobj_numbers.h
  lib/facil/fiobj/fiobject.h
  lib/facil/redis/redis_engine.h
  lib/facil/redis/resp_parser.h
  lib/facil/http/http.h
  lib/facil/http/http_internal.h
  lib/facil/http/websockets.h
  lib/facil/http/http1.h
  lib/facil/http/parsers/http_mime_parser.h
  lib/facil/http/parsers/websocket_parser.h
  lib/facil/http/parsers/http1_parser.h
  lib/facil/http/parsers/hpack.h
)

add_library(facil.io SHARED ${facil.io_SOURCES})
target_link_libraries(facil.io
  PRIVATE Threads::Threads
  PUBLIC pthread
  PUBLIC m
)

target_include_directories(facil.io
  PUBLIC  lib
  PUBLIC  lib/facil
  PUBLIC  lib/facil/tls
  PUBLIC  lib/facil/fiobj
  PUBLIC  lib/facil/cli
  PUBLIC  lib/facil/http
  PUBLIC  lib/facil/http/parsers
#  PUBLIC  lib/facil/redis
)


