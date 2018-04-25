
#ifndef SQLITE_HAS_CODEC
#define SQLITE_HAS_CODEC 1 
#endif
#include "../sqlite-amalgamation/sqlite3.c"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>





#define MAX_KEY_LEN  64
#define MAX_PAGE_LEN  4096

typedef struct Codec{
	size_t pgsize;
	char offdec;  //do not decode data when read
	char offenc;  //do not encode data when write 
	char rkey[MAX_KEY_LEN]; //read key
	char wkey[MAX_KEY_LEN]; //write key
	char buf[MAX_PAGE_LEN];
}Codec;

static void CodecInitKey(char* buf, const char* key, int keylen){
	int i;
	if(keylen < MAX_KEY_LEN){  //keylen too small, padding: password\0passwordpasswordpassword 		
		memcpy(buf, key, keylen);
		buf[keylen] = 0;
		for(i = keylen + 1; i < MAX_KEY_LEN; ++i){
			buf[i] = key[(i - 1) % keylen];
		}
	}else{ 	
		memcpy(buf, key, MAX_KEY_LEN);
	}
}

static Codec* CodecCreate(const char* key, int keylen){
	Codec* result;	
	if(!key || keylen<=0)return NULL;
	result = (Codec*)malloc(sizeof(Codec));
	if(!result)return NULL;	
	result->pgsize = 0;
	result->offdec = 0;
	result->offenc = 0;
	CodecInitKey(result->rkey,key,keylen);
	CodecInitKey(result->wkey,key,keylen);
	return result;
}

static void CodecFree(void* codec){
	if(codec)free(codec);
}

//byte[i] = data[i] ^ key[i % MAX_KEY_LEN] + byte[i-1]
static void* EncodeData(const char* key, const void* data, void* output, size_t datalen){
	char* pin = (char*)data;
	char* pout = (char*)output;
	char ch = 0;	
	size_t i;
	for(i=0; i<datalen; ++i){
		ch = ((*pin) ^ key[i % MAX_KEY_LEN]) + ch;
		*pout = ch;
		pin ++;
		pout ++;
	}
	return output;
}

//data[i] = (byte[i] -  byte[i-1]) ^ key[i % MAX_KEY_LEN]
static void* DecodeData(const char* key, const void* data, void* output, size_t datalen){
	char* pin = (char*)data;
	char* pout = (char*)output;
	char ch = 0;
	char t = 0;
	size_t i;
	for(i=0; i<datalen; ++i){
		t = *pin;
		*pout = ((*pin) - ch) ^ (key[i % MAX_KEY_LEN]);
		ch = t;
		pin ++;
		pout ++;
	}
	return output;
}

static void CodecChangePageSize(void* vcodec, int newsz, int oldsz ){
	Codec* codec = (Codec*)vcodec;
	if(codec)codec->pgsize = newsz;
}

static void* CodecTransformData(void* vcodec, void* data, Pgno pgno, int mode){
	Codec* codec = (Codec*)vcodec;
	size_t pgsize = 0;
	void* result = data;
	if(!codec)return data;
	pgsize = codec->pgsize;
	if(pgsize>MAX_PAGE_LEN||pgsize==0)return data;
	switch(mode){
		case 0:	// journal file decrypt 
 		case 2:	// page reload 
		case 3:	// page load 
			//read opration, modify data buf
			if(codec->offdec)return data;
			DecodeData(codec->rkey, data, result, pgsize);
			break;
		case 6:	// page encrypt 
 		case 7:	// journal file encrypt 
			//write opration, can not modify data buf, need return new buf
			if(codec->offenc)return data;
			result = EncodeData(codec->wkey, data, codec->buf, pgsize);			
			break;
	}
	return result;	
}




#if SQLITE_VERSION_NUMBER>=3015002
#define SqliteDbName(db) db->aDb[i].zDbSName
#define SqlitePagerGetPage(pager, idx, pout)  sqlite3PagerGet(pager, (idx)+1, pout, 0)
#define SqliteBtreeRollback(btree)  sqlite3BtreeRollback(btree, SQLITE_OK, 0)
#else
#define SqliteDbName(db) db->aDb[i].zName
#define SqlitePagerGetPage(pager, idx, pout)  sqlite3PagerGet(pager, (idx)+1, pout)
#define SqliteBtreeRollback(btree)  sqlite3BtreeRollback(btree, SQLITE_OK)
#endif

#define SqliteDbGetBtreeByIndex(db, idx)  db->aDb[idx].pBt
#define SqliteDbGetBtreeByName(db, name)  db->aDb[SqliteGetIndexByName(db, name)].pBt
#define SqliteDbGetPagerByIndex(db, idx)  SqliteBtreeGetPager(SqliteDbGetBtreeByIndex(db, idx))
#define SqliteDbGetPagerByName(db, name)  SqliteBtreeGetPager(SqliteDbGetBtreeByName(db, name))

#define SqlitePagerGetCodec(pager)  sqlite3PagerGetCodec(pager)
#define SqlitePagerSetCodec(pager, codec)  sqlite3PagerSetCodec(pager, codec?CodecTransformData:NULL, codec?CodecChangePageSize:NULL, codec?CodecFree:NULL, codec)

