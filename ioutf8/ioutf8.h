#pragma once


/*
使 windows下的 fopen等函数变为utf8编码
*/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#if defined(_WIN32) 
#include <windows.h>
#include <io.h>
#include <direct.h>
#include <sys/stat.h>
#include <tchar.h>

#define _IO_UTF8_TO_UTF16(dest, str, sz)  {dest[0]=0; MultiByteToWideChar(CP_UTF8, 0, str, -1, dest, sz);}
#define _IO_UTF16_TO_UTF8(dest, str, sz)  {dest[0]=0; WideCharToMultiByte(CP_UTF8, 0, str, -1, dest, sz, NULL, NULL);}
#define _IO_NAMESIZE MAX_PATH


static FILE* fopen_utf8(const char* name, const char* mode) {
	wchar_t namew[_IO_NAMESIZE];
	wchar_t modew[64];	
	FILE* result = NULL;
	_IO_UTF8_TO_UTF16(namew, name, _IO_NAMESIZE);
	_IO_UTF8_TO_UTF16(modew, mode, 64);
	_wfopen_s(&result, namew, modew);
	return result;
}

static FILE* freopen_utf8(const char* name, const char* mode, FILE* f) {
	wchar_t namew[_IO_NAMESIZE];
	wchar_t modew[64];
	FILE* result = NULL;
	_IO_UTF8_TO_UTF16(namew, name, _IO_NAMESIZE);
	_IO_UTF8_TO_UTF16(modew, mode, 64);
	_wfreopen_s(&result, namew, modew, f);
	return result;
}

static errno_t fopen_s_utf8(FILE** result, const char* name, const char* mode) {
	wchar_t namew[_IO_NAMESIZE];
	wchar_t modew[64];
	_IO_UTF8_TO_UTF16(namew, name, _IO_NAMESIZE);
	_IO_UTF8_TO_UTF16(modew, mode, 64);
	return _wfopen_s(result, namew, modew);
}

static errno_t freopen_s_utf8(FILE** result, const char* name, const char* mode, FILE* f) {
	wchar_t namew[_IO_NAMESIZE];
	wchar_t modew[64];
	_IO_UTF8_TO_UTF16(namew, name, _IO_NAMESIZE);
	_IO_UTF8_TO_UTF16(modew, mode, 64);
	return _wfreopen_s(result, namew, modew, f);
}


static int remove_utf8(const char* name) {
	wchar_t namew[_IO_NAMESIZE];
	_IO_UTF8_TO_UTF16(namew, name, _IO_NAMESIZE);
	return _wremove(namew);
}


static int rename_utf8(const char* oldname, const char* newname) {
	wchar_t oldnamew[_IO_NAMESIZE];
	wchar_t newnamew[_IO_NAMESIZE];
	_IO_UTF8_TO_UTF16(oldnamew, oldname, _IO_NAMESIZE);
	_IO_UTF8_TO_UTF16(newnamew, newname, _IO_NAMESIZE);
	return _wrename(oldnamew, newnamew);
}


static char* tmpnam_utf8(char* buf) {
	static char staticbuf[L_tmpnam];
	wchar_t namew[_IO_NAMESIZE];
	if (0 == _wtmpnam_s(namew, _IO_NAMESIZE)) {
		if (!buf)buf = staticbuf;
		_IO_UTF16_TO_UTF8(buf, namew, L_tmpnam);
		return buf;
	}
	return NULL;
}

static errno_t tmpnam_s_utf8(char* buf, size_t sz) {	
	wchar_t namew[_IO_NAMESIZE];
	errno_t ret;
	if (!buf || !sz)return EINVAL;
	ret = _wtmpnam_s(namew, _IO_NAMESIZE);
	if (ret) return ret;
	_IO_UTF16_TO_UTF8(buf, namew, sz);
	return 0;	
}




static int access_utf8(const char* name, int mode) {
	wchar_t namew[_IO_NAMESIZE];
	_IO_UTF8_TO_UTF16(namew, name, _IO_NAMESIZE);
	return _waccess_s(namew, mode);
}




