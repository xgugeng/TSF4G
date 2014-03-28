#include "tlog_rolling_file.h"
#include <string.h>


void rolling_file_init(tlog_rolling_file_instance_t *self, const tlog_rolling_file_t *config)
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

void rolling_file_log(tlog_rolling_file_instance_t *self, 
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
	fputc('\n', self->fout);
	fflush(self->fout);
	
done:
	return;
}

void rolling_file_fini(tlog_rolling_file_instance_t *self)
{
	if(self->fout != NULL)
	{
	    fclose(self->fout);
        self->fout = NULL;
        self->index = 0;
	}
}