#define SqlitePagerGetPageCount(pager, pout)  sqlite3PagerPagecount(pager, pout)
#define SqlitePageWrite(page)  sqlite3PagerWrite(page)
#define SqlitePageClose(page)  sqlite3PagerUnref(page)

#define SqliteBtreeGetPager(btree)  sqlite3BtreePager(btree)
#define SqliteBtreeBeginTrans(btree)  sqlite3BtreeBeginTrans(btree, 1)
#define SqliteBtreeCommit(btree)  sqlite3BtreeCommit(btree)



//sqlite private api alias, for different sqlite version
static int SqliteGetIndexByName(sqlite3* db, const char* name){	
	int i;
	if(db && name){
		for(i=0; i<db->nDb; i++){
			if(strcmp(SqliteDbName(db), name)==0){
				return i;
			}
		}
	}
	return 0;
}



static int SqliteSetKey(Btree* btree, const void* key, int keylen){
	Codec* codec;
	Pager* pager;
	if(!btree)return SQLITE_INTERNAL;
	pager = SqliteBtreeGetPager(btree);
	if(!pager)return SQLITE_INTERNAL;
	codec = CodecCreate((const char*)key, keylen);
	if(!codec)return SQLITE_IOERR_NOMEM;
	SqlitePagerSetCodec(pager, codec);
	return SQLITE_OK;
}


static int SqliteResetKey(Btree* btree, const void* key, int keylen){
	Codec* codec = NULL;
	Codec* srccodec = NULL;
	int pagecount = 0;
	DbPage* page = NULL;
	int ret;
	int i;	
	Pager* pager = SqliteBtreeGetPager(btree);
	if(!pager)return SQLITE_INTERNAL;

	// init codec 
	srccodec = (Codec*)SqlitePagerGetCodec(pager);
	if(srccodec){ //src is encrypted
		codec = srccodec;
		if(!key || keylen<=0){ //dest is plain
			codec->offenc = 1;
		}else{ //dest is encrypted			
			CodecInitKey(codec->wkey, (const char*)key, keylen);	
			if(memcmp(codec->rkey, codec->wkey, MAX_KEY_LEN)==0){
				//same key, do nonthing
				return SQLITE_OK;
			}
		}
	}else{ //src is plain
		if(!key || keylen<=0){ //dest is plain
			return SQLITE_OK;
		}else{ //dest is encrypted
			codec = CodecCreate((const char*)key, keylen);
			if(!codec)return SQLITE_IOERR_NOMEM;
			SqlitePagerSetCodec(pager, codec);
			codec->offdec = 1;
		}
	}

	// resave all data
	ret = SqliteBtreeBeginTrans(btree);
	if(!ret){
		pagecount = 0;
		SqlitePagerGetPageCount(pager, &pagecount);
		if(!pagecount)ret = SQLITE_INTERNAL;
		for(i=0; i<pagecount; i++){
			page = NULL;
			SqlitePagerGetPage(pager, i, &page);
			if(page){
				ret = SqlitePageWrite(page);
				SqlitePageClose(page);
				if(ret)break;
			}else{
				ret = SQLITE_INTERNAL;
				break;
			}
		}
		if(!ret){
			SqliteBtreeCommit(btree);
		}else{
			SqliteBtreeRollback(btree);
		}
	}

	// resotre codec when error
	if(ret){
		if(srccodec){
			srccodec->offenc = 0;
			srccodec->offdec = 0;
			memcpy(srccodec->wkey, srccodec->rkey, MAX_KEY_LEN);
		}else{
			SqlitePagerSetCodec(pager, NULL);
		}
		return ret;
	}

	// update codec when ok
	if(!ret){
		codec->offenc = 0;
		codec->offdec = 0;
		memcpy(codec->rkey, codec->wkey, MAX_KEY_LEN);
	}

	return ret;
}


void sqlite3CodecGetKey(sqlite3* db, int idx, void** key, int* keylen){
	Pager* pager;
	Codec* codec;
	if(key && db){
		pager = SqliteDbGetPagerByIndex(db, idx);
		if(pager){
			codec = (Codec*)SqlitePagerGetCodec(pager);
			if(codec){
				*key = codec->rkey;
				if(keylen)*keylen = MAX_KEY_LEN;
			}
		}
	}	
	if(keylen)*keylen = 0;
}

int sqlite3CodecAttach(sqlite3* db, int idx, const void* key, int keylen){
	return SqliteSetKey(SqliteDbGetBtreeByIndex(db,idx), key, keylen);
}

int sqlite3_key_v2(sqlite3 *db, const char *name, const void *key, int keylen){ 	
	return SqliteSetKey(SqliteDbGetBtreeByName(db,name), key, keylen);
}

int sqlite3_key(sqlite3 *db, const void *key, int keylen){
	int ret = 0;
	return SqliteSetKey(SqliteDbGetBtreeByIndex(db,0), key, keylen);
}

int sqlite3_rekey_v2(sqlite3 *db, const char *name, const void *key, int keylen){
	return SqliteResetKey(SqliteDbGetBtreeByName(db,name), key, keylen);
}

int sqlite3_rekey(sqlite3 *db, const void * key, int keylen){
	return SqliteResetKey(SqliteDbGetBtreeByIndex(db, 0), key, keylen);
}

void sqlite3_activate_see(const char *zPassPhrase){

}

