#ifndef UTF_H
#define UTF_H

enum utf8_byte_type {
  UTF8_ASCII_BYTE,
  UTF8_CONTINUATION_BYTE,
  UTF8_OVERLONG_ENCODING,
  UTF8_TWO_BYTE_SEQUENCE,
  UTF8_THREE_BYTE_SEQUENCE,
  UTF8_FOUR_BYTE_SEQUENCE,
  UTF8_INVALID_BYTE
};

#define UTF8_BYTE( byte, type, remaining ) {          \
    if ( byte <= 0x7F ) {                             \
      type = UTF8_ASCII_BYTE;                         \
      remaining = 0;                                  \
    }                                                 \
    else if ( byte >= 0x80 && byte <= 0xBF ) {        \
      type = UTF8_CONTINUATION_BYTE;                  \
      remaining = 0;                                  \
    }                                                 \
    else if ( byte == 0xC0 || byte == 0xC1 ) {        \
      remaining = 0;                                  \
      type = UTF8_OVERLONG_ENCODING;                  \
    }                                                 \
    else if ( byte >= 0xC2 && byte <= 0xDF ) {        \
      remaining = 1;                                  \
      type = UTF8_TWO_BYTE_SEQUENCE;                  \
    }                                                 \
    else if ( byte >= 0xE0 && byte <= 0xEF ) {        \
      remaining = 2;                                  \
      type = UTF8_THREE_BYTE_SEQUENCE;                \
    }                                                 \
    else if ( byte >= 0xF0 && byte <= 0xF4 ) {        \
      remaining = 3;                                  \
      type = UTF8_FOUR_BYTE_SEQUENCE;                 \
    }                                                 \
    else {                                            \
      remaining = 0;                                  \
      type = UTF8_INVALID_BYTE;                       \
    }                                                 \
  }

enum utf8_byte_type utf8_check_byte( unsigned char c, int *remaining );
void utf8_encode( int val, unsigned char *utf8_str );
void utf8_strcpyn( unsigned char *dst, unsigned char *src, int str_len );

#endif
