// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "x509_impl.h"
#include "hsm_client_data.h"

//static const char* const COMMON_NAME = "mender-azure-test-device";

//static const char* const CERTIFICATE = "-----BEGIN CERTIFICATE-----""\n"
//"MIIDfjCCAmagAwIBAgIQRqY2ZnQe07BOpDSBrJJe8zANBgkqhkiG9w0BAQsFADAoMSYwJAYDVQQDDB1BenVyZSBJb1QgQ0EgVGVzdE9ubHkgUm9vdCBDQTAeFw0xOTAxMDkxNTMwMTdaFw0xOTAyMDgxNTQwMTdaMCMxITAfBgNVBAMMGG1lbmRlci1henVyZS10ZXN0LWRldmljZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKl8W+ht9Fg/wZTumIPGjlRW4pgrn2WY0b7Iw3bYMlaVStqJznBmXNaiOZVlSmufv9P3W9v6HWjO/M8g55VHVxfbW0U7EVHdEV/6PQnDqHi5rkiMY2YtxbTMXrFYdhQfkNaG4gAtiNyI/I0TLz9jqkCaG39EtnF2qXUVach7ttxIrmspjSKxlpHbakoLtZmgTkz5UZIAK78WugOq1nE9DYzbwJEaKGETLQsoh99zutNxvhG5MJ7V/LIT4sSZe4Yd10K23Kevin6p7ZsUE8SDaCEQKP5YohplK4ONbG3yVAgpU4jkEJCQAqXpNvswjV2lx2ECGc+Hog5k2yDgu9mVLUECAwEAAaOBqDCBpTAOBgNVHQ8BAf8EBAMCBaAwIwYDVR0RBBwwGoIYbWVuZGVyLWF6dXJlLXRlc3QtZGV2aWNlMB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDATAPBgNVHRMBAf8EBTADAgEAMB8GA1UdIwQYMBaAFN3NYamoz6WTC3WSVSBPaK+SdSWoMB0GA1UdDgQWBBR5e0MtGi/1oL4K+8KQq4LKsGSlbTANBgkqhkiG9w0BAQsFAAOCAQEAU+CVBcbtVstTY1xG9u/Zx291S7Zknn3Wyqpn92ydPu3i4kE3JFuf06Hn/rcBP0kY1LMy4Uw3PoCsekaCgjVdTzbuURM2MHA4oXHOCEInDZBwiMZzCaWZs9u18+eut0wi3gFMYWfpupuLqHQ4dfM/W3jZlVQC3DFG4wbLyNtbZ1som/m5NEgVL9OcHAIWSEpak+Kyuz70W0v8rT7VITart5eZor7HpB1k3N9thFf4ggRR2YCHKk92IkrIDIW/WQ8yby9qtODsFuSx8FeUqalDl90a34yewh7DouD2IlEqiaX3+1ZUkNgxjMrVnvMlTWpGOW0s4Xic9Jb6QhSD2QAGKA==""\n"
//"-----END CERTIFICATE-----";

