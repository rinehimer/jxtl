#ifndef JXTL_H
#define JXTL_H

typedef struct {
  void ( *text_handler )( void *user_data, unsigned char *text );
  void ( *section_start_handler )( void *user_data, unsigned char *expr );
  void ( *section_end_handler )( void *user_data );
  void ( *if_start_handler )( void *user_data, unsigned char *expr );
  void ( *elseif_handler )( void *user_data, unsigned char *expr );
  void ( *else_handler )( void *user_data );
  void ( *if_end_handler )( void *user_data );
  void ( *separator_start_handler )( void *user_data );
  void ( *separator_end_handler )( void *user_data );
  void ( *value_handler )( void *user_data, unsigned char *expr );
  void *user_data;
} jxtl_callback_t;

#endif
