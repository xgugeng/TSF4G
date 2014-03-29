#include "tlog.h"
#include "terrno.h"

#include "protocol/tlibc_xml_reader.h"
#include "core/tlibc_string.h"

#include "tlog_config_reader.h"
#include "appender/tlog_appender_rolling_file.h"
#include "appender/tlog_appender_shm.h"


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>

static TERROR_CODE init(tlog_t *self)
{
    uint32_t i = 0;
    
    self->instance.appender_vec_num = self->config.appender_vec_num;
    for(i = 0; i < self->instance.appender_vec_num; ++i)
    {
        switch(self->config.appender_vec[i].type)
        {
        case e_tlog_appender_rolling_file:
            tlog_appender_rolling_file_init(&self->instance.appender_vec[i].appender.rolling_file
                , &self->config.appender_vec[i].appender.rolling_file);            
            break;
        case e_tlog_appender_shm:
            if(tlog_appender_shm_init(&self->instance.appender_vec[i].appender.shm
                , &self->config.appender_vec[i].appender.shm) != E_TS_NOERROR)
            {
                goto roll_back;
            }
            break;
        }   
    }
    return E_TS_NOERROR;
roll_back:
    for(--i; i > 0; --i)
    {
        switch(self->config.appender_vec[i].type)
        {
        case e_tlog_appender_rolling_file:
            tlog_appender_rolling_file_fini(&self->instance.appender_vec[i].appender.rolling_file);
            break;
        case e_tlog_appender_shm:
            tlog_appender_shm_fini(&self->instance.appender_vec[i].appender.shm);            
            break;
        }   
    }
    return E_TS_ERROR;
}

TERROR_CODE tlog_init(tlog_t *self, const tlog_config_t *config)
{
    memcpy(&self->config, config, sizeof(tlog_config_t));
    
    return init(self);
}

TERROR_CODE tlog_init_from_file(tlog_t *self, const char *config_file)
{
	TERROR_CODE ret = E_TS_NOERROR;;
	TLIBC_XML_READER xml_reader;
	TLIBC_ERROR_CODE r;
	    	
	tlibc_xml_reader_init(&xml_reader);
	if(tlibc_xml_reader_push_file(&xml_reader, config_file) != E_TLIBC_NOERROR)
	{
    	ret = E_TS_ERROR;
	    goto done;
	}

	r = tlibc_read_tlog_config_t(&xml_reader.super, &self->config);
	
	if(r != E_TLIBC_NOERROR)
	{
       	const TLIBC_XML_READER_YYLTYPE *lo = tlibc_xml_current_location(&xml_reader);
    	if(lo)
    	{
        	fprintf(stderr, "%s(%d,%d - %d,%d) %s\n"
        	    , lo->file_name
        	    , lo->first_line, lo->first_column, lo->last_line, lo->last_column
        	    , tstrerror(r));
    	}
    	else
    	{
        	fprintf(stderr, "%s %s", config_file, tstrerror(r));
    	}   	
    		
		ret = E_TS_ERROR;
		tlibc_xml_reader_pop_file(&xml_reader);
		goto done;
	}
    tlibc_xml_reader_pop_file(&xml_reader);

    return init(self); 
done:
    return ret;
}

void tlog_write(tlog_t *self, const tlog_message_t *message)
{
	uint32_t i;
	
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
		}
	}
}

void tlog_fini(tlog_t *self)
{
	uint32_t i;
	
	for(i = 0; i < self->config.appender_vec_num; ++i)	
	{
		switch(self->config.appender_vec[i].type)
		{
			case e_tlog_appender_rolling_file:
				tlog_appender_rolling_file_fini(&self->instance.appender_vec[i].appender.rolling_file);
				break;
				
            case e_tlog_appender_shm:
                tlog_appender_shm_fini(&self->instance.appender_vec[i].appender.shm);
                break;
		}
	}
}


