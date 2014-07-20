#include "tlog.h"
#include "tlibc_error_code.h"

#include "protocol/tlibc_xml_reader.h"
#include "core/tlibc_string.h"

#include "tlog_config_reader.h"
#include "appender/tlog_appender_rolling_file.h"
#include "appender/tlog_appender_shm.h"
#include "appender/tlog_appender_daily_directory.h"


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>

tlibc_error_code_t tlog_init(tlog_t *self, const tlog_config_t *config)
{
    int32_t i = 0;
    memcpy(&self->config, config, sizeof(tlog_config_t));
    
    self->instance.appender_vec_num = self->config.appender_vec_num;
    for(i = 0; i < self->instance.appender_vec_num; ++i)
    {
		if(i >= TLOG_MAX_APPENDER_NUM)
		{
			break;
		}

        switch(self->config.appender_vec[i].type)
        {
        case e_tlog_appender_rolling_file:
            tlog_appender_rolling_file_init(&self->instance.appender_vec[i].appender.rolling_file
                , &self->config.appender_vec[i].appender.rolling_file);            
            break;
        case e_tlog_appender_shm:
            if(tlog_appender_shm_init(&self->instance.appender_vec[i].appender.shm
                , &self->config.appender_vec[i].appender.shm) != E_TLIBC_NOERROR)
            {
                goto roll_back;
            }
            break;
		case e_tlog_appender_daily_directory:
            tlog_appender_daily_directory_init(&self->instance.appender_vec[i].appender.daily_directory
                , &self->config.appender_vec[i].appender.daily_directory);            
			break;
        }   
    }
    return E_TLIBC_NOERROR;
roll_back:
    for(--i; i >= 0; --i)
    {
        switch(self->config.appender_vec[i].type)
        {
        case e_tlog_appender_rolling_file:
            tlog_appender_rolling_file_fini(&self->instance.appender_vec[i].appender.rolling_file);
            break;
        case e_tlog_appender_shm:
            tlog_appender_shm_fini(&self->instance.appender_vec[i].appender.shm);            
            break;
		case e_tlog_appender_daily_directory:
			break;
        }   
    }
    return E_TLIBC_ERROR;
}

void tlog_write(tlog_t *self, const tlog_message_t *message)
{
	uint32_t i;

	if(!tlog_enable(self, message->level))
	{
		goto done;
	}
	
	for(i = 0; i < self->config.appender_vec_num; ++i)	
	{

		switch(self->config.appender_vec[i].type)
		{
			case e_tlog_appender_rolling_file:
				tlog_appender_rolling_file_log(&self->instance.appender_vec[i].appender.rolling_file
    				, &self->config.appender_vec[i].appender.rolling_file
				    , message);
				break;
			case e_tlog_appender_shm:
			    tlog_appender_shm_log(&self->instance.appender_vec[i].appender.shm
                , &self->config.appender_vec[i].appender.shm
                , message);
			    break;
			case e_tlog_appender_daily_directory:
			    tlog_appender_daily_directory_log(&self->instance.appender_vec[i].appender.daily_directory
                , &self->config.appender_vec[i].appender.daily_directory
                , message);
				break;
		}
	}
done:
	return;
}

void tlog_fini(tlog_t *self)
{
	int32_t i;
	i = (int32_t)self->config.appender_vec_num;
	if(i > TLOG_MAX_APPENDER_NUM)
	{
		i = TLOG_MAX_APPENDER_NUM;
	}
	
    for(; i >= 0; --i)
	{
		switch(self->config.appender_vec[i].type)
		{
			case e_tlog_appender_rolling_file:
				tlog_appender_rolling_file_fini(&self->instance.appender_vec[i].appender.rolling_file);
				break;
            case e_tlog_appender_shm:
                tlog_appender_shm_fini(&self->instance.appender_vec[i].appender.shm);
                break;
			case e_tlog_appender_daily_directory:
                tlog_appender_daily_directory_fini(&self->instance.appender_vec[i].appender.daily_directory);
				break;
		}
	}
}


