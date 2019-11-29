#ifdef USE_GTEST

#include "mempool.h"

#include "gtest/gtest.h"

static char mem[128];

static void random_buffer(uint8_t* buf, int len){
    for(int i=0;i<len;i++){
        buf[i] = rand() % 256;
    }
}


static void print_bytes(uint8_t* data, int len){    
    uint8_t* ptr = (uint8_t* )data; 
    //UNUSED(ptr);   
    for(int i=0; i<len; i++){
        if(i>60){
            printf("...");
            return;
        }
        printf("%02X ", ptr[i]);    
    }
    
}

TEST(mempool, test0){
    size_t bufsize = mempool_buffer_size(1,512);
    void* buf = malloc(bufsize);
    mempool* pool = mempool_init(buf, 1, 512);
    void* a = mempool_alloc(pool);
    void* b = mempool_alloc(pool);
    void* c = mempool_alloc(pool);
    void* d = mempool_alloc(pool);
    void* i = mempool_alloc(pool);
    mempool_free(pool,c);
    mempool_free(pool,d);
    void* e = mempool_alloc(pool);
    void* f = mempool_alloc(pool);
    void* g = mempool_alloc(pool);
    void* h = mempool_alloc(pool);    
    free(buf);
}

TEST(mempool, test1){
    mempool* pool = mempool_init(mem, 1, 16);
    void* a = mempool_alloc(pool);
    void* b = mempool_alloc(pool);
    void* c = mempool_alloc(pool);
    void* d = mempool_alloc(pool);
    void* i = mempool_alloc(pool);
    mempool_free(pool,c);
    mempool_free(pool,d);
    void* e = mempool_alloc(pool);
    void* f = mempool_alloc(pool);
    void* g = mempool_alloc(pool);
    void* h = mempool_alloc(pool);    
}

TEST(mempool, test2){
    mempool* pool = mempool_init(mem, 1, 3);
    void* a = mempool_alloc(pool);
    void* b = mempool_alloc(pool);
    void* c = mempool_alloc(pool);
    void* d = mempool_alloc(pool);
    void* e = mempool_alloc(pool);
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    ASSERT_TRUE(c);
    ASSERT_FALSE(d);
    ASSERT_FALSE(e);
    mempool_free(pool,a);
    void* f = mempool_alloc(pool);
    void* g = mempool_alloc(pool);
    ASSERT_TRUE(f);
    ASSERT_FALSE(g);    
}

TEST(mempool, test3){
    size_t bufsize = mempool_buffer_size(1,16);
    void* buf = malloc(bufsize);
    mempool* pool = mempool_init(buf, 1, 16);
    void* a = mempool_alloc(pool);
    void* b = mempool_alloc(pool);
    void* c = mempool_alloc(pool);
    void* d = mempool_alloc(pool);
    void* i = mempool_alloc(pool);
    mempool_free(pool,c);
    mempool_free(pool,d);
    void* e = mempool_alloc(pool);
    void* f = mempool_alloc(pool);
    void* g = mempool_alloc(pool);
    void* h = mempool_alloc(pool);    
    free(buf);
}


#endif

