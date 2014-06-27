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
    unsigned char bval = byte;                        \
    if ( bval <= 0x7F ) {                             \
      type = UTF8_ASCII_BYTE;                         \
      remaining = 0;                                  \
    }                                                 \
    else if ( bval >= 0x80 && bval <= 0xBF ) {        \
      type = UTF8_CONTINUATION_BYTE;                  \
      remaining = 0;                                  \
    }                                                 \
    else if ( bval == 0xC0 || bval == 0xC1 ) {        \
      remaining = 0;                                  \
      type = UTF8_OVERLONG_ENCODING;                  \
    }                                                 \
    else if ( bval >= 0xC2 && bval <= 0xDF ) {        \
      remaining = 1;                                  \
      type = UTF8_TWO_BYTE_SEQUENCE;                  \
    }                                                 \
    else if ( bval >= 0xE0 && bval <= 0xEF ) {        \
      remaining = 2;                                  \
      type = UTF8_THREE_BYTE_SEQUENCE;                \
    }                                                 \
    else if ( bval >= 0xF0 && bval <= 0xF4 ) {        \
      remaining = 3;                                  \
      type = UTF8_FOUR_BYTE_SEQUENCE;                 \
    }                                                 \
    else {                                            \
      remaining = 0;                                  \
      type = UTF8_INVALID_BYTE;                       \
    }                                                 \
  }

void utf8_encode( int val, char *utf8_str );
void utf8_strcpyn( char *dst, char *src, int str_len );

#endif
