#pragma once


/*
使fseek ftell变成64位
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef _FILE_OFFSET_BITS
#if _FILE_OFFSET_BITS != 64
#undef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#else
#define _FILE_OFFSET_BITS 64
#endif

static int fseek_int64(FILE* f, int64_t pos, int cur) {
#ifdef _WIN32
	return _fseeki64(f, pos, cur);
#else
	return fseeko(f, pos, cur);
#endif
}

static int64_t ftell_int64(FILE* f) {
#ifdef _WIN32
	return _ftelli64(f);
#else
	return ftello(f);
#endif
}

#define fseek fseek_int64
#define ftell ftell_int64






