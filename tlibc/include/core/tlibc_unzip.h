#ifndef _H_TLIBC_UNZIP_H
#define _H_TLIBC_UNZIP_H

#ifdef  __cplusplus
extern "C" {
#endif


#include "core/tlibc_error_code.h"

#include "zlib.h"
#include <stdint.h>
#include <stdio.h>

typedef struct _tlibc_unzip_global_info
{
    uint16_t number_entry;         /* total number of entries in
                                     the central dir on this disk */
    uint16_t size_comment;         /* size of the global comment of the zipfile */
}tlibc_unzip_global_info;

typedef struct _tlibc_unzip_file_info_s
{
    uint16_t version;              /* version made by                 2 bytes */
    uint16_t version_needed;       /* version needed to extract       2 bytes */
    uint16_t flag;                 /* general purpose bit flag        2 bytes */
    uint16_t compression_method;   /* compression method              2 bytes */
	uint16_t mod_file_time;
    uint16_t mod_file_date;
    uint32_t crc;                  /* crc-32                          4 bytes */
    uint32_t compressed_size;	   /* compressed size			      8 bytes */
    uint32_t uncompressed_size;    /* uncompressed size               8 bytes */
    uint16_t size_filename;        /* filename length                 2 bytes */
    uint16_t size_file_extra;      /* extra field length              2 bytes */
    uint16_t size_file_comment;    /* file comment length             2 bytes */
    uint16_t disk_num_start;       /* disk number start               2 bytes */
    uint16_t internal_fa;          /* internal file attributes        2 bytes */
	uint16_t unused;
    uint32_t external_fa;          /* external file attributes        4 bytes */
}tlibc_unzip_file_info_s;

typedef struct _tlibc_unzip_file_info_internal_s
{
    uint32_t offset_curfile;/* relative offset of local header 8 bytes */
}tlibc_unzip_file_info_internal_s;

typedef struct _tlibc_unzip_read_info_s
{
    char  *read_buffer;         /* internal buffer for compressed data */
    z_stream stream;            /* zLib stream structure for inflate */

	uint32_t pos_in_zipfile;       /* position in byte on the zipfile, for fseek*/
    uint32_t stream_initialised;   /* flag set if stream structure is initialised*/

    uint32_t offset_local_extrafield;/* offset of the local extra field */
    uInt  size_local_extrafield;/* size of the local extra field */
    uint32_t pos_local_extrafield;   /* position in the local extra field in read*/
    uint32_t total_out_64;

    uint32_t crc32;                /* crc32 of all data uncompressed */
    uint32_t crc32_wait;           /* crc32 we must obtain after decompress all */
    uint32_t rest_read_compressed; /* number of byte to be decompressed */
    uint32_t rest_read_uncompressed;/*number of byte to be obtained after decomp*/
    FILE* filestream;        /* io structore of the zipfile */
    uint32_t compression_method;   /* compression method (0==store) */
    uint32_t byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    int   raw;
}tlibc_unzip_read_info_s;

typedef struct _tlibc_unzip_s
{
    FILE* filestream;        /* io structore of the zipfile */


    tlibc_unzip_global_info gi;       /* public global information */
    uint32_t byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    uint32_t num_file;             /* number of the current file in the zipfile*/
    uint32_t pos_in_central_dir;   /* pos of the current file in the central dir*/
    uint32_t current_file_ok;      /* flag about the usability of the current file*/
    uint32_t central_pos;          /* position of the beginning of the central dir*/

    uint32_t size_central_dir;     /* size of the central directory  */
    uint32_t offset_central_dir;   /* offset of start of central directory with
                                   respect to the starting disk number */

    tlibc_unzip_file_info_s cur_file_info; /* public info about the current file in zip*/
    tlibc_unzip_file_info_internal_s cur_file_info_internal; /* private info about it*/
    tlibc_unzip_read_info_s pfile_in_zip_read; /* structure about the current
                                        file if we are decompressing it */
}tlibc_unzip_s;

tlibc_error_code_t tlibc_unzip_init(tlibc_unzip_s *self, const void *path);

tlibc_error_code_t tlibc_unzip_locate(tlibc_unzip_s *self, const char *szFileName);

tlibc_error_code_t tlibc_unzip_open_current_file(tlibc_unzip_s *self);

tlibc_error_code_t tlibc_read_current_file(tlibc_unzip_s *self, voidp buf, uint32_t *len);

tlibc_error_code_t tlibc_unzip_close_current_file(tlibc_unzip_s *self);

void tlibc_unzip_fini(tlibc_unzip_s *self);

#ifdef  __cplusplus
}
#endif

#endif//_H_TLIBC_UNZIP_H
