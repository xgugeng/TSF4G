#include "core/tlibc_unzip.h"

#include "platform/tlibc_platform.h"
#include "core/tlibc_util.h"

#include "zlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdint.h>


#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)

static tlibc_error_code_t read_uint16(FILE* filestream, uint16_t *p)
{
	if(fread(p, sizeof(int16_t), 1, filestream) != 1)
	{
		goto ERROR_RET;
	}
	tlibc_little_to_host16(*p);

	return E_TLIBC_NOERROR;
ERROR_RET:
	return E_TLIBC_ERROR;
}

static tlibc_error_code_t read_uint32(FILE* filestream, uint32_t *p)
{
	if(fread(p, sizeof(int32_t), 1, filestream) != 1)
	{
		goto ERROR_RET;
	}
	tlibc_little_to_host32(*p);

	return E_TLIBC_NOERROR;
ERROR_RET:
	return E_TLIBC_ERROR;
}

#define CENTRAL_DIR_SIGNATURE (0x06054b50)
#define CENTRAL_DIR_SIGNATURE_LENGTH (4)
//xlsx一般没有注释所以这个值很小
#define BUF_READ_COMMENT (32)
static size_t search_central_dir(FILE* filestream)
{
    char buf[BUF_READ_COMMENT + CENTRAL_DIR_SIGNATURE_LENGTH];
    size_t file_size;
    size_t pos_found=0;
	size_t read_begin;
	size_t read_end;

    if (fseek(filestream,0,SEEK_END) != 0)
        return 0;

    file_size = ftell(filestream);

	read_end = file_size;
	do
	{
		size_t read_size;		
		size_t i;
		if(read_end >= sizeof(buf))
		{
			read_begin = read_end - sizeof(buf);
		}
		else
		{
			read_begin = 0;
		}

		if (fseek((FILE*)filestream, read_begin, SEEK_SET)!=0)
			break;

		read_size = read_end - read_begin;
		if (fread(buf,1, read_size, (FILE*)filestream) != read_size)
			break;

		for(i = sizeof(buf) - CENTRAL_DIR_SIGNATURE_LENGTH + 1; i > 0 ; --i)
		{
			int flag = *(int*)(buf + i);
			if(flag == CENTRAL_DIR_SIGNATURE)
			{
				pos_found = read_begin + i;
				goto done;
			}
		}

		read_end = read_begin + CENTRAL_DIR_SIGNATURE_LENGTH;
	}while(read_begin > 0);
done:
    return pos_found;
}

