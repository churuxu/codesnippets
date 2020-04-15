#include "gtest.h"

#include "generic_packet.h"


typedef struct result{
    packet_value pv;
    char name[32];
}result;

static result r_[32];
static int i_;

static int parser_callback(void* udata, const char* name, packet_value* val){
    result* r = &r_[i_];
    memcpy(&r->pv, val, sizeof(packet_value));
    snprintf(r->name, 32,"%s",name);
    i_ ++;
    return 0;
}


TEST(generic_packet, binary){
    uint8_t pack[] = {0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x90 };
    int ret;
    
    i_ = 0;
    ret = packet_parse("u8 $addr|u16 B3 $abc123|u32", pack, sizeof(pack), parser_callback, NULL);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(2, i_); //

    EXPECT_STREQ("addr", r_[0].name); 
    EXPECT_EQ(PACKET_VALUE_INTEGER, r_[0].pv.type);
    EXPECT_EQ(0x2312, r_[0].pv.int_val);
    EXPECT_STREQ("abc123", r_[1].name); 
    EXPECT_EQ(PACKET_VALUE_INTEGER, r_[1].pv.type);
    EXPECT_EQ(0x90897867, r_[1].pv.int_val); 

}

TEST(generic_packet, str){
    char pack[] = ":01WA032;000,057,458,353,438,072,0100.C.CF\r\n";
    int ret;
    
    i_ = 0;
    ret = packet_parse("B1 D2 B2 $V1|D3 B1 $V2|D3 B1 $V3|D3 B1 $V4|D3 B1 $V5|D3 B1 $V6|D3 B1 $V7|D3 B1 $CRC|H4", pack, sizeof(pack), parser_callback, NULL);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(8, i_);

    EXPECT_STREQ("V1", r_[0].name); 
    EXPECT_EQ(PACKET_VALUE_INTEGER, r_[0].pv.type);
    EXPECT_EQ(32, r_[0].pv.int_val);
    EXPECT_STREQ("V2", r_[1].name);
    EXPECT_EQ(PACKET_VALUE_INTEGER, r_[1].pv.type);
    EXPECT_EQ(0, r_[1].pv.int_val);
    EXPECT_EQ(PACKET_VALUE_INTEGER, r_[2].pv.type);
    EXPECT_EQ(57, r_[2].pv.int_val);
    EXPECT_EQ(PACKET_VALUE_INTEGER, r_[3].pv.type);
    EXPECT_EQ(458, r_[3].pv.int_val);
    EXPECT_EQ(PACKET_VALUE_INTEGER, r_[4].pv.type);
    EXPECT_EQ(353, r_[4].pv.int_val);
    EXPECT_EQ(PACKET_VALUE_INTEGER, r_[5].pv.type);
    EXPECT_EQ(438, r_[5].pv.int_val);
    EXPECT_EQ(PACKET_VALUE_INTEGER, r_[6].pv.type);
    EXPECT_EQ(72, r_[6].pv.int_val);
    EXPECT_STREQ("CRC", r_[7].name);
    EXPECT_EQ(PACKET_VALUE_INTEGER, r_[7].pv.type);
    EXPECT_EQ(0x0100, r_[7].pv.int_val);
}

TEST(generic_packet, bcd){
    uint8_t pack[] = {0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x90 };
    int ret;
    
    i_ = 0;
    ret = packet_parse("u8 $addr|C2 B3 $abc123|C4", pack, sizeof(pack), parser_callback, NULL);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(2, i_); //

    EXPECT_STREQ("addr", r_[0].name); 
    EXPECT_EQ(PACKET_VALUE_INTEGER, r_[0].pv.type);
    EXPECT_EQ(1223, r_[0].pv.int_val);
    EXPECT_STREQ("abc123", r_[1].name); 
    EXPECT_EQ(PACKET_VALUE_INTEGER, r_[1].pv.type);
    EXPECT_EQ(67788990, r_[1].pv.int_val);
  
}



TEST(generic_packet, fbe){
    uint8_t pack[] = {0x45, 0x7A, 0x20, 0x00 };
    int ret;
    
    i_ = 0;
    ret = packet_parse("$v|F32", pack, sizeof(pack), parser_callback, NULL);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(1, i_); //

    EXPECT_STREQ("v", r_[0].name); 
    EXPECT_EQ(PACKET_VALUE_NUMBER, r_[0].pv.type);
    EXPECT_TRUE(r_[0].pv.num_val > 3900.0 && r_[0].pv.num_val < 4100.0);    
  
}


static int build_callback(void* udata, const char* name, packet_value* val){
    if(strcmp(name,"addr") == 0){
        val->int_val = 123;
        val->type = PACKET_VALUE_INTEGER;
    }else if(strcmp(name,"reg") == 0){
        val->int_val = 345;
        val->type = PACKET_VALUE_INTEGER;
    }
    return 0;
}

TEST(generic_packet, build_binary){
    uint8_t buf[64];
    int ret;
    
    ret = packet_build("11 $addr|U16 f2 a3", buf, 64, build_callback, NULL);

    EXPECT_EQ(5, ret);

  
}





