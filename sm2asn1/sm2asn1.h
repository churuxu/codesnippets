#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
sm2签名的asn1编码和解码

以下函数的参数：
buf [out] 输出缓冲区
buflen [in|out] 输出缓冲区长度，调用函数后，为输出结果长度
*/


//sm2签名 asn1编码  （64字节->70+字节） 成功返回>0  失败返回=0
int asn1_encode_sm2sig(const unsigned char* rawsig, unsigned int rawsiglen, unsigned char* buf, unsigned int* buflen);

//sm2签名 asn1解码  （70+字节->64字节） 成功返回>0  失败返回=0
int asn1_decode_sm2sig(const unsigned char* sig,unsigned int siglen, unsigned char* buf,unsigned int* buflen );



#ifdef __cplusplus
}
#endif
