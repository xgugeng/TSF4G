#include "tconnd_config.h"
#include "tcommon/terrno.h"

#include <stdio.h>


#include "tconnd/tconnd_config_reader.h"
#include "tlibc/protocol/tlibc_xml_reader.h"
#include "tlog/tlog_instance.h"
#include "tconnd/tconnd_config_types.h"

tconnd_config_t g_config;

TERROR_CODE tconnd_config_init(const char* config_file)
{
	TLIBC_XML_READER xml_reader;


    tlibc_xml_reader_init(&xml_reader);
    if(tlibc_xml_reader_push_file(&xml_reader, config_file) != E_TLIBC_NOERROR)
    {
   		ERROR_PRINT("load push config file [%s] failed.", config_file);
        goto ERROR_RET;
    }
    
	if(tlibc_read_tconnd_config_t(&xml_reader.super, &g_config) != E_TLIBC_NOERROR)
	{
    	const TLIBC_XML_READER_YYLTYPE *lo = tlibc_xml_current_location(&xml_reader);
    	if(lo)
    	{	
        	ERROR_PRINT("load read file [%s] failed at %d,%d - %d,%d.", lo->file_name, 
        	    lo->first_line, lo->first_column, lo->last_line, lo->last_column);
    	}
    	else
    	{
    	ERROR_PRINT("load read file [%s] failed.", config_file);
    	}   	
		
		tlibc_xml_reader_pop_file(&xml_reader);
		goto ERROR_RET;
	}
    tlibc_xml_reader_pop_file(&xml_reader);
    INFO_PRINT("tconnd_config_init(%s) succeed.", config_file);
    
	return E_TS_NOERROR;
ERROR_RET:
	return E_TS_ERROR;
}
