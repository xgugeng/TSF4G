#include "protocol/tlibc_xlsx_reader.h"
#include "tlibc_xlsx_reader_workbook_rels_l.h"
#include <string.h>

#define YYGETCONDITION()  self->scanner.state
#define YYSETCONDITION(s) self->scanner.state = s
#define STATE(name)  yyc##name
#define BEGIN(state) YYSETCONDITION(STATE(state))
#define YYCURSOR self->scanner.cursor
#define YYLIMIT self->scanner.limit
#define YYMARKER self->scanner.marker
#define YYCTYPE char

const char* tlibc_xlsx_reader_workbook_rels_search_file(tlibc_xlsx_reader_t *self, const char* rid)
{
	const char* target_begin = NULL;
	const char* rid_begin = NULL;

	self->scanner.cursor = self->workbook_rels_buff;
	self->scanner.limit = self->workbook_rels_buff + self->workbook_rels_buff_size;
	self->scanner.marker = NULL;	
	self->scanner.state = yycINITIAL;

restart:
	if(YYCURSOR >= YYLIMIT)
	{
		return NULL;
	}
/*!re2c
re2c:yyfill:enable   = 0;

<INITIAL>"<relationships"	 
{
	while(*YYCURSOR != '>')
		++YYCURSOR;
	++YYCURSOR;
	YYMARKER = YYCURSOR;
	BEGIN(IN_RELATIONSHIPS);
	goto restart;
}
<IN_RELATIONSHIPS>"target=\""
{
	target_begin = YYCURSOR;
	while(*YYCURSOR != '\"')
		++YYCURSOR;
	*YYCURSOR = 0;
	++YYCURSOR;
	YYMARKER = YYCURSOR;
	goto restart;
}
<IN_RELATIONSHIPS>"id=\""
{
	rid_begin = YYCURSOR;
	while(*YYCURSOR != '\"')
		++YYCURSOR;
	*YYCURSOR = 0;
	++YYCURSOR;
	YYMARKER = YYCURSOR;
	goto restart;
}
<IN_RELATIONSHIPS>">"
{ 
	if(strcmp(rid_begin, rid) == 0)
		return target_begin;

	target_begin = NULL;
	rid_begin = NULL;
	goto restart;
}
<INITIAL>"</relationships>"		 { BEGIN(INITIAL);				}
<*>[^]							 { goto restart;				}
*/
}
