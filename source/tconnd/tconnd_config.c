#include "tconnd_config.h"
#include "tcommon/terrno.h"

#include <stdio.h>


#include "tconnd/tconnd_config_reader.h"
#include "tlibc/protocol/tlibc_xml_reader.h"
#include "tconnd/tlog_instance.h"
#include "tconnd/tconnd_config_types.h"

tconnd_config_t g_config;

TERROR_CODE tconnd_config_init(const char* config_file)
{
	TLIBC_XML_READER xml_reader;


    tlibc_xml_reader_init(&xml_reader);
    if(tlibc_xml_reader_push_file(&xml_reader, config_file) != E_TLIBC_NOERROR)
    {
   		INFO_PRINT("load push config file [%s] failed.", config_file);
        goto ERROR_RET;
    }
    
	if(tlibc_read_tconnd_config_t(&xml_reader.super, &g_config) != E_TLIBC_NOERROR)
	{
		INFO_PRINT("load read file [%s] failed.", config_file);
		tlibc_xml_reader_pop_file(&xml_reader);
		goto ERROR_RET;
	}
    tlibc_xml_reader_pop_file(&xml_reader);

	return E_TS_NOERROR;
ERROR_RET:
	return E_TS_ERROR;
}
