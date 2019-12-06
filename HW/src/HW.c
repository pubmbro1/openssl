/*
 ============================================================================
 Name        : HW.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/ec.h>

#define A_PUB_FILE "afile"
#define B_PUB_FILE "bfile"

void initDomain(EVP_PKEY **params){
	EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);

	EVP_PKEY_paramgen_init(pctx);
	EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1);

	EVP_PKEY_paramgen(pctx, params);

	EVP_PKEY_CTX_free(pctx);
}

void generateKey(EVP_PKEY **params, EVP_PKEY **pkey){
	EVP_PKEY_CTX *kctx = EVP_PKEY_CTX_new( *params, NULL);

	*pkey = EVP_PKEY_new();
	EVP_PKEY_keygen_init(kctx);
	EVP_PKEY_keygen(kctx, pkey);

	EVP_PKEY_CTX_free(kctx);
}

void serializePubKey(EVP_PKEY *pkey, const char * filename){
	EC_KEY *eckey = EVP_PKEY_get1_EC_KEY(pkey);
	EC_POINT *ecpoint = EC_KEY_get0_public_key(eckey);

	int bufLen = EC_POINT_point2oct(EC_KEY_get0_group(eckey),
			ecpoint, POINT_CONVERSION_COMPRESSED, NULL, 0, NULL);

	unsigned char *pubKeyChar = (unsigned char *)malloc(bufLen);

	EC_POINT_point2oct(EC_KEY_get0_group(eckey), ecpoint,
			POINT_CONVERSION_COMPRESSED, pubKeyChar, bufLen, NULL);


	printf("\n==pubKeyChar : bob %d==\n", bufLen);
	for(int i=0;i<bufLen;i++){
		printf("0x%X ", pubKeyChar[i]);
	}

	FILE *fp = fopen(filename, "w");
	fwrite(pubKeyChar, sizeof(char), bufLen, fp);

	EC_KEY_free(eckey);
	EC_POINT_free(ecpoint);
	free(pubKeyChar);
	fclose(fp);
}

void deserializePubKey(const char * filename){
	EC_KEY *reckey = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	EC_POINT *rpoint = EC_POINT_new(EC_KEY_get0_group(reckey));

	FILE *fp = fopen(filename, "r");
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	unsigned char *pubKeyChar = (unsigned char *) malloc (len + 1);
	fseek(fp, 0, SEEK_SET);
	fread(pubKeyChar, len, 1, fp);


	EC_POINT_oct2point(EC_KEY_get0_group(reckey), rpoint,
			pubKeyChar, len, NULL);
	EC_KEY_set_public_key(reckey, rpoint);


	printf("\n==RepubKeyChar : bob %d==\n", len);
	for(int i=0;i<len;i++){
		printf("0x%X ", pubKeyChar[i]);
	}

	EC_KEY_free(reckey);
	EC_POINT_free(rpoint);
	fclose(fp);
	free(pubKeyChar);

}

void computeSharedSecret(EVP_PKEY *pkey, EVP_PKEY *peer_pub_key){
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL);
	EVP_PKEY_derive_init(ctx);
	EVP_PKEY_dervie_set_peer(ctx, peer_pub_key);

	int len = 0;

	EVP_PKEY_derive(ctx, NULL, &len);
	unsigned char *secret = (unsigned char *) malloc(len);
	EVP_PKEY_derive(ctx, secret, &len);

	EVP_PKEY_CTX_free(ctx);
}

int main(void) {
	puts("!!!Homework!!!"); /* prints !!!Hello World!!! */

	EVP_PKEY *params = NULL;		//domain
	EVP_PKEY *a_pkey = NULL;		//alice's keypair
	EVP_PKEY *b_pkey = NULL;		//bob's keypair

	/*
	 * 0. Generate EC domain params
	 * */
	initDomain(&params);
	/*
	 * 1. Generate Alice's EC Key Pair (done by A) => EVP PKEY
	 * */
	generateKey(&params, &a_pkey);
	/*
	 * 2. Generate Bob's EC Key Pair (done by B) => EVP PKEY
	 * */
	generateKey(&params, &b_pkey);
	/*
	 * 3. Serialize Bob's public key (done by B) : EVP_PKEY => char[]
	 *
	 * homework : bob's write char[] to file / alice reads
	 * */
	serializePubKey(b_pkey, B_PUB_FILE);
	/*
	 * 4. DeSerialize Bob's public key (done by A) : char[] => EVP_PKEY
	 * */
	deserializePubKey(B_PUB_FILE);
	/*
	 * 5. Compute ECDH shared secret (done by A)
	 * */
	compute()

	/*
	 * 6. Serialize A's public key (done by A) : EVP_PKEY => char[]
	 * */

	/*
	 * 7. De-serialize A's public key (done by B) : char[] => EVP_PKEY
	 * */

	/*
	 * 8. Compute ECDH shared secret (done by B)
	 * */

	return EXIT_SUCCESS;
}