static tlibc_error_code_t unz64local_GetCurrentFileInfoInternal (tlibc_unzip_s *self,
												  tlibc_unzip_file_info_s *pfile_info,
												  tlibc_unzip_file_info_internal_s *pfile_info_internal,
												  char *szFileName,
												  uint32_t fileNameBufferSize,
												  void *extraField,
												  uint32_t extraFieldBufferSize,
												  char *szComment,
												  uint32_t commentBufferSize)
{
	tlibc_unzip_file_info_s file_info;
	tlibc_unzip_file_info_internal_s file_info_internal;
	tlibc_error_code_t err=E_TLIBC_NOERROR;
	uint32_t uMagic;
	long lSeek=0;

	if (fseek(self->filestream,
		self->pos_in_central_dir + self->byte_before_the_zipfile,
		SEEK_SET)!=0)
	{
		err = E_TLIBC_ERROR;
		goto ERROR_RET;
	}


	/* we check the magic */
	if (err==E_TLIBC_NOERROR)
	{
		if (read_uint32(self->filestream,&uMagic) != E_TLIBC_NOERROR)
		{
			err = E_TLIBC_ERROR;
			goto ERROR_RET;
		}
		else if (uMagic!=0x02014b50)
		{
			err = E_TLIBC_BAD_FILE;
			goto ERROR_RET;
		}
	}

	if (read_uint16(self->filestream,&file_info.version) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}

	if (read_uint16(self->filestream,&file_info.version_needed) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}


	if (read_uint16(self->filestream,&file_info.flag) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}


	if (read_uint16(self->filestream,&file_info.compression_method) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}


	if (read_uint16(self->filestream,&file_info.mod_file_time) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}


	if (read_uint16(self->filestream,&file_info.mod_file_date) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}


	if (read_uint32(self->filestream,&file_info.crc) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}


	if (read_uint32(self->filestream,&file_info.compressed_size) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}



	if (read_uint32(self->filestream,&file_info.uncompressed_size) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}


	if (read_uint16(self->filestream,&file_info.size_filename) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}


	if (read_uint16(self->filestream,&file_info.size_file_extra) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}


	if (read_uint16(self->filestream,&file_info.size_file_comment) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}


	if (read_uint16(self->filestream,&file_info.disk_num_start) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}


	if (read_uint16(self->filestream,&file_info.internal_fa) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}


	if (read_uint32(self->filestream,&file_info.external_fa) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}


	// relative offset of local header
	if (read_uint32(self->filestream,&file_info_internal.offset_curfile) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}



	lSeek+=file_info.size_filename;
	if ((err==E_TLIBC_NOERROR) && (szFileName!=NULL))
	{
		uint32_t uSizeRead ;
		if (file_info.size_filename<fileNameBufferSize)
		{
			*(szFileName+file_info.size_filename)='\0';
			uSizeRead = file_info.size_filename;
		}
		else
			uSizeRead = fileNameBufferSize;

		if ((file_info.size_filename>0) && (fileNameBufferSize>0))
		{
			if (fread(szFileName,1, uSizeRead, self->filestream)!=uSizeRead)
			{
				err=E_TLIBC_ERROR;
				goto ERROR_RET;
			}
		}
		lSeek -= uSizeRead;
	}

	// Read extrafield
	if ((err==E_TLIBC_NOERROR) && (extraField!=NULL))
	{
		size_t uSizeRead ;
		if (file_info.size_file_extra<extraFieldBufferSize)
			uSizeRead = file_info.size_file_extra;
		else
			uSizeRead = extraFieldBufferSize;

		if (lSeek!=0)
		{
			if (fseek(self->filestream,lSeek,SEEK_CUR)==0)
			{
				lSeek=0;
			}
			else
			{
				err=E_TLIBC_ERROR;
				goto ERROR_RET;
			}

		}

		if ((file_info.size_file_extra>0) && (extraFieldBufferSize>0))
		{
			if (fread(extraField,1, (uint32_t)uSizeRead, self->filestream)!=uSizeRead)
			{
				err=E_TLIBC_ERROR;
				goto ERROR_RET;
			}
		}

		lSeek += file_info.size_file_extra - (uint32_t)uSizeRead;
	}
	else
		lSeek += file_info.size_file_extra;


	if ((err==E_TLIBC_NOERROR) && (file_info.size_file_extra != 0))
	{
		uint32_t acc = 0;

		// since lSeek now points to after the extra field we need to move back
		lSeek -= file_info.size_file_extra;

		if (lSeek!=0)
		{
			if (fseek(self->filestream,lSeek,SEEK_CUR)==0)
				lSeek=0;
			else
			{
				err=E_TLIBC_ERROR;
				goto ERROR_RET;
			}

		}

		while(acc < file_info.size_file_extra)
		{
			uint16_t headerId;
			uint16_t dataSize;

			if (read_uint16(self->filestream,&headerId) != E_TLIBC_NOERROR)
			{
				err=E_TLIBC_ERROR;
				goto ERROR_RET;
			}


			if (read_uint16(self->filestream,&dataSize) != E_TLIBC_NOERROR)
			{
				err=E_TLIBC_ERROR;
				goto ERROR_RET;
			}


			/* ZIP64 extra fields */
			if (headerId == 0x0001)
			{
				//不支持zip64格式
				err=E_TLIBC_ERROR;
				goto ERROR_RET;
			}
			else
			{
				if (fseek(self->filestream,dataSize,SEEK_CUR)!=0)
				{
					err=E_TLIBC_ERROR;
					goto ERROR_RET;
				}

			}

			acc += 2 + 2 + dataSize;
		}
	}

	if ((err==E_TLIBC_NOERROR) && (szComment!=NULL))
	{
		uint32_t uSizeRead ;
		if (file_info.size_file_comment<commentBufferSize)
		{
			*(szComment+file_info.size_file_comment)='\0';
			uSizeRead = file_info.size_file_comment;
		}
		else
			uSizeRead = commentBufferSize;

		if (lSeek!=0)
		{
			if (fseek(self->filestream,lSeek,SEEK_CUR)==0)
				lSeek=0;
			else
			{
				err=E_TLIBC_ERROR;
				goto ERROR_RET;
			}

		}

		if ((file_info.size_file_comment>0) && (commentBufferSize>0))
		{
			if (fread(szComment,1, uSizeRead, self->filestream)!=uSizeRead)
			{
				err=E_TLIBC_ERROR;
				goto ERROR_RET;
			}
		}
		lSeek+=file_info.size_file_comment - uSizeRead;
	}
	else
		lSeek+=file_info.size_file_comment;


	if ((err==E_TLIBC_NOERROR) && (pfile_info!=NULL))
		*pfile_info=file_info;

	if ((err==E_TLIBC_NOERROR) && (pfile_info_internal!=NULL))
		*pfile_info_internal=file_info_internal;

	return err;
ERROR_RET:
	return err;
}

