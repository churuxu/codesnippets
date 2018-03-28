#include "ioutf8.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>


int main() {
	unsigned char str[] = { 0xe4,0xbd,0xa0,0xe5,0xa5,0xbd,0xe5,0x95,0x8a,'.','t','x','t',0 }; // utf8±àÂëµÄ "ÄãºÃ°¡.txt" 
	char* name = (char*)str;
	FILE* f = NULL;
	int fd = 0;
	char buf[32];		
	struct stat fst;
	int iret = 0;

	f = fopen((char*)str, "wb");
	assert(f != NULL);
	fwrite("hello", 1, 5, f);
	fclose(f);

	iret = stat(name, &fst);
	assert(iret == 0);
	assert(fst.st_size == 5);

	fd = open(name, O_RDONLY);
	assert(fd >= 0);
	iret = read(fd, buf, 32);
	assert(iret == 5);

	close(fd);

	buf[iret] = 0;
	printf("%s\n", buf);	

	remove(name);

	return 0;

}
