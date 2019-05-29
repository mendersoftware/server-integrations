#ifndef X509_IMPL_H_
#define X509_IMPL_H_

#include <stddef.h>
#include <stdbool.h>

typedef struct X509_CERT_INFO_TAG
{
    char* X509_CERTIFICATE;
    char* X509_PRIVATEKEY;
    char* X509_COMMON_NAME;
} X509_CERT_INFO;

typedef struct X509_IMPL_CERT_INFO_TAG* X509_IMPL_HANDLE;

extern int x509_initialize(const char *path);
extern void x509_deinitialize();

extern X509_IMPL_HANDLE x509_impl_create(void);
extern void x509_impl_destroy();

extern const char* x509_impl_retrieve_cert(X509_IMPL_HANDLE handle);
extern const char* x509_impl_retrieve_key_alias(X509_IMPL_HANDLE handle);
extern const char* x509_impl_cert_common_name(X509_IMPL_HANDLE handle);
extern const char* x509_impl_get_last_error(X509_IMPL_HANDLE handle);

#endif /* X509_IMPL_H_ */