static tlibc_error_code_t unzGoToFirstFile (tlibc_unzip_s *self)
{
    tlibc_error_code_t err=E_TLIBC_NOERROR;    
    self->pos_in_central_dir=self->offset_central_dir;
    self->num_file=0;
    err=unz64local_GetCurrentFileInfoInternal(self,&self->cur_file_info,
                                             &self->cur_file_info_internal,
                                             NULL,0,NULL,0,NULL,0);
    self->current_file_ok = (err == E_TLIBC_NOERROR);
    return err;
}

static tlibc_error_code_t  unzGoToNextFile (tlibc_unzip_s  *self)
{
    tlibc_error_code_t err;

    if (!self->current_file_ok)
        return E_TLIBC_EOF;
    if (self->gi.number_entry != 0xffff)    /* 2^16 files overflow hack */
      if (self->num_file+1==self->gi.number_entry)
        return E_TLIBC_EOF;

    self->pos_in_central_dir += SIZECENTRALDIRITEM + self->cur_file_info.size_filename +
            self->cur_file_info.size_file_extra + self->cur_file_info.size_file_comment ;
    self->num_file++;
    err = unz64local_GetCurrentFileInfoInternal(self,&self->cur_file_info,
                                               &self->cur_file_info_internal,
                                               NULL,0,NULL,0,NULL,0);
    self->current_file_ok = (err == E_TLIBC_NOERROR);
    return err;
}