//static const char* const PRIVATE_KEY = "-----BEGIN RSA PRIVATE KEY-----""\n"
//"MIIEogIBAAKCAQEAqXxb6G30WD/BlO6Yg8aOVFbimCufZZjRvsjDdtgyVpVK2onOcGZc1qI5lWVKa5+/0/db2/odaM78zyDnlUdXF9tbRTsRUd0RX/o9CcOoeLmuSIxjZi3FtMxesVh2FB+Q1obiAC2I3Ij8jRMvP2OqQJobf0S2cXapdRVpyHu23EiuaymNIrGWkdtqSgu1maBOTPlRkgArvxa6A6rWcT0NjNvAkRooYRMtCyiH33O603G+EbkwntX8shPixJl7hh3XQrbcp6+KfqntmxQTxINoIRAo/liiGmUrg41sbfJUCClTiOQQkJACpek2+zCNXaXHYQIZz4eiDmTbIOC72ZUtQQIDAQABAoIBADMRYJQZRW2t0ENy3QufsVUiVOZrBn1TXaex0IunuwsxsrtEmKpvfXvCp70nJxRkryVMPCZ9J41kCPLURFNLiqeKlea3VJGpHcogmXGuvb2anTQI8Q5hrle62LFxKSsJE9hADirljX593DMZ27uq6SrMBIsFhj+L9KVtzVD2TfPziOM0zvO6AV1hjBcXTJ41ITyXbHfufIj3FGjyTFaDM9BjfAPVn8s/SasHHOBQIpeWi7o3rJQH3GXAgrSIWmsegsnnZ72cQBY4RUDHV6gz3t5gOXvDraNXRQpeg9GqnTas7StLEVQooi4z6RPRru5eCEgcPDUEY1059MznQMZCf/0CgYEA1RwOHZrrYNxCNfWeq75BbS807RoENbuyFzu5Ef3NVcbU7XTBzrVoNXhYXfoYPRMV/Z7e9dam9oWxMd9ZFvpoNgu+kbLqtedhH/v4DuzlkaaIaVY7OQ6lZXjkUKeRzpcuRKaXmgqOy4CAu3MC/W3DmlkdpjpzVmMupWsRUDYMBuMCgYEAy5ix2tbhkAmX45GcZlXiNjp/8Rj1bx32GZh//d7z6FDn87/7OMUGPBWILQPFUUnaTVt8c2L31LHggCnN69uYM+JMrhdymo7aaJ5PmNUHpiVKcZnC+zOj+U5TT3JxSQZ+HPkWGM3GHwJg1IUzCUrW4Dehz/yGNIvIatn3U+vH0IsCgYBIr6382QcpXVbT8rew2k6UgAJsUHBZJ3DBzeP8x/VOKWgcqbMk3ujxO9/gBdpYknbwr0xay2DdOpJKcWSAmt4KeQ0BwuWdL5MIBrf/1zsh54wlaXz/pAgzyP0OqWjo7tWgIrYYmztDG8xMoxjE+bg6n+Sl+s+2ClQxpYgdNKH7gwKBgFb2dNmNuTEF9/u7IXvQqVwSZr27tHHu8USDkAAHMFWF4V2LHipkdcJKTC02FB0hl349Wo4KsuMFy2fS82MQHBXlTm16FHiSFhZjsIoOTEMaYI7fZ5pWt25wl0MzFdJkVI/ZkSetFTf/kw6TinYoolmJSbOGIGjnYAbL7UnKR0H7AoGAR/zedrRS6Hjc/nUyrUWRzO+OIlPkvUOyyNodkV4jiJa0CGXh5R+sBKyTcYZ1pDPnHs71+C+Tnw6t0jQRi45Vz/4VgDpFkT75jC/hG4GwMR9JlXPxfRVplz2LHYhogjzvo82nIPohd56mIWaY8M39xriO2wRYm8r7fhoODJhyNYw=""\n"
//"-----END RSA PRIVATE KEY-----";

// Provided for sample only
static const char* const SYMMETRIC_KEY = "Symmetric Key value";
static const char* const REGISTRATION_NAME = "Registration Name";

static const char* const MENDER_CERT_PATH = "/data/mender/mender-certificate.pfx";

// Provided for sample only, canned values
static const unsigned char EK[] = { 0x45, 0x6e, 0x64, 0x6f, 0x72, 0x73, 0x65, 0x6d, 0x65, 0x6e, 0x74, 0x20, 0x6b, 0x65, 0x79, 0x0d, 0x0a };
static const size_t EK_LEN = sizeof(EK)/sizeof(EK[0]);
static const unsigned char SRK[] = { 0x53, 0x74, 0x6f, 0x72, 0x65, 0x20, 0x72, 0x6f, 0x6f, 0x74, 0x20, 0x6b, 0x65, 0x79, 0x0d, 0x0a };
static const size_t SRK_LEN = sizeof(SRK) / sizeof(SRK[0]);
static const unsigned char ENCRYPTED_DATA[] = { 0x45, 0x6e, 0x63, 0x72, 0x79, 0x70, 0x74, 0x65, 0x64, 0x20, 0x64, 0x61, 0x74, 0x61, 0x0d, 0x0a };

typedef struct CUSTOM_HSM_SAMPLE_INFO_TAG
{
    X509_IMPL_HANDLE x509_impl;
    const unsigned char* endorsment_key;
    size_t ek_length;
    const unsigned char* storage_root_key;
    size_t srk_len;
    const char* symm_key;
    const char* registration_name;
} CUSTOM_HSM_SAMPLE_INFO;

