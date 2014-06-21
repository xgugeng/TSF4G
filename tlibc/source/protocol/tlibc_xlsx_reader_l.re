#include "protocol/tlibc_xlsx_reader.h"
#include "tlibc_xlsx_reader_l.h"
#include <errno.h>

#define YYGETCONDITION()  self->scanner.state
#define YYSETCONDITION(s) self->scanner.state = s
#define STATE(name)  yyc##name
#define BEGIN(state) YYSETCONDITION(STATE(state))
#define YYCURSOR self->scanner.cursor
#define YYLIMIT self->scanner.limit
#define YYMARKER self->scanner.marker
#define YYCTYPE char
/*!re2c
re2c:yyfill:enable   = 0;
*/

static void xpos2pos(tlibc_xlsx_pos *self, const char* xpos)
{
	self->col = 0;
	while(*xpos >='A')
	{
		self->col *= 26;
		self->col += *xpos - 'A' + 1;
		++xpos;
	}
	--(self->col);

	self->row = 0;
	while(*xpos != 0)
	{
		self->row *= 10;
		self->row += *xpos - '0';
		++xpos;
	}
}

tlibc_error_code_t tlibc_xlsx_reader_loadsheet(tlibc_xlsx_reader_t *self, uint32_t bindinfo_row)
{
	tlibc_xlsx_cell_s *cell = NULL;
	int is_sharedstring = FALSE;
	tlibc_xlsx_cell_s *current_row = NULL;

	self->scanner.cursor = self->sheet_buff;
	self->scanner.limit = self->sheet_buff + self->sheet_buff_size;
	self->scanner.marker = self->scanner.cursor;
	self->scanner.state = yycINITIAL;
	self->cell_matrix = NULL;
	self->bindinfo_row = NULL;

restart:
	if(self->scanner.cursor >= self->scanner.limit)
	{
		if(self->bindinfo_row == NULL)
		{
			goto ERROR_RET;
		}
		return E_TLIBC_NOERROR;
	}
/*!re2c
<INITIAL>"<dimension" (' '|'\t')* "ref=\""
{
	uint32_t i;
	char *size_min = YYCURSOR;
	char *size_max = NULL;

	
	while(*YYCURSOR != '"')
	{
		if(*YYCURSOR == ':')
		{
			*YYCURSOR = 0;
			++YYCURSOR;
			size_max = YYCURSOR;
		}
		else
		{
			++YYCURSOR;
		}
	}
	*YYCURSOR = 0;
	++YYCURSOR;

	xpos2pos(&self->cell_min_pos, size_min);
	xpos2pos(&self->cell_max_pos, size_max);
	self->cell_row_size = (self->cell_max_pos.row - self->cell_min_pos.row + 1);
	self->cell_col_size = (self->cell_max_pos.col - self->cell_min_pos.col + 1);
	if(self->use_cache)
	{
		self->hash_cache = malloc(sizeof(tlibc_xlsx_cell_s) * self->cell_col_size);
	}
	self->cell_matrix = malloc(sizeof(tlibc_xlsx_cell_s) * self->cell_row_size * self->cell_col_size);
	if(self->cell_matrix == NULL)
	{
		goto ERROR_RET;
	}
	for(i = 0; i < self->cell_row_size * self->cell_col_size; ++i)
	{
		self->cell_matrix[i].empty = TRUE;
	}
	goto restart;
}
<INITIAL>"<sheetdata>"				{ BEGIN(IN_SHEETDATA);goto restart;	}
<IN_SHEETDATA>"<row" (' '|'\t')* "r=\""
{
	const char *r = YYCURSOR;
	uint32_t row;
	int is_single = FALSE;
	while(*YYCURSOR != '"')
	{
		++YYCURSOR;
	}
	*YYCURSOR = 0;
	errno = 0;
	row = strtoul(r, NULL, 10);
	if(errno != 0)
	{
		goto ERROR_RET;
	}
	current_row = self->cell_matrix + (row - self->cell_min_pos.row) * self->cell_col_size;

	while(*YYCURSOR != '>')
	{
		if(*YYCURSOR == '/')
		{
			is_single = TRUE;
		}
		++YYCURSOR;
	}
	++YYCURSOR;
	
	if(is_single)
	{
		goto restart;
	}

	if(row == bindinfo_row)
	{
		self->bindinfo_row = current_row;
	}
	BEGIN(IN_ROW);
	goto restart;
}
<IN_ROW>"<c"
{
	cell = NULL;
	is_sharedstring = FALSE;

	BEGIN(IN_COL);	
	goto restart;
}
<IN_COL>"r=\""
{
	const char* xpos = YYCURSOR;
	tlibc_xlsx_pos pos;
	while(*YYCURSOR != '"')
	{
		++YYCURSOR;
	}
	*YYCURSOR = 0;
	++YYCURSOR;

	xpos2pos(&pos, xpos);
	cell = current_row + (pos.col - self->cell_min_pos.col);
	cell->empty = FALSE;
	cell->xpos = xpos;
		
	goto restart;
}
<IN_COL>"t=\""
{
	if((*YYCURSOR == 's') && (*(YYCURSOR + 1) == '"'))
	{
		is_sharedstring = TRUE;
	}	
	
	while(*YYCURSOR != '>')
	{
		++YYCURSOR;
	}
	++YYCURSOR;
}
<IN_COL>"/>"
{
	BEGIN(IN_ROW);
	goto restart;
}
<IN_COL>"</c>"
{
	if(is_sharedstring)
	{
		uint32_t string_index;
		errno = 0;
		string_index = strtoul(cell->val_begin, NULL, 10);
		if(errno != 0)
		{
			goto ERROR_RET;
		}
		cell->val_begin = self->sharedstring_begin_list[string_index];
		cell->val_start = cell->val_begin;
		cell->val_end = self->sharedstring_end_list[string_index];
		cell->val_limit = cell->val_end;
	}
	BEGIN(IN_ROW);
	goto restart;
}
<IN_COL>"<v>"						{ cell->val_begin = YYCURSOR; cell->val_start = cell->val_begin; goto restart;}
<IN_COL>"</v>"						{ cell->val_end = YYCURSOR - 4; *(YYCURSOR - 4)= 0; cell->val_limit = cell->val_end; goto restart;}
<IN_ROW>"</row>"					{ BEGIN(IN_SHEETDATA); goto restart; }
<IN_SHEETDATA>"</sheetdata>"		{ BEGIN(INITIAL); goto restart;		 }
<INITIAL>"</sheetdata>"				{ BEGIN(INITIAL);goto restart;		 }
<*>[^]								{ goto restart;}
*/
ERROR_RET:
	if(self->cell_matrix != NULL)
	{
		free(self->cell_matrix);
	}
	return E_TLIBC_ERROR;
}
