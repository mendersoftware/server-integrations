#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pkcs12.h>
#include <openssl/bio.h>

#include "x509_impl.h"

typedef struct X509_IMPL_CERT_INFO_TAG
{
    char* X509_CERTIFICATE;
    char* X509_PRIVATEKEY;
    char* X509_COMMON_NAME;
} X509_IMPL_CERT_INFO;

static X509_CERT_INFO *x509_cert_info;

char **strsplit(const char* str, size_t str_size, const char* delim, size_t* numtokens) {
    // copy the original string so that we don't overwrite parts of it
    char *s = (char*)malloc(str_size + 1);
    memcpy(s, str, str_size);
    s[str_size] = '\0';

    size_t tokens_alloc = 1;
    size_t tokens_used = 0;
    char **tokens = calloc(tokens_alloc, sizeof(char*));
    char *token, *strtok_ctx;

    for (token = strtok_r(s, delim, &strtok_ctx);
            token != NULL;
            token = strtok_r(NULL, delim, &strtok_ctx)) {
        // check if we need to allocate more space for tokens
        if (tokens_used == tokens_alloc) {
            tokens_alloc *= 2;
            tokens = realloc(tokens, tokens_alloc * sizeof(char*));
        }
        tokens[tokens_used++] = strdup(token);
    }
    // cleanup
    if (tokens_used == 0) {
        free(tokens);
        tokens = NULL;
    } else {
        tokens = realloc(tokens, tokens_used * sizeof(char*));
    }
    *numtokens = tokens_used;
    free(s);
    return tokens;
}

void normalize_x509_item(char **tokens, size_t count, char **x509_item) {
    size_t total_length = 0;
    for (size_t i = 0; i < count; i++) {
        total_length += strlen(tokens[i]);
    }

    // add space for 2 newline characters and terminating null
    total_length += 2 * strlen("\n") + 1;

    *x509_item = NULL;
    *x509_item = (char*)malloc(sizeof(char) * total_length);

    if (*x509_item == NULL) {
        free(*x509_item);
        printf("Memory allocation did not complete successfully!\n");
        return;
    }

    strcpy(*x509_item, tokens[0]);
    strcat(*x509_item, "\n");

    for (size_t i = 1; i < count - 1; i++) {
        strcat(*x509_item, tokens[i]);
    }
    strcat(*x509_item, "\n");
    strcat(*x509_item, tokens[count - 1]);
    (*x509_item)[total_length - 1] = '\0';
}

X509_IMPL_HANDLE x509_impl_create(void)
{
    X509_IMPL_CERT_INFO* handle;
    if ((handle = (X509_IMPL_CERT_INFO*)malloc(sizeof(X509_IMPL_CERT_INFO))) == NULL) {
        printf("Failure: malloc X509_INFO.");
    }
    else
    {
        memset(handle, 0, sizeof(X509_IMPL_CERT_INFO));

        size_t l = strlen(x509_cert_info->X509_CERTIFICATE);
        char* temp = (char*)malloc(l + 1);
        handle->X509_CERTIFICATE = temp;
        memcpy(handle->X509_CERTIFICATE, x509_cert_info->X509_CERTIFICATE, l + 1);

        l = strlen(x509_cert_info->X509_PRIVATEKEY);
        temp = (char*)malloc(l + 1);
        handle->X509_PRIVATEKEY = temp;
        memcpy(handle->X509_PRIVATEKEY, x509_cert_info->X509_PRIVATEKEY, l + 1);

        l = strlen(x509_cert_info->X509_COMMON_NAME);
        temp = (char*)malloc(l + 1);
        handle->X509_COMMON_NAME = temp;
        memcpy(handle->X509_COMMON_NAME, x509_cert_info->X509_COMMON_NAME, l + 1);

        if (handle->X509_CERTIFICATE == NULL || handle->X509_PRIVATEKEY == NULL || handle->X509_COMMON_NAME == NULL) {
            free(handle->X509_CERTIFICATE);
            free(handle->X509_PRIVATEKEY);
            free(handle->X509_COMMON_NAME);
            free(handle);
            handle = NULL;
        }
    }
    return handle;
}

void x509_impl_destroy(X509_IMPL_HANDLE handle)
{
    if (handle != NULL)
    {
        free(handle->X509_CERTIFICATE);
        free(handle->X509_PRIVATEKEY);
        free(handle->X509_COMMON_NAME);
        free(handle);
    }
}

