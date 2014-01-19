#include "tlog/tlog.h"
#include "terrno.h"

#include "tlibc/protocol/tlibc_xml_reader.h"
#include "tlog/tlog_config_reader.h"

#include <stdio.h>

TERROR_CODE tlog_init(tlog_t *self, const char *config_file)
{
	TLIBC_XML_READER xml_reader;
	
	tlibc_xml_reader_init(&xml_reader, config_file);
	if(tlibc_read_tlog_config_t(&xml_reader.super, &self->config) != E_TLIBC_NOERROR)
	{
		goto ERROR_RET;
	}

	return E_TS_NOERROR;
ERROR_RET:
	return E_TS_ERROR;
}
