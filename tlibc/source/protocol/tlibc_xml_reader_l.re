#include "platform/tlibc_platform.h"
#include "protocol/tlibc_xml_reader.h"
#include "protocol/tlibc_xml_reader_scanner.h"
#include "tlibc_xml_reader_l.h"
#include <string.h>




		
tlibc_xml_reader_token_t tlibc_xml_reader_scan(tlibc_xml_reader_t *self)
{
	tlibc_xml_reader_scanner_context_t *sp = NULL;
	self->error_code = E_TLIBC_NOERROR;
restart:
	sp = &self->scanner_context_stack[self->scanner_context_stack_num - 1];
	if(YYCURSOR >= YYLIMIT)
	{
		if(self->scanner_context_stack_num > 0)
		{
			if(self->scanner_context_stack[self->scanner_context_stack_num - 1].filecontent_ptr)
			{
				free(self->scanner_context_stack[self->scanner_context_stack_num - 1].filecontent_ptr);
			}
			--self->scanner_context_stack_num;
			goto restart;
		}
		return tok_end;
	}
	sp->yylloc.first_line = sp->yylineno;
	sp->yylloc.first_column = sp->yycolumn;
	yytext = YYCURSOR;
/*!re2c
re2c:yyfill:check = 0;

identifier		([a-zA-Z_][a-zA-Z_0-9]*)
comment			("<!--")
anychar			([^])



<!*> := yyleng = YYCURSOR - yytext; tlibc_xml_reader_locate(self);

<*>{comment}
{
	const char * comment_begin = NULL;
	for(comment_begin = YYCURSOR; YYCURSOR != YYLIMIT; ++YYCURSOR)
	{
		if(YYCURSOR - comment_begin >= 2)
		{
			if((*(YYCURSOR - 2) == '-') && (*(YYCURSOR - 1) == '-') && (*(YYCURSOR - 0) == '>'))
			{
				goto restart;
			}
		}
	}
	goto restart;
}
<INITIAL>'<!include'
{
	tlibc_error_code_t r;
	const char* file_begin;
	const char* file_end;
	size_t file_len;
	char file[TLIBC_MAX_PATH_LENGTH];

	for(file_begin = NULL; YYCURSOR != YYLIMIT; ++YYCURSOR)
	{
		if(*YYCURSOR == '"')
		{
			++YYCURSOR;
			file_begin = YYCURSOR;
			break;
		}
	}
	if(file_begin == NULL)
	{
		self->error_code = E_TLIBC_SYNTAX;
		return tok_error;
	}
	for(file_end = NULL; YYCURSOR != YYLIMIT; ++YYCURSOR)
	{
		if(*YYCURSOR == '"')
		{			
			file_end = YYCURSOR;
			++YYCURSOR;
			break;
		}		
	}
	if(file_end == NULL)
	{
		self->error_code = E_TLIBC_SYNTAX;
		return tok_error;
	}
	for(;YYCURSOR != YYLIMIT; ++YYCURSOR)
	{
		if(*YYCURSOR == '>')
		{
			++YYCURSOR;
			break;
		}
	}
	file_len = file_end - file_begin;
	if(file_len >= TLIBC_MAX_PATH_LENGTH)
	{
		self->error_code = E_TLIBC_OUT_OF_MEMORY;
		return tok_error;
	}
	memcpy(file, file_begin, file_len);
	file[file_len] = 0;

	r = tlibc_xml_reader_push_file(self, file); 
	if(r != E_TLIBC_NOERROR)
	{
		self->error_code = r;
		return tok_error;
	}
	goto restart;
}
<INITIAL>'<' {identifier} '>'			{return tok_tag_begin;															}
<INITIAL>'<' '/' {identifier}'>'		{return tok_tag_end;															}

<*>{anychar}							{ goto restart;					 }	
*/	
}
