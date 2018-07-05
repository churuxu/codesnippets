#include "sm2asn1.h"

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>


typedef struct SM2Signature{
    ASN1_INTEGER * r;
    ASN1_INTEGER * s;
}SM2Signature;
ASN1_SEQUENCE(SM2Signature) = {
    ASN1_SIMPLE(SM2Signature, r, ASN1_INTEGER),
    ASN1_SIMPLE(SM2Signature, s, ASN1_INTEGER), 
} ASN1_SEQUENCE_END(SM2Signature)
IMPLEMENT_ASN1_FUNCTIONS(SM2Signature)



//签名结果asn1解码
int asn1_decode_sm2sig(const unsigned char* sig,unsigned int siglen,unsigned char* buf, unsigned int* buflen ){
    SM2Signature* sm2sig = NULL;
    BIGNUM* bnr = NULL;
    BIGNUM* bns = NULL;   
    int len1, len2;

    if(*buflen<64)return 0;
    
    sm2sig = SM2Signature_new();
    if(!sm2sig)goto error;
    bnr = BN_new();
    if(!bnr)goto error;
    bns = BN_new();
    if(!bns)goto error;
    
    if(!d2i_SM2Signature(&sm2sig, &sig, siglen))goto error;
    
    if(!ASN1_INTEGER_to_BN(sm2sig->r, bnr))goto error;
    if(!ASN1_INTEGER_to_BN(sm2sig->s, bns))goto error;
    
    len1 = BN_bn2binpad(bnr, buf, 32);
    if(len1!=32)goto error;
    len2  = BN_bn2binpad(bns, buf + len1, 32);
    if(len2!=32)goto error;
        
    if(sm2sig)SM2Signature_free(sm2sig);
    if(bnr)BN_free(bnr);    
    if(bns)BN_free(bns);

    *buflen = 64;
    return 1;
    
error:
    if(sm2sig)SM2Signature_free(sm2sig);
    if(bnr)BN_free(bnr);    
    if(bns)BN_free(bns);   
    return 0;
}

//签名结果asn1编码
int asn1_encode_sm2sig(const unsigned char* rawsig, unsigned int rawsiglen, unsigned char* buf,unsigned int* buflen ){
    SM2Signature* sm2sig = NULL;
    BIGNUM* bnr = NULL;
    BIGNUM* bns = NULL;  
    unsigned char* outbuf = buf;
    int len;
    if(*buflen<72)return 0;
    
    sm2sig = SM2Signature_new();
    if(!sm2sig)goto error;
    bnr = BN_new();
    if(!bnr)goto error;
    bns = BN_new();
    if(!bns)goto error; 
  
    if(!BN_bin2bn(rawsig, 32, bnr))goto error;
    if(!BN_bin2bn(rawsig+32, 32, bns))goto error;   
    
    if(!BN_to_ASN1_INTEGER(bnr, sm2sig->r))goto error;
    if(!BN_to_ASN1_INTEGER(bns, sm2sig->s))goto error;
        
    len = i2d_SM2Signature(sm2sig, &outbuf);
    
    if(sm2sig)SM2Signature_free(sm2sig);
    if(bnr)BN_free(bnr);    
    if(bns)BN_free(bns);  

    *buflen = len;
    return 1;
    
error:  
    if(sm2sig)SM2Signature_free(sm2sig);
    if(bnr)BN_free(bnr);    
    if(bns)BN_free(bns);      
    return 0;    
}