static int chmod_utf8(const char* name, int mode) {
	wchar_t namew[_IO_NAMESIZE];
	_IO_UTF8_TO_UTF16(namew, name, _IO_NAMESIZE);	
	return _wchmod(namew, mode);
}

//static char* mktemp_utf8(char* buf);

static int unlink_utf8(const char* name) {
	wchar_t namew[_IO_NAMESIZE];
	_IO_UTF8_TO_UTF16(namew, name, _IO_NAMESIZE);
	return _wunlink(namew);
}

static int open_utf8(const char* name, int flag) {
	int fd = -1;
	wchar_t namew[_IO_NAMESIZE];	
	_IO_UTF8_TO_UTF16(namew, name, _IO_NAMESIZE);	
	_wsopen_s(&fd, namew, flag, _SH_DENYWR, 0);
	return fd;
}

static int sopen_utf8(const char* name, int flag, int share) {
	int fd = -1;
	wchar_t namew[_IO_NAMESIZE];
	_IO_UTF8_TO_UTF16(namew, name, _IO_NAMESIZE);
	_wsopen_s(&fd, namew, flag, share, 0);
	return fd;
}

static int creat_utf8(const char* name, int mode) {
	return open_utf8(name, O_CREAT | O_WRONLY | O_TRUNC);
}


static int mkdir_utf8(const char* name) {
	wchar_t namew[_IO_NAMESIZE];
	_IO_UTF8_TO_UTF16(namew, name, _IO_NAMESIZE);
	return _wmkdir(namew);
}

static int rmdir_utf8(const char* name) {
	wchar_t namew[_IO_NAMESIZE];
	_IO_UTF8_TO_UTF16(namew, name, _IO_NAMESIZE);
	return _wrmdir(namew);
}

static int chdir_utf8(const char* name) {
	wchar_t namew[_IO_NAMESIZE];
	_IO_UTF8_TO_UTF16(namew, name, _IO_NAMESIZE);
	return _wchdir(namew);
}


static char* getcwd_utf8(char* buf, size_t size){
	wchar_t namew[_IO_NAMESIZE];
	wchar_t* ret = _wgetcwd(namew, _IO_NAMESIZE);
	if (!ret)return NULL;
	_IO_UTF16_TO_UTF8(buf, namew, size);
	return buf;
}

struct stat64_utf8{
	_dev_t         st_dev;
	_ino_t         st_ino;
	unsigned short st_mode;
	short          st_nlink;
	short          st_uid;
	short          st_gid;
	_dev_t         st_rdev;
	__int64        st_size;
	__time64_t     st_atime;
	__time64_t     st_mtime;
	__time64_t     st_ctime;
};

static int stat64_utf8(const char* name, struct stat64_utf8 * buf) {
	wchar_t namew[_IO_NAMESIZE];
	_IO_UTF8_TO_UTF16(namew, name, _IO_NAMESIZE);
	return _wstat64(namew, (struct _stat64*) buf);
}



//stdio
#define fopen fopen_utf8
#define freopen freopen_utf8
#define remove remove_utf8
#define rename rename_utf8
#define tmpnam tmpnam_utf8

#define fopen_s fopen_s_utf8   
#define freopen_s freopen_s_utf8  
#define tmpnam_s tmpnam_s_utf8  

//io
#define access access_utf8
#define creat creat_utf8
#define chmod chmod_utf8
#define mktemp mktemp_utf8
#define unlink unlink_utf8
#define open open_utf8
#define sopen sopen_utf8

#define _access access_utf8
#define _creat creat_utf8
#define _chmod chmod_utf8
#define _mktemp mktemp_utf8
#define _unlink unlink_utf8
#define _open open_utf8
#define _sopen sopen_utf8


//direct
#define mkdir mkdir_utf8
#define rmdir rmdir_utf8
#define chdir chdir_utf8
#define getcwd getcwd_utf8

#define _mkdir mkdir_utf8
#define _rmdir rmdir_utf8
#define _chdir chdir_utf8
#define _getcwd getcwd_utf8

//stat
#undef _stat
#define stat stat64_utf8
#define _stat stat64_utf8






#endif



