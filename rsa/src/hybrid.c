/*
 * hybrid.c
 *
 *  Created on: 2019. 11. 28.
 *      Author: shyun
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>



int main(){
	/*
	 * generate Bob's RSA key pair (struct EVP_PKEY)
	 */

	EVP_PKEY_CTX *keyCtx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
	if(keyCtx == NULL) goto cleanup;
	EVP_PKEY_keygen_init(keyCtx);
	EVP_PKEY_CTX_set_rsa_keygen_bits(keyCtx, 2048);

	EVP_PKEY *bob_keypair = NULL;
	EVP_PKEY_keygen(keyCtx, &bob_keypair);
	if(bob_keypair == NULL) goto cleanup;
 	EVP_PKEY_CTX_free(keyCtx);

	/*
	 * Serialize Bob's public key
	 * */
 	BIO *publicBIO = BIO_new(BIO_s_mem());
 	if(publicBIO == NULL) goto cleanup;
 	PEM_write_bio_PUBKEY(publicBIO, bob_keypair);
 	int publicKeyLen = BIO_pending(publicBIO);
 	unsigned char *pubkeyChar = malloc(publicKeyLen+1);
 	BIO_read(publicBIO, pubkeyChar, publicKeyLen);
 	pubkeyChar[publicKeyLen] = '\0';

 	puts(pubkeyChar);

 	/*
 	 * Deserialize char[] Bob's pub -> EVP_PKEY (by Alice)
 	 * */
 	unsigned char *rsaPublicKeyChar = pubkeyChar;
 	BIO *rsaPublicBIO = BIO_new_mem_buf(rsaPublicKeyChar, - 1);
 	RSA *rsaPublicKey = NULL;
 	PEM_read_bio_RSA_PUBKEY(rsaPublicBIO, &rsaPublicKey, NULL, NULL);

 	EVP_PKEY *publicKey = EVP_PKEY_new();
 	EVP_PKEY_assign_RSA(publicKey, rsaPublicKey);

	/*
	 * Serialize Bob's private key
	 * */
 	BIO *privateBIO = BIO_new(BIO_s_mem());
 	if(privateBIO == NULL) goto cleanup;
 	PEM_write_bio_PrivateKey(privateBIO, bob_keypair, NULL, NULL,0, 0, NULL);
 	int privateKeyLen = BIO_pending(privateBIO);
 	unsigned char *privateChar = malloc(privateKeyLen+1);
 	BIO_read(privateBIO, privateChar, privateKeyLen);
 	privateChar[publicKeyLen] = '\0';





	/*
	 * EVP_SealInit => Data enc init
	 * */

 	EVP_CIPHER_CTX *rsaEncryptCtx = EVP_CIPHER_CTX_new();
 	//ek = AES key ctxt(encrypted with the publicKey)
 	unsigned char *ek = (unsigned char *) malloc(EVP_PKEY_size(publicKey));
 	int ekLen = 0;
 	unsigned char *iv = (unsigned char *) malloc(EVP_MAX_IV_LENGTH);

 	EVP_SealInit(rsaEncryptCtx, EVP_aes_128_cbc(), &ek, &ekLen, iv,
 			&publicKey, 1);

 	for(int i=0;i<ekLen; i++){
 		printf("0x%X ", ek[i]);
 	}


 	/*
 	 * Encrypt Msg
 	 * */

 	char *msg = "This is Message!!";
 	int msglen = strlen(msg) + 1;

 	unsigned char *ctxt = (unsigned char * ) malloc (msglen +
 			EVP_MAX_IV_LENGTH);
 	if(ctxt == NULL) goto cleanup;

 	int ctxt_len = 0;
 	int encBlockLen = 0; // (=tmp)

 	EVP_SealUpdate(rsaEncryptCtx, ctxt, &encBlockLen, msg, msglen);
 	ctxt_len = encBlockLen;
 	EVP_SealFinal(rsaEncryptCtx, ctxt + encBlockLen, &encBlockLen);
 	ctxt_len += encBlockLen;

 	ctxt[ctxt_len] = '\0';

 	printf("\n== CTXT ==\n");
 	for(int i=0;i<ctxt_len;i++){
 		printf("0x%X ", ctxt[i]);
 	}

 	/*
 	 * Deserialize Bob's private key : char[] -> EVP_PKEY
 	 */

 	unsigned char *rsaPrivateKeyChar = privateChar;
 	BIO *rsaPrivateBIO = BIO_new_mem_buf(rsaPrivateKeyChar, -1);
 	RSA *rsaPrivateKey = NULL;
 	PEM_read_bio_RSAPrivateKey(rsaPrivateBIO, &rsaPrivateKey, NULL, NULL);
 	EVP_PKEY *privateKey = EVP_PKEY_new();
 	EVP_PKEY_assign_RSA(privateKey, rsaPrivateKey);

 	printf("\nend\n");

 	EVP_CIPHER_CTX *rsaDecryptCtx = EVP_CIPHER_CTX_new();
 	printf("\nend2\n");
 	EVP_OpenInit(rsaDecryptCtx, EVP_aes_128_cbc(), ek, ekLen, iv, privateKey);
 	printf("\nend3\n");


 	unsigned char *rtxt = (unsigned char*) malloc (ctxt_len +
 			EVP_CIPHER_CTX_block_size(rsaDecryptCtx)+1);
 	int rtxt_len = 0;
 	int decBlockLen = 0;

 	EVP_OpenUpdate(rsaDecryptCtx, rtxt, &decBlockLen, ctxt, ctxt_len);
 	rtxt_len = decBlockLen;

 	EVP_OpenFinal(rsaDecryptCtx, rtxt + decBlockLen, &decBlockLen);
 	rtxt_len += decBlockLen;
 	rtxt[rtxt_len] = '\0';



 	printf("Decrypted >> %s\n", rtxt);


cleanup:
	if(keyCtx != NULL) EVP_PKEY_CTX_free(keyCtx);
	if(bob_keypair != NULL) EVP_PKEY_free(bob_keypair);
	if(publicBIO != NULL) BIO_free_all(publicBIO);
	if(privateBIO != NULL) BIO_free_all(privateBIO);
	//if(rsaEncryptCtx != NULL) EVP_CIPHER_free_all(rsaEncryptCtx);
	if(ctxt != NULL) free(ctxt);
	//if(rtxt != NULL) free(rtxt);
	return 0;
}