int x509_initialize(const char *path)
{
    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_digests();

    if ((x509_cert_info = (X509_CERT_INFO*)malloc(sizeof(X509_CERT_INFO))) == NULL)
    {
        printf("Failure: malloc X509_CERT_INFO.");
        return 1;
    }

    memset(x509_cert_info, 0, sizeof(X509_CERT_INFO));

    FILE *fp;
    EVP_PKEY *pkey;
    X509 *cert;
    RSA *rsa_pkey;
    STACK_OF(X509) *ca = NULL;
    PKCS12 *p12;

    ERR_load_CRYPTO_strings();

    if (!(fp = fopen(path, "rb"))) {
        fprintf(stderr, "Error opening PFX file: %d\n", errno);
        return 1;
    }

    p12 = d2i_PKCS12_fp(fp, NULL);
    fclose(fp);

    if (!p12) {
        fprintf(stderr, "Error reading PKCS#12 file\n");
        ERR_print_errors_fp(stderr);
        return 1;
    }
    if (!PKCS12_parse(p12, "1234", &pkey, &cert, &ca)) {
        fprintf(stderr, "Error parsing PKCS#12 file\n");
        ERR_print_errors_fp(stderr);
        return 1;
    }
    PKCS12_free(p12);

    BIO *bio_mem = BIO_new(BIO_s_mem());

    if (!PEM_write_bio_X509(bio_mem, cert)) {
        fprintf(stderr, "Error writing a certificate to a BIO\n");
        return 1;
    }

    BUF_MEM *mem = NULL;
    BIO_get_mem_ptr(bio_mem, &mem);

    //split certificate to tokens
    char **tokens;
    size_t numtokens;

    tokens = strsplit(mem->data, mem->length, "\n", &numtokens);

    normalize_x509_item(tokens, numtokens, &x509_cert_info->X509_CERTIFICATE);

    printf("Certificate:\n%s\n", x509_cert_info->X509_CERTIFICATE);

    for (size_t i = 0; i < numtokens; i++) {
        free(tokens[i]);
    }
    if (tokens != NULL)
        free(tokens);

    BIO_reset(bio_mem);

    rsa_pkey = EVP_PKEY_get1_RSA(pkey);

    if (!PEM_write_bio_RSAPrivateKey(bio_mem, rsa_pkey, NULL, NULL, 0, NULL, NULL)) {
        fprintf(stderr, "Error writing rsa private key to a BIO\n");
        return 1;
    }

    BIO_get_mem_ptr(bio_mem, &mem);
    tokens = strsplit(mem->data, mem->length, "\n", &numtokens);

    normalize_x509_item(tokens, numtokens, &x509_cert_info->X509_PRIVATEKEY);

    printf("RSA Private Key:\n%s\n", x509_cert_info->X509_PRIVATEKEY);

    for (size_t i = 0; i < numtokens; i++) {
        free(tokens[i]);
    }
    if (tokens != NULL)
        free(tokens);

    BIO_reset(bio_mem);
    if (!PEM_write_bio_RSAPublicKey(bio_mem, rsa_pkey)) {
        fprintf(stderr, "Error writing rsa public key to a BIO\n");
        return 1;
    }

    size_t pub_key_len = BIO_pending(bio_mem);
    char *pub_key = malloc(pub_key_len + 1);
    BIO_read(bio_mem, pub_key, pub_key_len);
    pub_key[pub_key_len] = '\0';
    fprintf(stdout, "RSA Public Key:\n%s\n", pub_key);
    free(pub_key);

    BIO_reset(bio_mem);
    if (!PEM_write_bio_PUBKEY(bio_mem, pkey)) {
        fprintf(stderr, "Error writing a public key to a BIO\n");
        return 1;
    }
    BIO_get_mem_ptr(bio_mem, &mem);
    pub_key_len = mem->length;
    pub_key = malloc(pub_key_len + 1);
    memcpy(pub_key, mem->data, pub_key_len);
    pub_key[pub_key_len] = '\0';
    fprintf(stdout, "Public Key:\n%s\n", pub_key);
    free(pub_key);

    EVP_PKEY *public_key = NULL;
    if ((public_key = X509_get_pubkey(cert)) == NULL) {
        fprintf(stderr, "Error extracting a public key\n");
	return 1;
    }
    BIO *outbio = BIO_new_fp(stdout, BIO_NOCLOSE);
    if(!PEM_write_bio_PUBKEY(outbio, public_key)) {
	fprintf(stderr, "Error writing public key data in PEM format");
	return 1;
    }
    EVP_PKEY_free(public_key);
    BIO_free_all(outbio);

    X509_NAME *subjectName = NULL;
    char subjectCn[256] = {0};
    subjectName = X509_get_subject_name(cert);
    X509_NAME_get_text_by_NID(subjectName, NID_commonName, subjectCn, sizeof(subjectCn));

    size_t length = strlen(subjectCn) + 1;
    x509_cert_info->X509_COMMON_NAME = (char*)malloc(length);
    memcpy(x509_cert_info->X509_COMMON_NAME, subjectCn, length);

    printf("Common Name: %s\n", x509_cert_info->X509_COMMON_NAME);

    sk_X509_pop_free(ca, X509_free);
    X509_free(cert);
    EVP_PKEY_free(pkey);
    BIO_free_all(bio_mem);

    return 0;
}

void x509_deinitialize()
{
    if (x509_cert_info != NULL) {
        free(x509_cert_info->X509_CERTIFICATE);
        free(x509_cert_info->X509_PRIVATEKEY);
        free(x509_cert_info->X509_COMMON_NAME);
        free(x509_cert_info);
    }
}

const char* x509_impl_retrieve_cert(X509_IMPL_HANDLE handle)
{
    const char* result;
    if (handle == NULL)
    {
        result = NULL;
    }
    else
    {
        result = handle->X509_CERTIFICATE;
    }
    return result;
}

const char* x509_impl_retrieve_key_alias(X509_IMPL_HANDLE handle)
{
    const char* result;
    if (handle == NULL)
    {
        result = NULL;
    }
    else
    {
        result = handle->X509_PRIVATEKEY;
    }
    return result;
}

const char* x509_impl_cert_common_name(X509_IMPL_HANDLE handle)
{
    const char* result;
    if (handle == NULL)
    {
        result = NULL;
    }
    else
    {
        result = handle->X509_COMMON_NAME;
    }
    return result;
}

const char* x509_impl_get_last_error(X509_IMPL_HANDLE handle)
{
    const char* result;
    if (handle == NULL)
    {
        result = "invalid_parameter";
    }
    else
    {
        result = "error";
    }
    return result;
}
