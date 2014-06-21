#include "core/tlibc_string.h"

const char *tstrerror(tlibc_error_code_t terrno)
{
	switch (terrno)
	{
	case E_TLIBC_NOERROR:
		return "正常";
	case E_TLIBC_ERROR:
		return "错误";
	case E_TLIBC_OUT_OF_MEMORY:
		return "内存溢出";
	case E_TLIBC_NOT_FOUND:
		return "找不到";
	case E_TLIBC_SYNTAX:
		return "语法错误";
	case E_TLIBC_MISMATCH:
		return "不匹配";
	case E_TLIBC_ERRNO:
		return "errno";
	case E_TLIBC_WOULD_BLOCK:
		return "会被阻塞";
	case E_TLIBC_BAD_FILE:
		return "错误的文件";
	case E_TLIBC_EOF:
		return "文件结束";
	case E_TLIBC_EMPTY:
		return "目标为空";
	case E_TLIBC_INTEGER_OVERFLOW:
		return "整数溢出";
	case E_TLIBC_IGNORE:
		return "忽略";
	case E_TLIBC_PLEASE_READ_ENUM_NAME:
		return "请读取枚举名";
	default:
		return "未知错误";
	}
}
