/*
 ============================================================================
 Name        : ecdh.c
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

int main(void) {
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */

	/* Public Key serialize
	 * peerkey -> (EVP_PKEY_get1_EC_Key) -> peer_eckey -> (EVP_PKEY_get0_public_key
	 * 				-> peer_pubc_ecpoint -> (EC_POINT_point2oct) -> char[] *peerPubKeyChar
	 *  */


	// p = Alice, peer = Bob

	/*
	 * 0. Generate EC domain params
	 * */
	EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
	if(pctx == NULL) goto cleanup;
	EVP_PKEY_paramgen_init(pctx);

	EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1);
	EVP_PKEY *params = NULL;
	EVP_PKEY_paramgen(pctx, &params);

	/*
	 * 1. Generate Alice's EC Key Pair (done by A) => EVP PKEY
	 * */
	EVP_PKEY_CTX *kctx = EVP_PKEY_CTX_new(params, NULL);
	if(kctx == NULL) goto cleanup;
	EVP_PKEY_keygen_init(kctx);

	EVP_PKEY *pkey = EVP_PKEY_new();
	EVP_PKEY_keygen(kctx, &pkey);

	EVP_PKEY_CTX_free(kctx); // free to reuse

	/*
	 * 2. Generate Bob's EC Key Pair (done by B) => EVP PKEY
	 * */
	kctx = EVP_PKEY_CTX_new(params, NULL);
	if(kctx == NULL) goto cleanup;
	EVP_PKEY_keygen_init(kctx);

	EVP_PKEY *peerkey = EVP_PKEY_new();
	EVP_PKEY_keygen(kctx, &peerkey);

	/*
	 * 3. Serialize Bob's public key (done by B) : EVP_PKEY => char[]
	 *
	 * homework : bob's write char[] to file / alice reads
	 * */
	EC_KEY *peer_eckey = EVP_PKEY_get1_EC_KEY(peerkey);
	EC_POINT *peer_pub_ecpoint = EC_KEY_get0_public_key(peer_eckey);
	if(peer_eckey == NULL || peer_pub_ecpoint == NULL) goto cleanup;

	// get bob's public key length
	size_t buflen = EC_POINT_point2oct(EC_KEY_get0_group(peer_eckey),
			peer_pub_ecpoint, POINT_CONVERSION_COMPRESSED, NULL, 0, NULL);

	unsigned char *peerPubkeyChar = (unsigned char*)malloc(buflen);
	if(peerPubkeyChar == NULL) goto cleanup;
	// bob's pub key to char[]
	EC_POINT_point2oct(EC_KEY_get0_group(peer_eckey), peer_pub_ecpoint,
			POINT_CONVERSION_COMPRESSED, peerPubkeyChar, buflen, NULL);

	// write to file (peerPubkeyChar)
	FILE *file_bob = fopen("file_bob", "w");
	if(file_bob == NULL) return -1;
	fwrite(peerPubkeyChar, sizeof(char), strlen(peerPubkeyChar)+1, file_bob);
	fclose(file_bob);

	/*
	 * 4. DeSerialize Bob's public key (done by A) : char[] => EVP_PKEY
	 * */

	// read from file
	FILE *file_bob = fopen("file_bob", "r");
	if(file_bob == NULL) return -1;
	int fSzie = ftell(file_bob);
	fread()

	EC_KEY *reckey = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	EC_POINT *rpoint = EC_POINT_new(EC_KEY_get0_group(reckey));
	if(reckey == NULL || rpoint == NULL) goto cleanup;

	EC_POINT_oct2point(EC_KEY_get0_group(reckey), rpoint,
			peerPubkeyChar, buflen, NULL);
	EC_KEY_set_public_key(reckey, rpoint);
	EVP_PKEY *peer_pub_key = EVP_PKEY_new();
	if(peer_pub_key == NULL) goto cleanup;
	EVP_PKEY_set1_EC_KEY(peer_pub_key, reckey);

	/*
	 * 5. Compute ECDH shared secret (done by A)
	 * */

	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL);
	if(ctx == NULL) goto cleanup;
	EVP_PKEY_derive_init(ctx);
	EVP_PKEY_derive_set_peer(ctx, peer_pub_key);

	// get buf length
	size_t secret_len = 0;
	EVP_PKEY_derive(ctx, NULL, &secret_len);

	unsigned char *secret = (unsigned char *) malloc (secret_len);
	if(secret == NULL) goto cleanup;

	EVP_PKEY_derive(ctx, secret, &secret_len);

	/*
	 * 6. Serialize A's public key (done by A) : EVP_PKEY => char[]
	 * */



	/*
	 * 7. De-serialize A's public key (done by B) : char[] => EVP_PKEY
	 * */

	/*
	 * 8. Compute ECDH shared secret (done by B)
	 * */




cleanup :
	if(pctx != NULL) EVP_PKEY_CTX_free(pctx);
	if(kctx != NULL) EVP_PKEY_CTX_free(kctx);
	if(pkey != NULL) EVP_PKEY_free(pkey);
	if(peerkey != NULL) EVP_PKEY_free(peerkey);

	if(peer_eckey != NULL) EC_KEY_free(peer_eckey);
	if(peer_pub_ecpoint != NULL) EC_POINT_free(peer_pub_ecpoint);
	if(peerPubkeyChar != NULL) free(peerPubkeyChar);
	if(reckey != NULL) EC_KEY_free(reckey);
	if(rpoint != NULL) EC_POINT_free(rpoint);
	if(peer_pub_key != NULL) EVP_PKEY_free(peer_pub_key);

	if(ctx != NULL) EVP_PKEY_CTX_free(ctx);
	if(secret != NULL) free(secret);

	return EXIT_SUCCESS;
}
