#ifndef XML2JSON_H
#define XML2JSON_H

extern int xml_file_read( const char *filename, json_writer_t *writer,
                          int skip_root );

#endif
