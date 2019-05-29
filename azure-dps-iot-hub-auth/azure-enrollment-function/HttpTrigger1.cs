#r "Newtonsoft.Json"

using System.Net;
using System.Net.Http.Headers;
using System.Text;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Primitives;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.Net.Http;

using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;


public static async Task<HttpResponseMessage> Run(HttpRequestMessage req, ILogger log)
{
    log.LogInformation("C# HTTP trigger function processed a request.");
    log.LogInformation("Request.Content:...");
    log.LogInformation(req.Content.ReadAsStringAsync().Result);

    string message = "Uncaught error";
    bool fail = false;
    ResponseObj obj = new ResponseObj();

    dynamic data = await req.Content.ReadAsAsync<object>();

    string regId = data?.deviceRuntimeContext?.registrationId;
    string[] hubs = data?.linkedHubs.ToObject<string[]>();
    string clientCertificate = data?.deviceRuntimeContext?.x509?.clientCertificate;

    object messageFormatted = null;

    if (regId == null)
    {
        message = "Registration ID not provided for the device.";
        log.LogInformation("Registration ID : NULL");
        fail = true;
    }
    else
    {
        if (hubs == null)
        {
            message = "No hub group defined for the enrollment.";
            log.LogInformation("linkedHubs : NULL");
            fail = true;
        }
        else
        {
            foreach(string hubString in hubs)
            {
                if (hubString.Contains("mender-azure-test-hub"))
                    obj.iotHubHostName = hubString;
            }

            if (obj.iotHubHostName == null)
            {
                log.LogInformation("No valid hub found for the enrollment.");
                fail = true;
            }

            if (clientCertificate == null)
            {
                log.LogInformation("No valid client certificate found for the enrollment.");
                fail = true;
            }

            byte[] clientCertBytes = Convert.FromBase64String(clientCertificate);

            X509Certificate2 clientCert = new X509Certificate2(clientCertBytes, "1234", X509KeyStorageFlags.DefaultKeySet | X509KeyStorageFlags.Exportable);

            if (clientCert == null)
            {
                log.LogInformation("Certificate is null");
                fail = true;
            }
            else
            {
                log.LogInformation("Certificate:");
                log.LogInformation(clientCert.ToString(true));

                log.LogInformation("PublicKey EncodedKeyValue Format(true):");
                log.LogInformation(clientCert.PublicKey.EncodedKeyValue.Format(true));

                byte[] constructedSeq1 = new byte[] { 0x30, 0x82 };

                string oid = clientCert.PublicKey.Oid.Value;
                byte[] publicKey = clientCert.PublicKey.EncodedKeyValue.RawData;
                byte[] parameters = clientCert.PublicKey.EncodedParameters.RawData;
                byte[] objectIdentifier = CryptoConfig.EncodeOID(oid);

                byte[] constructedSeq2 = { 0x30, Convert.ToByte(objectIdentifier.Length + parameters.Length) };

                byte[] constructedSeq3 = { 0x03, 0x82 };
                byte[] padding = { 0x00 };

                UInt16 key_and_padding_len = (UInt16)(publicKey.Length + padding.Length);
                byte[] keyAndPaddingLen = BitConverter.GetBytes(key_and_padding_len);
                Array.Reverse(keyAndPaddingLen);

                UInt16 totalLength = (UInt16)(constructedSeq2.Length + objectIdentifier.Length
                    + parameters.Length + constructedSeq3.Length + keyAndPaddingLen.Length
                    + padding.Length + publicKey.Length);

                byte[] totalLengthArray = BitConverter.GetBytes(totalLength);
                Array.Reverse(totalLengthArray);

                List<byte> rsaPublicKeyList = new List<byte>(constructedSeq1);
                rsaPublicKeyList.AddRange(totalLengthArray);
                rsaPublicKeyList.AddRange(constructedSeq2);
                rsaPublicKeyList.AddRange(objectIdentifier);
                rsaPublicKeyList.AddRange(parameters);
                rsaPublicKeyList.AddRange(constructedSeq3);
                rsaPublicKeyList.AddRange(keyAndPaddingLen);
                rsaPublicKeyList.AddRange(padding);
                rsaPublicKeyList.AddRange(publicKey);

                byte[] rsaPublicKeyBytes = rsaPublicKeyList.ToArray();
                string rsaPublicKeyString = Convert.ToBase64String(rsaPublicKeyBytes, Base64FormattingOptions.None);
                var sb = new StringBuilder(rsaPublicKeyString);

                for (int i = 64; i < sb.Length; i += 65)
                {
                    sb.Insert(i, "\n");
                }

                StringBuilder publicKeyBuilder = new StringBuilder();
                publicKeyBuilder.AppendLine("-----BEGIN PUBLIC KEY-----");
                publicKeyBuilder.Append(sb.ToString()).AppendLine();
                publicKeyBuilder.Append("-----END PUBLIC KEY-----");

                string rsaPublicKey = publicKeyBuilder.ToString();

                string credentials = "REPLACE_WITH_HOSTED_MENDER_CREDENTIALS";

                var loginUrl = new Uri($"https://hosted.mender.io/api/management/v1/useradm/auth/login");

                string JWT;

                using (var client = new HttpClient())
                {
                    client.DefaultRequestHeaders.Add("Authorization", $"Basic {credentials}");
                    var result = client.PostAsync($"{loginUrl}", null).Result;

                    JWT = result.Content.ReadAsStringAsync().Result.Trim('"'); //get  JWT for call funtion key

                    log.LogInformation("JWT Token:");
                    log.LogInformation(JWT);
                }

                log.LogInformation("Public key:");
                log.LogInformation(rsaPublicKey);

                var preAuthUrl = new Uri($"https://hosted.mender.io/api/management/v2/devauth/devices");

                dynamic body = new JObject();
                body.pubkey = rsaPublicKey;
                body.identity_data = new JObject();
                body.identity_data.DeviceId = "mender-azure-test-device1";

                using (var client = new HttpClient())
                using (var request = new HttpRequestMessage(HttpMethod.Post, preAuthUrl))
                using (var httpContent = CreateHttpContent(body))
                {
                    request.Content = httpContent;

                    client.DefaultRequestHeaders.Add("Authorization", "Bearer " + JWT);

                    var response = await client.SendAsync(request);

                    if (!response.IsSuccessStatusCode)
                    {
                        message = await response.Content.ReadAsStringAsync();
                        messageFormatted = JsonConvert.DeserializeObject(message);
                        log.LogInformation("Sending preauthorization request failed.");
                        fail = true;
                    }
                }
            }
        }
    }

    return (fail)
        ? new HttpResponseMessage(HttpStatusCode.BadRequest)
        {
            Content = new StringContent(JsonConvert.SerializeObject(messageFormatted, Formatting.Indented),
                Encoding.UTF8, "application/json")
        }
        : new HttpResponseMessage(HttpStatusCode.OK)
        {
            Content = new StringContent(JsonConvert.SerializeObject(obj, Formatting.Indented),
                Encoding.UTF8, "application/json")
        };
}

public static void SerializeJsonIntoStream(object value, Stream stream)
{
    using (var sw = new StreamWriter(stream, new UTF8Encoding(false), 1024, true))
    using (var jtw = new JsonTextWriter(sw) { Formatting = Formatting.None })
    {
        var js = new JsonSerializer();
        js.Serialize(jtw, value);
        jtw.Flush();
    }
}

public static HttpContent CreateHttpContent(object content)
{
    HttpContent httpContent = null;

    if (content != null)
    {
        var ms = new MemoryStream();
        SerializeJsonIntoStream(content, ms);
        ms.Seek(0, SeekOrigin.Begin);
        httpContent = new StreamContent(ms);
        httpContent.Headers.ContentType = new MediaTypeHeaderValue("application/json");
    }

    return httpContent;
}

public class DeviceTwinObj
{
    public string deviceId {get; set;}
}

public class ResponseObj
{
    public string iotHubHostName {get; set;}
    public string IoTHub {get; set;}
    public DeviceTwinObj initialTwin {get; set;}
    public string[] linkedHubs {get; set;}
    public string enrollment {get; set;}
}
