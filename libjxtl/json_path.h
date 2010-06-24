#ifndef JSON_PATH_H
#define JSON_PATH_H

typedef struct json_path_callback_t {
  void *user_data;
} json_path_callback_t;

extern int json_path_compile( const char *json_file );

#endif
