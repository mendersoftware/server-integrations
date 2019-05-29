//#include <cstdio>
#include <stdlib.h>
#include <errno.h>

#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pkcs12.h>
#include <openssl/bio.h>

#include <iothub.h>
#include <azure_c_shared_utility/shared_util_options.h>
#include <azure_c_shared_utility/http_proxy_io.h>
#include <azure_c_shared_utility/threadapi.h>

#include "azure_prov_client/prov_device_client.h"
#include "azure_prov_client/prov_security_factory.h"

#include "iothubtransportamqp.h"
#include "azure_prov_client/prov_transport_amqp_client.h"

static const char* global_prov_uri = "global.azure-devices-provisioning.net";

static bool g_registration_complete = false;

MU_DEFINE_ENUM_STRINGS(PROV_DEVICE_RESULT, PROV_DEVICE_RESULT_VALUE);
MU_DEFINE_ENUM_STRINGS(PROV_DEVICE_REG_STATUS, PROV_DEVICE_REG_STATUS_VALUES);

static void registation_status_callback(PROV_DEVICE_REG_STATUS reg_status, void* user_context)
{
    (void)user_context;
    (void)printf("Provisioning Status: %s\r\n", MU_ENUM_TO_STRING(PROV_DEVICE_REG_STATUS, reg_status));
}

static void register_device_callback(PROV_DEVICE_RESULT register_result,
                                     const char* iothub_uri, const char* device_id,
                                     void* user_context)
{
    (void)user_context;
    if (register_result == PROV_DEVICE_RESULT_OK)
    {
        (void)printf("\r\nRegistration Information received from service: %s, deviceId: %s\r\n", iothub_uri, device_id);
    }
    else
    {
        (void)printf("\r\nFailure registering device: %s\r\n", MU_ENUM_TO_STRING(PROV_DEVICE_RESULT, register_result));
    }
    g_registration_complete = true;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Insufficient number of arguments provided.Aborting.\n");
	exit(1);
    }

    const char *id_scope = argv[1]; //"0ne0003F2D9";

    (void)IoTHub_Init();

    const char globalDeviceEndpoint[] = "global.azure-devices-provisioning.net";

    SECURE_DEVICE_TYPE hsm_type = SECURE_DEVICE_TYPE_X509;
    (void)prov_dev_security_init(hsm_type);

    printf("Device Security Initialized\n");

    HTTP_PROXY_OPTIONS http_proxy;
    PROV_DEVICE_TRANSPORT_PROVIDER_FUNCTION prov_transport;

    memset(&http_proxy, 0, sizeof(HTTP_PROXY_OPTIONS));
    prov_transport = Prov_Device_AMQP_Protocol;

    printf("Provisioning API Version: %s\r\n", Prov_Device_GetVersionString());

    PROV_DEVICE_RESULT prov_device_result = PROV_DEVICE_RESULT_ERROR;
    PROV_DEVICE_HANDLE prov_device_handle;

    if ((prov_device_handle = Prov_Device_Create(global_prov_uri, id_scope, prov_transport)) == NULL) {
        (void)printf("failed calling Prov_Device_Create\r\n");
    }
    else {
        prov_device_result = Prov_Device_Register_Device(prov_device_handle, register_device_callback,
                                                         NULL, registation_status_callback, NULL);

        (void)printf("\r\nRegistering Device\r\n\r\n");
	do
	{
	    ThreadAPI_Sleep(1000);
	} while (!g_registration_complete);

	Prov_Device_Destroy(prov_device_handle);
    }

    prov_dev_security_deinit();
    (void)IoTHub_Deinit();

    (void)printf("Press enter key to exit:\r\n");
    (void)getchar();

    return 0;
}