extern tlibc_error_code_t  tlibc_unzip_locate (tlibc_unzip_s *self, const char *szFileName)
{
    tlibc_error_code_t err;

    /* We remember the 'current' position in the file so that we can jump
     * back there if we fail.
     */
    tlibc_unzip_file_info_s cur_file_infoSaved;
    tlibc_unzip_file_info_internal_s cur_file_info_internalSaved;
    size_t num_fileSaved;
    size_t pos_in_central_dirSaved;


    if (strlen(szFileName) >= TLIBC_MAX_PATH_LENGTH)
        return E_TLIBC_OUT_OF_MEMORY;

    /* Save the current state */
    num_fileSaved = self->num_file;
    pos_in_central_dirSaved = self->pos_in_central_dir;
    cur_file_infoSaved = self->cur_file_info;
    cur_file_info_internalSaved = self->cur_file_info_internal;

    err = unzGoToFirstFile(self);

    while (err == E_TLIBC_NOERROR)
    {
        char szCurrentFileName[TLIBC_MAX_PATH_LENGTH+1];
		
        err = unz64local_GetCurrentFileInfoInternal(self,NULL, NULL,
                                    szCurrentFileName,sizeof(szCurrentFileName)-1,
                                    NULL,0,NULL,0);
        if (err == E_TLIBC_NOERROR)
        {
            if (strcmp(szCurrentFileName, szFileName)==0)
                return E_TLIBC_NOERROR;
            err = unzGoToNextFile(self);
        }
    }

    /* We failed, so restore the state of the 'current file' to where we
     * were.
     */
    self->num_file = num_fileSaved ;
    self->pos_in_central_dir = pos_in_central_dirSaved ;
    self->cur_file_info = cur_file_infoSaved;
    self->cur_file_info_internal = cur_file_info_internalSaved;
    return err;
}

static tlibc_error_code_t unz64local_CheckCurrentFileCoherencyHeader (tlibc_unzip_s* s, uInt* piSizeVar,
                                                    size_t * poffset_local_extrafield,
                                                    uInt  * psize_local_extrafield)
{
    uint32_t uMagic, uData;
	uint16_t uFlags;
    uint16_t size_filename;
	uint16_t version_needed;
    uint16_t size_extra_field;
	uint16_t compression_method;
	uint32_t crc, compress_size, uncompress_size;
    tlibc_error_code_t err = E_TLIBC_NOERROR;

    *piSizeVar = 0;
    *poffset_local_extrafield = 0;
    *psize_local_extrafield = 0;

    if (fseek(s->filestream,s->cur_file_info_internal.offset_curfile +
                                s->byte_before_the_zipfile,SEEK_SET)!=0)
	{
        return E_TLIBC_ERROR;
	}

    if (err==E_TLIBC_NOERROR)
    {
        if (read_uint32(s->filestream,&uMagic) != E_TLIBC_NOERROR)
		{
            err=E_TLIBC_ERROR;
			goto ERROR_RET;		
		}
        else if (uMagic!=0x04034b50)
		{
            err=E_TLIBC_BAD_FILE;
			goto ERROR_RET;
		}
    }

    if (read_uint16(s->filestream,&version_needed) != E_TLIBC_NOERROR)
	{
        err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}
  
	if (read_uint16(s->filestream,&uFlags) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}

    if (read_uint16(s->filestream,&compression_method) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}
    else if ((err==E_TLIBC_NOERROR) && (compression_method!=s->cur_file_info.compression_method))
	{
		err=E_TLIBC_BAD_FILE;
		goto ERROR_RET;
	}

    if ((err==E_TLIBC_NOERROR) && (s->cur_file_info.compression_method!=0) &&
                         (s->cur_file_info.compression_method!=Z_DEFLATED))
	{
		err=E_TLIBC_BAD_FILE;
		goto ERROR_RET;
	}

    if (read_uint32(s->filestream,&uData) != E_TLIBC_NOERROR) /* date/time */
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}

    if (read_uint32(s->filestream,&crc) != E_TLIBC_NOERROR) /* crc */
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}
    else if ((err==E_TLIBC_NOERROR) && (crc!=s->cur_file_info.crc) && ((uFlags & 8)==0))
	{
		err=E_TLIBC_BAD_FILE;
		goto ERROR_RET;
	}

    if (read_uint32(s->filestream,&compress_size) != E_TLIBC_NOERROR) /* size compr */
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}
    else if (uData != 0xFFFFFFFF && (err==E_TLIBC_NOERROR) && (compress_size!=s->cur_file_info.compressed_size) && ((uFlags & 8)==0))
	{
		err=E_TLIBC_BAD_FILE;
		goto ERROR_RET;
	}

    if (read_uint32(s->filestream,&uncompress_size) != E_TLIBC_NOERROR) /* size uncompr */
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}
    else if (uData != 0xFFFFFFFF && (err==E_TLIBC_NOERROR) && (uncompress_size!=s->cur_file_info.uncompressed_size) && ((uFlags & 8)==0))
	{
		err=E_TLIBC_BAD_FILE;
		goto ERROR_RET;
	}

    if (read_uint16(s->filestream,&size_filename) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}
    else if ((err==E_TLIBC_NOERROR) && (size_filename!=s->cur_file_info.size_filename))
	{
		err=E_TLIBC_BAD_FILE;
		goto ERROR_RET;
	}

    *piSizeVar += (uInt)size_filename;

    if (read_uint16(s->filestream,&size_extra_field) != E_TLIBC_NOERROR)
	{
		err=E_TLIBC_ERROR;
		goto ERROR_RET;
	}

    *poffset_local_extrafield= s->cur_file_info_internal.offset_curfile +
                                    SIZEZIPLOCALHEADER + size_filename;
    *psize_local_extrafield = (uInt)size_extra_field;

    *piSizeVar += (uInt)size_extra_field;

    return err;
