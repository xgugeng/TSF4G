#include "tlog.h"
#include "terrno.h"

#include "protocol/tlibc_xml_reader.h"
#include "tlog_config_reader.h"
#include "tlog_rolling_file_instance.h"
#include "tlog_shm_instance.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>



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
			tlog_rolling_file_instance_init(&self->instance.appender_instance[i].body.rolling_file, &self->config.appender[i].body.rolling_file);
			break;
		case e_tlog_shm:
			tlog_shm_instance_init(&self->instance.appender_instance[i].body.shm, &self->config.appender[i].body.shm);
			break;
		}	
	}

done:
	return ret;
}

void tlog_write(tlog_t *self, const tlog_message_t *message)
{
	uint32_t i;
	
	for(i = 0; i < self->config.appender_num; ++i)	
	{
		switch(self->config.appender[i].type)
		{
			case e_tlog_rolling_file:
				tlog_rolling_file_instance_log(&self->instance.appender_instance[i].body.rolling_file, message);
				break;
			case e_tlog_shm:
			    tlog_shm_instance_log(&self->instance.appender_instance[i].body.shm, message);
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
				tlog_rolling_file_instance_fini(&self->instance.appender_instance[i].body.rolling_file);
				break;
				
            case e_tlog_shm:
                tlog_shm_instance_fini(&self->instance.appender_instance[i].body.shm);
                break;
		}
	}
}


