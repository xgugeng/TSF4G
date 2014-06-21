#include "protocol/tlibc_xlsx_reader.h"
#include "tlibc_xlsx_reader_sharedstring_l.h"
#include <string.h>
#include <errno.h>

#define YYGETCONDITION()  self->scanner.state
#define YYSETCONDITION(s) self->scanner.state = s
#define STATE(name)  yyc##name
#define BEGIN(state) YYSETCONDITION(STATE(state))
#define YYCURSOR self->scanner.cursor
#define YYLIMIT self->scanner.limit
#define YYMARKER self->scanner.marker
#define YYCTYPE char

tlibc_error_code_t tlibc_xlsx_reader_load_sharedstring(tlibc_xlsx_reader_t *self)
{
	char* str_begin = NULL;

	self->scanner.cursor = self->sharedstring_buff;
	self->scanner.limit = self->sharedstring_buff + self->sharedstring_buff_size;
	self->scanner.marker = self->scanner.cursor;
	self->scanner.state = yycINITIAL;
	self->sharedstring_begin_list = NULL;
	self->sharedstring_end_list = NULL;
	self->sharedstring_list_num = 0;

restart:
	if(YYCURSOR >= YYLIMIT)
	{
		return E_TLIBC_NOERROR;
	}

/*!re2c
re2c:yyfill:enable   = 0;

<INITIAL>"<sst" {BEGIN(IN_SST_TAG); goto restart;}
<IN_SST_TAG>"count=\""
{
	const char* number_begin = YYCURSOR;
	uint32_t num;
	while(*YYCURSOR != '"')
		++YYCURSOR;
	*YYCURSOR = 0;
	errno = 0;
	num = strtoul(number_begin, NULL, 10);
	if(errno != 0)
	{
		goto ERROR_RET;
	}
	self->sharedstring_begin_list = malloc(sizeof(char*) * num);
	if(self->sharedstring_begin_list == NULL)
	{
		goto ERROR_RET;
	}

	self->sharedstring_end_list = malloc(sizeof(char*) * num);
	if(self->sharedstring_end_list == NULL)
	{
		goto ERROR_RET;
	}

	while(*YYCURSOR != '>')
		++YYCURSOR;
	++YYCURSOR;
	YYMARKER = YYCURSOR;
	BEGIN(IN_SST);
	goto restart;
}
<IN_SST>"<t>"
{
	str_begin = YYCURSOR;
	goto restart;
}
<IN_SST>"</t>"
{
	self->sharedstring_begin_list[self->sharedstring_list_num] = str_begin;
	self->sharedstring_end_list[self->sharedstring_list_num] = YYCURSOR - 4;
	++self->sharedstring_list_num;
	goto restart;
}
<IN_SST>"</sst>"
{
	BEGIN(INITIAL);
	goto restart;
}
<*>[^] {goto restart;}
*/
ERROR_RET:
	if(self->sharedstring_begin_list != NULL)
	{
		free(self->sharedstring_begin_list);
	}

	if(self->sharedstring_end_list != NULL)
	{
		free(self->sharedstring_end_list);
	}
	return E_TLIBC_ERROR;
}