ERROR_RET:
	return err;
}

static tlibc_error_code_t  unzOpenCurrentFile3 (tlibc_unzip_s *self, int* method,
                                            int* level, int raw)
{
    tlibc_error_code_t err = E_TLIBC_NOERROR;
    uInt iSizeVar;
    tlibc_unzip_read_info_s* pfile_in_zip_read_info;
    size_t offset_local_extrafield;  /* offset of the local extra field */
    uInt  size_local_extrafield;    /* size of the local extra field */	
	

    if (!self->current_file_ok)
	{
        return E_TLIBC_ERROR;
	}


    if (unz64local_CheckCurrentFileCoherencyHeader(self,&iSizeVar, &offset_local_extrafield,&size_local_extrafield)!=E_TLIBC_NOERROR)
	{
        return E_TLIBC_BAD_FILE;
	}

    pfile_in_zip_read_info = &self->pfile_in_zip_read;

    pfile_in_zip_read_info->read_buffer=(char*)malloc(self->cur_file_info.compressed_size);
    pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
    pfile_in_zip_read_info->size_local_extrafield = size_local_extrafield;
    pfile_in_zip_read_info->pos_local_extrafield=0;
    pfile_in_zip_read_info->raw=raw;

    if (pfile_in_zip_read_info->read_buffer==NULL)
    {
        return E_TLIBC_ERROR;
    }

    pfile_in_zip_read_info->stream_initialised=0;

    if (method!=NULL)
        *method = (int)self->cur_file_info.compression_method;

    if (level!=NULL)
    {
        *level = 6;
        switch (self->cur_file_info.flag & 0x06)
        {
          case 6 : *level = 1; break;
          case 4 : *level = 2; break;
          case 2 : *level = 9; break;
        }
    }

    if ((self->cur_file_info.compression_method!=0) &&
        (self->cur_file_info.compression_method!=Z_DEFLATED))
	{
        err=E_TLIBC_BAD_FILE;
		goto ERROR_RET;
	}

    pfile_in_zip_read_info->crc32_wait=self->cur_file_info.crc;
    pfile_in_zip_read_info->crc32=0;
    pfile_in_zip_read_info->total_out_64=0;
    pfile_in_zip_read_info->compression_method = self->cur_file_info.compression_method;
    pfile_in_zip_read_info->filestream=self->filestream;
    pfile_in_zip_read_info->byte_before_the_zipfile=self->byte_before_the_zipfile;

    pfile_in_zip_read_info->stream.total_out = 0;

    if ((self->cur_file_info.compression_method==Z_DEFLATED) && (!raw))
    {
      pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
      pfile_in_zip_read_info->stream.zfree = (free_func)0;
      pfile_in_zip_read_info->stream.opaque = (voidpf)0;
      pfile_in_zip_read_info->stream.next_in = 0;
      pfile_in_zip_read_info->stream.avail_in = 0;

      if (inflateInit2(&pfile_in_zip_read_info->stream, -MAX_WBITS) == Z_OK)
        pfile_in_zip_read_info->stream_initialised=Z_DEFLATED;
      else
      {
			return E_TLIBC_ERROR;
      }
        /* windowBits is passed < 0 to tell that there is no zlib header.
         * Note that in this case inflate *requires* an extra "dummy" byte
         * after the compressed stream in order to complete decompression and
         * return Z_STREAM_END.
         * In unzip, i don't wait absolutely Z_STREAM_END because I known the
         * size of both compressed and uncompressed data
         */
    }
    pfile_in_zip_read_info->rest_read_compressed =
            self->cur_file_info.compressed_size ;
    pfile_in_zip_read_info->rest_read_uncompressed =
            self->cur_file_info.uncompressed_size ;


    pfile_in_zip_read_info->pos_in_zipfile =
            self->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER +
              iSizeVar;

    pfile_in_zip_read_info->stream.avail_in = (uInt)0;
    

    return E_TLIBC_NOERROR;
ERROR_RET:
	return err;
}


