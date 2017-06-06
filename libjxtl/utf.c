#include <stdio.h>

#include "utf.h"

void utf8_encode( int val, char *utf8_str )
{
  int len = 0;

  if ( val >= 0x0000 && val <= 0x007f ) {
    /* Easy case, repr is 0xxx xxxx just one byte needed */
    utf8_str[0] = 0x7F & val;
    len = 1;
  }
  else if ( val >= 0x0080 && val <= 0x07ff ) {
    /* Next case, repr is 110y yyxx 10xx xxxx */
    utf8_str[1] = ( ( val & 0x3f ) | 0x80 );
    val = val >> 6;
    utf8_str[0] = ( ( val & 0x1f ) | 0xc0 );
    len = 2;
  }
  else if ( val >= 0x0800 && val <= 0xffff ) {
    /* Next case, repr is 1110 yyyy 10yy yyxx 10xx xxxx */
    utf8_str[2] = ( ( val & 0x3f ) | 0x80 );
    val = val >> 6;
    utf8_str[1] = ( ( val & 0x3f ) | 0x80 );
    val = val >> 6;
    utf8_str[0] = ( ( val & 0x0f ) | 0xe0 );
    len = 3;
  }
  else if ( val >= 0x10000 && val <= 0x10FFFF ) {
    /* Last case, repr is 1111 0zzz 10zz yyyy 10yy yyxx 10xx xxxx */
    utf8_str[3] = ( ( val & 0x3f ) | 0x80 );
    val = val >> 6;
    utf8_str[2] = ( ( val & 0x3f ) | 0x80 );
    val = val >> 6;
    utf8_str[1] = ( ( val & 0x3f ) | 0x80 );
    val = val >> 6;
    utf8_str[0] = ( ( val & 0x07 ) | 0xF0 );
    len = 4;
  }

  utf8_str[len] = '\0';
}

/**
 * Return the code point of the next character.  If for some reason this
 * function is not called at the beginning of a character sequence, it returns
 * 0.
 */
int utf8_decode_byte( char *utf8_str )
{
  enum utf8_byte_type byte_type;
  int len;
  int val = 0;

  UTF8_BYTE( utf8_str[0], byte_type, len );  
  switch ( byte_type ) {
  case UTF8_ASCII_BYTE:
    val = (int) utf8_str[0];
    break;

  case UTF8_TWO_BYTE_SEQUENCE:
    val = utf8_str[0] & 0x1f;
    val = val << 6;
    val |= utf8_str[1] & 0x3f;
    break;

  case UTF8_THREE_BYTE_SEQUENCE:
    val = utf8_str[0] & 0x0F;
    val = val << 4;
    val |= ( ( utf8_str[1] & 0x3C ) >> 2 );
    val = val << 4;
    val |= ( ( ( utf8_str[1] & 0x03 ) << 2 ) |
             ( ( utf8_str[2] & 0x30 ) >> 4 ) );
    val = val << 4;
    val |= ( ( utf8_str[2] & 0x0F ) );
    break;

  case UTF8_FOUR_BYTE_SEQUENCE:
    val = utf8_str[0] & 0x07;
    val = val << 3;
    val |= utf8_str[1] & 0x3f;
    val = val << 6;
    val |= utf8_str[2] & 0x3f;
    val = val << 6;
    val |= utf8_str[3] & 0x3f;
    break;

  case UTF8_CONTINUATION_BYTE:
  case UTF8_OVERLONG_ENCODING:
  case UTF8_INVALID_BYTE:
    val = 0;
    break;
  }

  return val;
}

void utf8_strcpyn( char *dst, char *src, int str_len )
{
  enum utf8_byte_type byte_type;
  int len;
  int tmp;
  int seq_start;
  int chars_left_in_seq;
  int invalid_seq;
  int i = 0;
  int value;

  while( i < str_len ) {
    dst[i] = src[i];
    value = 0;
    UTF8_BYTE( src[i], byte_type, len );
    switch ( byte_type ) {
    case UTF8_ASCII_BYTE:
      break;

    case UTF8_CONTINUATION_BYTE:
    case UTF8_OVERLONG_ENCODING:
    case UTF8_INVALID_BYTE:
      dst[i] = '?';
      fprintf( stderr, "unable to decode 0x%x at pos %d\n", src[i], i );
      break;

    case UTF8_TWO_BYTE_SEQUENCE:
    case UTF8_THREE_BYTE_SEQUENCE:
    case UTF8_FOUR_BYTE_SEQUENCE:
      if ( len == 1 )
        value = dst[i] & 0x1F;
      else if ( len == 2 )
        value = dst[i] & 0xF;
      else if ( len == 3 )
        value = dst[i] & 0x7;

      seq_start = i++;
      chars_left_in_seq = len;
      invalid_seq = 0;

      do {
        dst[i] = src[i];
        UTF8_BYTE( src[i], byte_type, tmp );
        if ( byte_type != UTF8_CONTINUATION_BYTE ) {
          invalid_seq = 1;
          break;
        }
        value = ( value << 6 ) + ( src[i] & 0x3F );
      }while ( ( ++i < str_len ) && ( --len > 0 )  );

      if ( invalid_seq ) {
        /* Discard the first byte that started the invalid sequence and
           continue. */
        fprintf( stderr, "unable to decode 0x%x at offset %d\n", src[seq_start],
                 seq_start );
        dst[seq_start] = '?';
        i = seq_start;
      }
      else if ( ( value > 0x10FFFF ) ||
                ( ( chars_left_in_seq == 1 ) && value < 0x80 ) ||
                ( ( chars_left_in_seq == 2 ) && value < 0x800 ) ||
                ( ( chars_left_in_seq == 3 ) && value < 0x10000 ) ||
                ( ( value >= 0xD800 ) && ( value <= 0xDFFF ) ) ) {
        /* We got a valid sequence but the value is not in the correct range
           for its length. */
        fprintf( stderr,
                 "unable to decode %d byte sequence at pos %d, invalid value 0x%x\n",
                 chars_left_in_seq + 1, seq_start, value );
        for ( len = 0; len < chars_left_in_seq; len++ ) {
          dst[seq_start + len] = '?';
        }
        i = seq_start + chars_left_in_seq;
      }
      else
        i = seq_start + chars_left_in_seq;
      break;
    }
    i++;
  }
  dst[i] = '\0';
}
