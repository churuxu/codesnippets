#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
sm2ǩ����asn1����ͽ���

���º����Ĳ�����
buf [out] ���������
buflen [in|out] ������������ȣ����ú�����Ϊ����������
*/


//sm2ǩ�� asn1����  ��64�ֽ�->70+�ֽڣ� �ɹ�����>0  ʧ�ܷ���=0
int asn1_encode_sm2sig(const unsigned char* rawsig, unsigned int rawsiglen, unsigned char* buf, unsigned int* buflen);

//sm2ǩ�� asn1����  ��70+�ֽ�->64�ֽڣ� �ɹ�����>0  ʧ�ܷ���=0
int asn1_decode_sm2sig(const unsigned char* sig,unsigned int siglen, unsigned char* buf,unsigned int* buflen );



#ifdef __cplusplus
}
#endif