tlibc_error_code_t tlibc_unzip_init(tlibc_unzip_s *self, const void *path)
{
    size_t central_pos;
    uint32_t flag;

    uint16_t number_disk;          /* number of the current dist, used for
                                   spaning ZIP, unsupported, always 0*/
    uint16_t number_disk_with_CD;  /* number the the disk with central dir, used
                                   for spaning ZIP, unsupported, always 0*/
    uint16_t number_entry_CD;      /* total number of entries in
                                   the central dir
                                   (same than number_entry on nospan) */

    tlibc_error_code_t err = E_TLIBC_NOERROR;


	self->filestream = fopen((const char*)path, "rb");
	
    if (self->filestream==NULL)
	{
        err = E_TLIBC_NOT_FOUND;
		goto done;
	}

    central_pos = search_central_dir(self->filestream);
    if (central_pos==0)
	{
        err = E_TLIBC_ERROR;
		goto free_filestream;
	}

	if(fseek(self->filestream, central_pos, SEEK_SET) != 0)
	{
		err = E_TLIBC_ERROR;
		goto free_filestream;
	}
	
    /* the signature, already checked */
    if (read_uint32(self->filestream,&flag)!=E_TLIBC_NOERROR)
	{
        err=E_TLIBC_ERROR;
		goto free_filestream;
	}

    /* number of this disk */
    if (read_uint16(self->filestream,&number_disk)!=E_TLIBC_NOERROR)
	{
        err=E_TLIBC_ERROR;
		goto free_filestream;
	}

    /* number of the disk with the start of the central directory */
    if (read_uint16(self->filestream,&number_disk_with_CD)!=E_TLIBC_NOERROR)
	{
        err=E_TLIBC_ERROR;
		goto free_filestream;
	}

    /* total number of entries in the central dir on this disk */
    if (read_uint16(self->filestream,&self->gi.number_entry)!=E_TLIBC_NOERROR)
	{
        err=E_TLIBC_ERROR;
		goto free_filestream;
	}

    /* total number of entries in the central dir */
    if (read_uint16(self->filestream,&number_entry_CD)!=E_TLIBC_NOERROR)
	{
        err=E_TLIBC_ERROR;
		goto free_filestream;
	}

    if ((number_entry_CD!=self->gi.number_entry) ||
        (number_disk_with_CD!=0) ||
        (number_disk!=0))
	{
        err=E_TLIBC_BAD_FILE;
		goto free_filestream;
	}

    /* size of the central directory */
    if (read_uint32(self->filestream,&self->size_central_dir)!=E_TLIBC_NOERROR)
	{
        err=E_TLIBC_ERROR;
		goto free_filestream;
	}

    /* offset of start of central directory with respect to the
        starting disk number */
    if (read_uint32(self->filestream,&self->offset_central_dir)!=E_TLIBC_NOERROR)
	{
        err=E_TLIBC_ERROR;
		goto free_filestream;
	}

    /* zipfile comment length */
    if (read_uint16(self->filestream,&self->gi.size_comment)!=E_TLIBC_NOERROR)
	{
        err=E_TLIBC_ERROR;
		goto free_filestream;
	}

    if ((central_pos<self->offset_central_dir+self->size_central_dir) &&
        (err==E_TLIBC_NOERROR))
	{
        err=E_TLIBC_BAD_FILE;
		goto free_filestream;
	}

    self->byte_before_the_zipfile = central_pos - (self->offset_central_dir+self->size_central_dir);
    self->central_pos = central_pos;


    unzGoToFirstFile(self);
	return err;
free_filestream:
	fclose(self->filestream);
done:
    return err;
}

