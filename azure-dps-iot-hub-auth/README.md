## Azure DPS and IoT Hub authentication integration

This integration automates authentication of devices in Hosted Mender.
When devices are provisioned into Azure Device Provisioning Service, they are also automatically preauthorized in Hosted Mender
so the Mender client can immediately start deploying software updates to provisioned devices.

See the diagram below for the flow provided by this integration.

![Mender Azure Integration Flowchart](images/mender-azure-integration-flowchart.png)


## Prerequisites

#### Clone the integration

```bash
git clone git@github.com:mendersoftware/mender-server-integrations.git
```

#### Install dependencies on your laptop or workstation

Install the latest version of these build dependencies:

* [git](https://git-scm.com/download/)
* [CMake](https://cmake.org/download/)
* [OpenSSL](https://www.openssl.org/)

#### Sign up for a Hosted Mender account

[Sign up for Hosted Mender](https://mender.io/signup).

#### Sign up for an Azure account

[Sign up for Azure](https://azure.microsoft.com/en-us/free/) and log in to the [Azure portal](https://portal.azure.com).


## Create an Azure IoT Hub and Device Provisioning Service

Follow [these instructions](https://docs.microsoft.com/en-us/azure/iot-dps/quick-setup-auto-provision), while applying the adjustments below:


#### Create an Azure IoT Hub 

An Azure IoT Hub helps to connect, monitor and manage your IoT assets.
Use this configuration for the new IoT Hub:

* Resource group: `mender-azure-test-hub-group`
* IoT Hub Name: `mender-azure-test-hub`

Click Next: *Size and Scale* to continue. Use these settings:
* Pricing and scale tier: `F1 for Free`
* Number of free Messages per day: reduced to `8000 per day` (from 400 000 for S1: Standard Tier)
* Device-to-cloud-partitions: reduced to `2` (from 4)

Click *Review + Create* to continue, then click *Create*.

It can take up to 10 minutes to complete creation.


#### Create an Azure Device Provisioning Service

An Azure Device Provisioning Service (DPS) provides seamless, zero-touch registration of devices to an IoT Hub with security that begins at the device and ends with Microsoft cloud.

Use this configuration for the DPS:

* Name: `mender-test-provisioning-service`
* Rest of the fields: use the automatic defaults

Click *Create* to create your new Device Provisioning Service.

Now you need to link your IoT hub and Device Provisioning Service.
Select *Linked IoT hubs* and click the *+ Add* button

* IoT hub: `mender-azure-test-hub`
* Access Policy: `iothubowner`
* Rest of the fields: use the automatic defaults


## Generate and add an X.509 root certificate to your IoT Hub

In this section and the following we will generate the required certificates for
authenticating with Azure. To keep the steps to a minimum we will
only consider individual enrollment in the IoT Hub, but this can be extended.

The X.509 certificate-based security in Azure DPS requires you to start with an X.509
certificate chain, which includes the root certificate as well as any intermediate
certificates up until the leaf certificate. 

The steps below are for Unix/bash, refer to
[Generating test CA certificates](https://github.com/Azure/azure-iot-sdk-c/blob/master/tools/CACertificates/CACertificateOverview.md)
for Powershell instructions and a more in-depth explanation.

First we need to download and run the certificate generation scripts. Run the following commands:

```bash
mkdir ~/certs
cd ~/certs
git clone https://github.com/Azure/azure-iot-sdk-c.git
cd azure-iot-sdk-c/tools/CACertificates
chmod +x certGen.sh
./certGen.sh create_root_and_intermediate
```

Now you should have the required certificates. Please **keep this terminal open** as we will return to it later.

Open the *Azure IoT Hub* console in your web browser. Then click on the IoT Hub you created above (mender-azure-test-hub),
followed by *Certificates* and *Add*. Fill in the following:
* Name: `mender-test-root`
* Browse to upload the certificate you generated at `~/certs/azure-iot-sdk-c/tools/CACertificates/certs/azure-iot-test-only.root.ca.cert.pem`.

Then click *Save*.

The certificate should now have status *Unverified*, so we will verify it next.

Click on the certificate, then find *Certificate Details*. Click on *Generate Verification Code*.
Copy this code (similar to 106A5SD242AF512B3498BD6098C4941E66R34H268DDB3288) to the clipboard.
Please **keep this browser tab open** as we will return to it soon.

Now go back to the terminal from above (you should be standing in the `CACertificates` directory) and run the following commands:

```bash
VERIFICATION_CODE= #PASTE IT INTO THE TERMINAL
```

```bash
./certGen.sh create_verification_certificate $VERIFICATION_CODE
```

Again, please **keep this terminal open** as we will return to it later.

Go back to the Certificate Details in the *Azure IoT Hub web console*, and find *Verification code*. Click *Browse*.
Browse to the verification certificate at `~/certs/azure-iot-sdk-c/tools/CACertificates/certs/verification-code.cert.pem`.
Then click *Verify*. Your root certificate should now show as Verified.


## Generate an X.509 device certificate

Your devices will authenticate using certificates signed by the root certificate we just generated and verified above.

First, return to the terminal from above and run the following commands:

```bash
./certGen.sh create_device_certificate mender-azure-test-device1
cat certs/new-device.cert.pem certs/azure-iot-test-only.intermediate.cert.pem certs/azure-iot-test-only.root.ca.cert.pem > certs/new-device-full-chain.cert.pem
```

We will use the generated files later when enrolling the device.


## Encode your Hosted Mender credentials into the Azure function HttpTrigger1.cs

In order for our custom Azure function to preauthorize devices to Hosted Mender,
we first need to encode the Hosted Mender credentials it should use and then add them to the function.

Note that encoding the Hosted Mender credentials in the function itself can be insecure if other users
have access to your Azure function. In this case the function should be adjusted to obtain the credentials
from a secure system, but this would complicate the design for the purpose of this tutorial as the function
would have to authenticate itself as well.

First [sign up for Hosted Mender](https://mender.io/signup) if you have not done so already.

Open a terminal and run the following commands (assuming a Unix shell is used) to encode your credentials
into a shell variable:

```bash
cd # or change to a temporary working directory
HOSTED_MENDER_EMAIL=# INSERT YOUR ACCOUNT EMAIL
HOSTED_MENDER_PASSWORD= #INSERT YOUR ACCOUNT PASSWORD
CREDENTIALS_ENCODED=$(echo -n “$HOSTED_MENDER_EMAIL:$HOSTED_MENDER_PASSWORD” | base64)
```

Now we will add these credentials in the right place in the `HttpTrigger1.cs` function.
It is provided as an asset together with this tutorial, so you might need to adjust the first command
to the correct path to the `mender-server-integrations` repository.

```bash
cp ~/mender-server-integrations/azure-dps-iot-hub-auth/azure-enrollment-function/HttpTrigger1.cs .
sed -i “s/REPLACE_WITH_HOSTED_MENDER_CREDENTIALS/$CREDENTIALS_ENCODED/g” HttpTrigger1.cs
```

Keep the resulting file `HttpTrigger1.cs`, as we will attach it to a device enrollment entry below
so that it can preauthorize devices to Hosted Mender as they are enrolled into Azure.


## Create a custom device enrollment entry

Next we will configure Azure DPS to authenticate our device which will be using the generated certificate and
run the Azure function that preauthorizes the device in Hosted Mender.

Go to the *Azure portal*, click on *All Resources* in the left-hand menu and open your *Device Provisioning Service*.
Select the *Manage Enrollments* tab and click *Add Individual Enrollment*. Fill in these fields:

* Primary Certificate: Browse to `~/certs/azure-iot-sdk-c/tools/CACertificates/certs/new-device-cert.pem`
* How You Want to Assign Devices to Hubs: `Custom (Use Azure Function)`
* Make sure `mender-azure-test-hub.azure-devices.net` is listed in the IoT Hubs List and that enrollment can be assigned to it.

In the same dialog, scroll down to the *Select Azure Function section* and click *Create a new function app* and input the following:
* App Name: `mender-test-function-app-x509`
* Resource group: Use existing; Select `mender-azure-test-hub-group`
* Runtime stack: `.NET`
* Click *Create*.

Go back to the *Add Enrollment* page and **make sure your new function app is selected**;
you may have to re-select the subscription to refresh the function app list. Then click *Create a new function* and fill in the following:

* On your function app, click *New function*
* Chose a development environment: `In-portal`
* Create a function: `Webhook + API`
* Click *Create* to create your new HttpTrigger1 C# function.

**Replace default C# code** with the HttpTrigger1.cs function from above (with your encoded credentials). Click *Save*.

Go back to the *Add Enrollment* page again and **make sure your new function is selected**;
you may have to re-select the Function App to refresh the functions list.

Click *Save* to complete the enrollment.
