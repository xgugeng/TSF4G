#include "appender/tlog_appender_daily_directory.h"
#include <string.h>
#include <sys/stat.h>

void tlog_appender_daily_directory_init(tlog_appener_daily_directory_t *self, const tlog_config_appender_daily_directory_t *config)
{
	char *ch;

	self->fout = NULL;
	self->year = 0;
	self->month = 0;
	self->day = 0;
	self->hour = 0;
	self->min = 0;
	self->sec = 0;

	snprintf(self->file_name, TSERVER_FILE_NAME_LENGH, "%s/%s", config->directory, config->file_name);
	for(ch = self->file_name; *ch; ++ch)
	{
		if(*ch == '/')
		{
			*ch = 0;
			mkdir(self->file_name, 0755);
			*ch = '/';
		}
	}	
}

void tlog_appender_daily_directory_log(tlog_appener_daily_directory_t *self, const tlog_config_appender_daily_directory_t *config, const tlog_message_t *message)
{
	if((self->fout != NULL) && ((message->year != self->year) || (message->month != self->month) || (message->day != self->day)))
	{
		char *ch;
		char path[TSERVER_DIRECTORY_NAME_LENGH];

		fclose(self->fout);
		self->fout = NULL;

		snprintf(path, TSERVER_DIRECTORY_NAME_LENGH, "%s/%d-%d-%d/%s", config->directory, self->year, self->month, self->day, config->file_name);
		path[TSERVER_DIRECTORY_NAME_LENGH - 1] = 0;
		for(ch = path; *ch; ++ch)
		{
			if(*ch == '/')
			{
				*ch = 0;
				mkdir(path, 0755);
				*ch = '/';
			}
		}	

		rename(self->file_name, path);
	}

	if(self->fout == NULL)
	{
		self->fout = fopen(self->file_name, "ab+");
		if(self->fout == NULL)
		{
			fprintf(stderr, "daily_directory_log[%s] throw the log: %s\n", self->file_name, message->msg);
			goto done;
		}
		fseek(self->fout, 0, SEEK_END);		

		self->year = message->year;
		self->month = message->month;
		self->day = message->day;
		self->hour = message->hour;
		self->min = message->min;
		self->sec = message->sec;
	}
	

	fwrite(message->msg, 1, strlen(message->msg), self->fout);
	fputc('\n', self->fout);
	fflush(self->fout);
done:
	return;
}

void tlog_appender_daily_directory_fini(tlog_appener_daily_directory_t *self)
{
	if(self->fout != NULL)
	{
	    fclose(self->fout);
	}

	self->fout = NULL;
	self->file_name[0] = 0;

	self->year = 0;
	self->month = 0;
	self->day = 0;
	self->hour = 0;
	self->min = 0;
	self->sec = 0;
}