void  tlibc_unzip_fini (tlibc_unzip_s *self)
{
    fclose(self->filestream);
}

tlibc_error_code_t tlibc_unzip_open_current_file (tlibc_unzip_s *self)
{
    return unzOpenCurrentFile3(self, NULL, NULL, 0);
}

tlibc_error_code_t  tlibc_read_current_file(tlibc_unzip_s *self, voidp buf, uint32_t *len)
{
    tlibc_error_code_t err = E_TLIBC_NOERROR;
    uInt iRead = 0;
    tlibc_unzip_read_info_s* pfile_in_zip_read_info;
    pfile_in_zip_read_info=&self->pfile_in_zip_read;

    if (pfile_in_zip_read_info->read_buffer == NULL)
	{
        return E_TLIBC_ERROR;
	}


    pfile_in_zip_read_info->stream.next_out = (Bytef*)buf;

    pfile_in_zip_read_info->stream.avail_out = (uInt)*len;

    if ((*len > pfile_in_zip_read_info->rest_read_uncompressed) &&
        (!(pfile_in_zip_read_info->raw)))
        pfile_in_zip_read_info->stream.avail_out =
            (uInt)pfile_in_zip_read_info->rest_read_uncompressed;

    if ((*len > pfile_in_zip_read_info->rest_read_compressed+
           pfile_in_zip_read_info->stream.avail_in) &&
         (pfile_in_zip_read_info->raw))
        pfile_in_zip_read_info->stream.avail_out =
            (uInt)pfile_in_zip_read_info->rest_read_compressed+
            pfile_in_zip_read_info->stream.avail_in;

    while (pfile_in_zip_read_info->stream.avail_out>0)
    {
        if ((pfile_in_zip_read_info->stream.avail_in==0) &&
            (pfile_in_zip_read_info->rest_read_compressed>0))
        {
            uInt uReadThis = self->cur_file_info.compressed_size;
            if (pfile_in_zip_read_info->rest_read_compressed<uReadThis)
                uReadThis = (uInt)pfile_in_zip_read_info->rest_read_compressed;
            if (uReadThis == 0)
			{
				*len = 0;
                goto done;
			}
            if (fseek((FILE*)pfile_in_zip_read_info->filestream,
                      pfile_in_zip_read_info->pos_in_zipfile +
                         pfile_in_zip_read_info->byte_before_the_zipfile,
                         SEEK_SET)!=0)
			{
				err = E_TLIBC_ERROR;
                goto done;
			}
            if (fread(pfile_in_zip_read_info->read_buffer, 1,
                      uReadThis, (FILE*)pfile_in_zip_read_info->filestream)!=uReadThis)
			{
				err = E_TLIBC_ERROR;
				goto done;
			}

            pfile_in_zip_read_info->pos_in_zipfile += uReadThis;

            pfile_in_zip_read_info->rest_read_compressed-=uReadThis;

            pfile_in_zip_read_info->stream.next_in =
                (Bytef*)pfile_in_zip_read_info->read_buffer;
            pfile_in_zip_read_info->stream.avail_in = (uInt)uReadThis;
        }

        if ((pfile_in_zip_read_info->compression_method==0) || (pfile_in_zip_read_info->raw))
        {
            uInt uDoCopy,i ;

            if ((pfile_in_zip_read_info->stream.avail_in == 0) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
			{
				*len = iRead;
				goto done;
			}

            if (pfile_in_zip_read_info->stream.avail_out <
                            pfile_in_zip_read_info->stream.avail_in)
                uDoCopy = pfile_in_zip_read_info->stream.avail_out ;
            else
                uDoCopy = pfile_in_zip_read_info->stream.avail_in ;

            for (i=0;i<uDoCopy;i++)
                *(pfile_in_zip_read_info->stream.next_out+i) =
                        *(pfile_in_zip_read_info->stream.next_in+i);

            pfile_in_zip_read_info->total_out_64 = pfile_in_zip_read_info->total_out_64 + uDoCopy;

            pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32,
                                pfile_in_zip_read_info->stream.next_out,
                                uDoCopy);
            pfile_in_zip_read_info->rest_read_uncompressed-=uDoCopy;
            pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
            pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
            pfile_in_zip_read_info->stream.next_out += uDoCopy;
            pfile_in_zip_read_info->stream.next_in += uDoCopy;
            pfile_in_zip_read_info->stream.total_out += uDoCopy;
            iRead += uDoCopy;
        }
        else
        {
			int iret;
            size_t uTotalOutBefore,uTotalOutAfter;
            const Bytef *bufBefore;
            size_t uOutThis;
            int flush=Z_SYNC_FLUSH;

            uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;
            bufBefore = pfile_in_zip_read_info->stream.next_out;

			iret = inflate(&pfile_in_zip_read_info->stream,flush);

            if ((iret >= 0) && (pfile_in_zip_read_info->stream.msg!=NULL))
			{
				err = E_TLIBC_ERROR;
				goto done;
			}

            uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;
            uOutThis = uTotalOutAfter-uTotalOutBefore;

            pfile_in_zip_read_info->total_out_64 = pfile_in_zip_read_info->total_out_64 + uOutThis;

            pfile_in_zip_read_info->crc32 =
                crc32(pfile_in_zip_read_info->crc32,bufBefore,
                        (uInt)(uOutThis));

            pfile_in_zip_read_info->rest_read_uncompressed -=
                uOutThis;

            iRead += (uInt)(uTotalOutAfter - uTotalOutBefore);

            if (iret==Z_STREAM_END)
			{
				*len = iRead;
				goto done;
			}
            if (iret!=Z_OK)
			{
				err = E_TLIBC_ERROR;
                goto done;
			}
        }
    }

done:
    return err;
}

tlibc_error_code_t tlibc_unzip_close_current_file (tlibc_unzip_s *self)
{
    tlibc_error_code_t err=E_TLIBC_NOERROR;

    tlibc_unzip_read_info_s* pfile_in_zip_read_info;
    pfile_in_zip_read_info=&self->pfile_in_zip_read;


    if ((pfile_in_zip_read_info->rest_read_uncompressed == 0) &&
        (!pfile_in_zip_read_info->raw))
    {
        if (pfile_in_zip_read_info->crc32 != pfile_in_zip_read_info->crc32_wait)
		{
            err=E_TLIBC_ERROR;
			goto done;
		}
    }


    free(pfile_in_zip_read_info->read_buffer);
    pfile_in_zip_read_info->read_buffer = NULL;
    if (pfile_in_zip_read_info->stream_initialised == Z_DEFLATED)
        inflateEnd(&pfile_in_zip_read_info->stream);


    pfile_in_zip_read_info->stream_initialised = 0;
done:
    return err;
}
