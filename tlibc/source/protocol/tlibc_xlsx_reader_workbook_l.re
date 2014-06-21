#include "protocol/tlibc_xlsx_reader.h"
#include "tlibc_xlsx_reader_workbook_l.h"
#include <string.h>

#define YYGETCONDITION()  self->scanner.state
#define YYSETCONDITION(s) self->scanner.state = s
#define STATE(name)  yyc##name
#define BEGIN(state) YYSETCONDITION(STATE(state))
#define YYCURSOR self->scanner.cursor
#define YYLIMIT self->scanner.limit
#define YYMARKER self->scanner.marker
#define YYCTYPE char

const char* tlibc_xlsx_reader_workbook_search_rid(tlibc_xlsx_reader_t *self, const char* sheet)
{
	const char* rid_begin = NULL;
	const char* name_begin = NULL;

	self->scanner.cursor = self->workbook_buff;
	self->scanner.limit = self->workbook_buff + self->workbook_buff_size;
	self->scanner.marker = NULL;
	self->scanner.state = yycINITIAL;

restart:
	if(YYCURSOR >= YYLIMIT)
	{
		return NULL;
	}

/*!re2c
re2c:yyfill:enable   = 0;

<INITIAL>"<sheets>"				 { BEGIN(IN_SHEETS);goto restart;}
<IN_SHEETS>"r:id=\""			 
{
	rid_begin = YYCURSOR;
	while(*YYCURSOR != '\"')
		++YYCURSOR;
	*YYCURSOR = 0;
	++YYCURSOR;
	YYMARKER = YYCURSOR;
	goto restart;
}
<IN_SHEETS>"name=\""
{
	name_begin = YYCURSOR;
	while(*YYCURSOR != '\"')
		++YYCURSOR;
	*YYCURSOR = 0;
	++YYCURSOR;
	YYMARKER = YYCURSOR;
	goto restart;
}
<IN_SHEETS>"\"/>"
{
	if((sheet == NULL) || (strcmp(name_begin, sheet) == 0))
		return rid_begin;
	goto restart;
}
<INITIAL>"</sheets>"
{
	BEGIN(INITIAL);
	goto restart;
}
<*>[^]							 { goto restart;				}
*/
}



