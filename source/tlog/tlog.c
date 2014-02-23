#include "tlog/tlog.h"
#include "tcommon/terrno.h"

#include "tlibc/protocol/tlibc_xml_reader.h"
#include "tlog/tlog_config_reader.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>


static void rolling_file_init(tlog_rolling_file_instance_t *self, const tlog_rolling_file_t *config)
{
	uint32_t i;	
	char file_name[TSERVER_FILE_NAME_LENGH];
	size_t file_name_len = strlen(config->file_name);
	strncpy(file_name, config->file_name, file_name_len);
	file_name[TSERVER_FILE_NAME_LENGH - 1] = 0;
	for(i = 0; i < file_name_len; ++i)
	{
		if(file_name[i] == '/')
		{
			file_name[i] = 0;
			mkdir(file_name, 0755);
			file_name[i] = '/';
		}
	}	

	self->fout = NULL;
	self->index = 0;
}

static void rolling_file_log(tlog_rolling_file_instance_t *self, 
		const tlog_rolling_file_t *config,
		const char *message, size_t message_size)
{
	size_t file_size;
	long ft;

	if(self->fout == NULL)
	{
		self->fout = fopen(config->file_name, "wb+");
		if(self->fout == NULL)
		{
			goto done;
		}
		fseek(self->fout, 0, SEEK_END);		
	}
	
	ft = ftell(self->fout);
	if(ft < 0)
	{
	    goto done;
	}
	else
	{
	    file_size = (size_t)ft;
	}
	
	if(file_size + message_size > config->max_file_size)
	{
		char file_name[TSERVER_FILE_NAME_LENGH];
		snprintf(file_name, TSERVER_FILE_NAME_LENGH, "%s.%u", config->file_name, self->index);
		++self->index;
		if(self->index > config->max_backup_index)
		{
			self->index = 0;
		}
		
		fclose(self->fout);
		self->fout = NULL;
		
		rename(config->file_name, file_name);
		
		self->fout = fopen(config->file_name, "wb+");
		if(self->fout == NULL)
		{
			goto done;
		}
		fseek(self->fout, 0, SEEK_END);		
	}
	
	fwrite(message, 1, message_size, self->fout);
	fflush(self->fout);
	
done:
	return;
}

static void rolling_file_fini(tlog_rolling_file_instance_t *self)
{
	if(self->fout != NULL)
	{
	    fclose(self->fout);
        self->fout = NULL;
        self->index = 0;
	}
}


TERROR_CODE tlog_init(tlog_t *self, const char *config_file)
{
	TERROR_CODE ret = E_TS_NOERROR;;
	TLIBC_XML_READER xml_reader;
	uint32_t i;
	
	tlibc_xml_reader_init(&xml_reader);
	if(tlibc_xml_reader_push_file(&xml_reader, config_file) != E_TLIBC_NOERROR)
	{
    	ret = E_TS_ERROR;
	    goto done;
	}	
	if(tlibc_read_tlog_config_t(&xml_reader.super, &self->config) != E_TLIBC_NOERROR)
	{
		ret = E_TS_ERROR;
		tlibc_xml_reader_pop_file(&xml_reader);
		goto done;
	}
    tlibc_xml_reader_pop_file(&xml_reader);

	self->instance.appender_instance_num = self->config.appender_num;
	for(i = 0; i < self->instance.appender_instance_num; ++i)
	{
		switch(self->config.appender[i].type)
		{
		case e_tlog_rolling_file:
			rolling_file_init(&self->instance.appender_instance[i].rolling_file, &self->config.appender[i].rolling_file);
			break;
		}	
	}

done:
	return ret;
}

void tlog_write(tlog_t *self, const char *message, size_t message_size)
{
	uint32_t i;
	
	for(i = 0; i < self->config.appender_num; ++i)	
	{
		switch(self->config.appender[i].type)
		{
			case e_tlog_rolling_file:
				rolling_file_log(&self->instance.appender_instance[i].rolling_file, &self->config.appender[i].rolling_file, message, message_size);
				break;
		}
	}
}

void tlog_fini(tlog_t *self)
{
	uint32_t i;
	
	for(i = 0; i < self->config.appender_num; ++i)	
	{
		switch(self->config.appender[i].type)
		{
			case e_tlog_rolling_file:
				rolling_file_fini(&self->instance.appender_instance[i].rolling_file);
				break;
		}
	}
}