int hsm_client_x509_init()
{
    x509_initialize(MENDER_CERT_PATH);
    return 0;
}

void hsm_client_x509_deinit()
{
}

int hsm_client_tpm_init()
{
    return 0;
}

void hsm_client_tpm_deinit()
{
}

HSM_CLIENT_HANDLE custom_hsm_create()
{
    HSM_CLIENT_HANDLE result;
    CUSTOM_HSM_SAMPLE_INFO* hsm_info = malloc(sizeof(CUSTOM_HSM_SAMPLE_INFO));
    if (hsm_info == NULL)
    {
        (void)printf("Failed allocating hsm info\r\n");
        result = NULL;
    }
    else
    {
        hsm_info->x509_impl = x509_impl_create();
        if (hsm_info->x509_impl == NULL)
        {
            (void)printf("Failure: x509 openssl create failed.");
            free(hsm_info);
            hsm_info = NULL;
        }
        result = hsm_info;

        hsm_info->ek_length = EK_LEN;
        hsm_info->srk_len = SRK_LEN;
        hsm_info->endorsment_key = EK;
        hsm_info->storage_root_key = SRK;
        hsm_info->registration_name = REGISTRATION_NAME;
        hsm_info->symm_key = SYMMETRIC_KEY;
    }
    return result;
}

void custom_hsm_destroy(HSM_CLIENT_HANDLE handle)
{
    if (handle != NULL)
    {
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        // Free anything that has been allocated in this module
        x509_impl_destroy(hsm_info->x509_impl);
    }
}

char* custom_hsm_get_certificate(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified\r\n");
        result = NULL;
    }
    else
    {
        // TODO: Malloc the certificate for the iothub sdk to free
        // this value will be sent unmodified to the tlsio
        // layer to be processed
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        const char* certificate = x509_impl_retrieve_cert(hsm_info->x509_impl);

        if (certificate == NULL)
        {
            (void)printf("Failure allocating certificate\r\n");
            result = NULL;
        }
        else
        {
            size_t l = strlen(certificate);
            result = (char*)malloc(l + 1);
            memcpy(result, certificate, l + 1);
        }
    }
    return result;
}

char* custom_hsm_get_key(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified\r\n");
        result = NULL;
    }
    else
    {
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        const char* private_key = x509_impl_retrieve_key_alias(hsm_info->x509_impl);

        if (private_key == NULL)
        {
            (void)printf("Failure allocating private key\r\n");
            result = NULL;
        }
        else
        {
            size_t l = strlen(private_key);
            result = (char*)malloc(l + 1);
            memcpy(result, private_key, l + 1);
        }
    }
    return result;
}

char* custom_hsm_get_common_name(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified\r\n");
        result = NULL;
    }
    else
    {
        // TODO: Malloc the common name for the iothub sdk to free
        // this value will be sent to dps
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        const char* cn = x509_impl_cert_common_name(hsm_info->x509_impl);

        if (cn == NULL)
        {
            (void)printf("Failure allocating common name\r\n");
            result = NULL;
        }
        else
        {
            size_t l = strlen(cn);
            result = (char*)malloc(l + 1);
            memcpy(result, cn, l + 1);
        }
    }
    return result;
}

int custom_hsm_get_endorsement_key(HSM_CLIENT_HANDLE handle, unsigned char** key, size_t* key_len)
{
    int result;
    if (handle == NULL || key == NULL || key_len == NULL)
    {
        (void)printf("Invalid parameter specified");
        result = __LINE__;
    }
    else
    {
        // TODO: Retrieve the endorsement key and malloc the value and return
        // it to the sdk
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        if ((*key = (unsigned char*)malloc(hsm_info->ek_length)) == NULL)
        {
            (void)printf("Failure allocating endorsement key\r\n");
            result = __LINE__;
        }
        else
        {
            memcpy(*key, hsm_info->endorsment_key, hsm_info->ek_length);
            *key_len = hsm_info->ek_length;
            result = 0;
        }
    }
    return result;
}

