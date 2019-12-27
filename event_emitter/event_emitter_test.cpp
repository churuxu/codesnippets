#include "event_emitter.h"

#include "gtest/gtest.h"

typedef struct cb_ctx{
    cb_ctx(){
        count = 0;
        data = NULL;
        len = 0;
    }
    int count;
    void* data;
    int len;
}cb_ctx;

static void on_signal(void* ctx){
    cb_ctx* c = (cb_ctx*)ctx;
    c->count ++;
    c->data = NULL;
    c->len = 0;
}
static void on_data(void* ctx, void* data, int len){
    cb_ctx* c = (cb_ctx*)ctx;
    c->count ++;
    c->data = data;
    c->len = len;
}

//按整数ID触发事件
TEST(event_emitter, integer_id){
    uint8_t mem[256];
    cb_ctx ctx1;
    cb_ctx ctx2;
    cb_ctx ctx3;
    event_emitter* em = event_emitter_init(mem, 256, 0);

    event_emitter_on(em, EVENT_ID(1), on_signal, &ctx1);
    event_emitter_on(em, EVENT_ID(2), on_signal, &ctx2);
    event_emitter_on(em, EVENT_ID(2), on_data, &ctx3);

    event_emitter_emit(em, EVENT_ID(1));
    event_emitter_emit_data(em, EVENT_ID(2), NULL, 5);
    event_emitter_emit(em, EVENT_ID(3));
    event_emitter_emit(em, EVENT_ID(1));

    EXPECT_EQ(2, ctx1.count);
    EXPECT_EQ(1, ctx2.count);
    EXPECT_EQ(1, ctx3.count);
    EXPECT_EQ(5, ctx3.len);
}



//按静态字符串触发事件
TEST(event_emitter, str_id){
    uint8_t mem[256];
    cb_ctx ctx1;
    cb_ctx ctx2;
    cb_ctx ctx3;
    event_emitter* em = event_emitter_init(mem, 256, 0);

    event_emitter_on(em, "event1", on_signal, &ctx1);
    event_emitter_on(em, "event2", on_signal, &ctx2);
    event_emitter_on(em, "event2", on_data, &ctx3);

    event_emitter_emit(em, "event1");
    event_emitter_emit_data(em, "event2", NULL, 5);
    event_emitter_emit(em, "event3");
    event_emitter_emit(em, "event1");

    EXPECT_EQ(2, ctx1.count);
    EXPECT_EQ(1, ctx2.count);
    EXPECT_EQ(1, ctx3.count);
    EXPECT_EQ(5, ctx3.len);
}


//按字符串触发事件
TEST(event_emitter, str){
    uint8_t mem[256];
    cb_ctx ctx1;
    cb_ctx ctx2;
    cb_ctx ctx3;
    event_emitter* em = event_emitter_init(mem, 256, 16);

    event_emitter_on(em, "event1", on_signal, &ctx1);
    event_emitter_on(em, "event2", on_signal, &ctx2);
    event_emitter_on(em, "event2", on_data, &ctx3);

    event_emitter_emit(em, "event1");
    event_emitter_emit_data(em, "event2", NULL, 5);
    event_emitter_emit(em, "event3");
    event_emitter_emit(em, "event1");

    EXPECT_EQ(2, ctx1.count);
    EXPECT_EQ(1, ctx2.count);
    EXPECT_EQ(1, ctx3.count);
    EXPECT_EQ(5, ctx3.len);
}



//订阅多级
TEST(event_emitter, multi){
    uint8_t mem[256];
    cb_ctx ctx1;
    cb_ctx ctx2;
    cb_ctx ctx3;
    event_emitter* em = event_emitter_init(mem, 256, 16);

    event_emitter_on(em, "a/#", on_signal, &ctx1);
    event_emitter_on(em, "b/#", on_signal, &ctx2);
    event_emitter_on(em, "b/2/#", on_data, &ctx3);

    event_emitter_emit(em, "a/1");
    event_emitter_emit_data(em, "b/2/3", NULL, 5);
    event_emitter_emit(em, "b/1/3");
    event_emitter_emit(em, "a/2");

    EXPECT_EQ(2, ctx1.count);
    EXPECT_EQ(2, ctx2.count);
    EXPECT_EQ(1, ctx3.count);
    EXPECT_EQ(5, ctx3.len);
}