int custom_hsm_get_storage_root_key(HSM_CLIENT_HANDLE handle, unsigned char** key, size_t* key_len)
{
    int result;
    if (handle == NULL || key == NULL || key_len == NULL)
    {
        (void)printf("Invalid handle value specified");
        result = __LINE__;
    }
    else
    {
        // TODO: Retrieve the storage root key and malloc the value and return
        // it to the sdk
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        if ((*key = (unsigned char*)malloc(hsm_info->srk_len)) == NULL)
        {
            (void)printf("Failure allocating storage root key\r\n");
            result = __LINE__;
        }
        else
        {
            memcpy(*key, hsm_info->storage_root_key, hsm_info->srk_len);
            *key_len = hsm_info->srk_len;
            result = 0;
        }
    }
    return result;
}

int custom_hsm_sign_with_identity(HSM_CLIENT_HANDLE handle, const unsigned char* data, size_t data_len, unsigned char** key, size_t* key_len)
{
    int result;
    if (handle == NULL || data == NULL || key == NULL || key_len == NULL)
    {
        (void)printf("Invalid handle value specified");
        result = __LINE__;
    }
    else
    {
        (void)data;
        (void)data_len;

        // TODO: Need to implement signing the data variable and malloc the key and return it to the sdk

        size_t signed_data_len = 10;
        if ((*key = (unsigned char*)malloc(signed_data_len)) == NULL)
        {
            (void)printf("Failure allocating storage root key\r\n");
            result = __LINE__;
        }
        else
        {
            memcpy(*key, ENCRYPTED_DATA, signed_data_len);
            *key_len = signed_data_len;
            result = 0;
        }
    }
    return result;
}

int custom_hsm_activate_identity_key(HSM_CLIENT_HANDLE handle, const unsigned char* key, size_t key_len)
{
    int result;
    if (handle == NULL || key == NULL || key_len == 0)
    {
        (void)printf("Invalid handle value specified");
        result = __LINE__;
    }
    else
    {
        // Decrypt the key and store the value in the hsm
        result = 0;
    }
    return result;
}

char* custom_hsm_symm_key(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified\r\n");
        result = NULL;
    }
    else
    {
        // TODO: Malloc the symmetric key for the iothub 
        // The SDK will call free() this value
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        size_t len = strlen(hsm_info->symm_key);
        if ((result = (char*)malloc(len + 1)) == NULL)
        {
            (void)printf("Failure allocating certificate\r\n");
            result = NULL;
        }
        else
        {
            strcpy(result, hsm_info->symm_key);
        }
    }
    return result;
}

char* custom_hsm_get_registration_name(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified\r\n");
        result = NULL;
    }
    else
    {
        // TODO: Malloc the registration name for the iothub 
        // The SDK will call free() this value
        CUSTOM_HSM_SAMPLE_INFO* hsm_info = (CUSTOM_HSM_SAMPLE_INFO*)handle;
        size_t len = strlen(hsm_info->registration_name);
        if ((result = (char*)malloc(len + 1)) == NULL)
        {
            (void)printf("Failure allocating certificate\r\n");
            result = NULL;
        }
        else
        {
            strcpy(result, hsm_info->registration_name);
        }
    }
    return result;
}

// Defining the v-table for the x509 hsm calls
static const HSM_CLIENT_X509_INTERFACE x509_interface =
{
    custom_hsm_create,
    custom_hsm_destroy,
    custom_hsm_get_certificate,
    custom_hsm_get_key,
    custom_hsm_get_common_name
};

// Defining the v-table for the x509 hsm calls
static const HSM_CLIENT_TPM_INTERFACE tpm_interface =
{
    custom_hsm_create,
    custom_hsm_destroy,
    custom_hsm_activate_identity_key,
    custom_hsm_get_endorsement_key,
    custom_hsm_get_storage_root_key,
    custom_hsm_sign_with_identity
};

static const HSM_CLIENT_KEY_INTERFACE symm_key_interface =
{
    custom_hsm_create,
    custom_hsm_destroy,
    custom_hsm_symm_key,
    custom_hsm_get_registration_name
};

const HSM_CLIENT_TPM_INTERFACE* hsm_client_tpm_interface()
{
    // tpm interface pointer
    return &tpm_interface;
}

const HSM_CLIENT_X509_INTERFACE* hsm_client_x509_interface()
{
    // x509 interface pointer
    return &x509_interface;
}

const HSM_CLIENT_KEY_INTERFACE* hsm_client_key_interface()
{
    return &symm_key_interface;
}
