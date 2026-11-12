#ifndef SMART_HOME_SECRETS_H
#define SMART_HOME_SECRETS_H

// Copy this file to `secrets.h` in the same directory and fill in your own values.
// `secrets.h` is ignored by git and must never be committed.

const char* WSSID = "YOUR_WIFI_SSID";
const char* WPASS = "YOUR_WIFI_PASSWORD";

// Huawei Cloud IoTDA device credentials
String DID  = "YOUR_DEVICE_ID";
String DSEC = "YOUR_DEVICE_SECRET";
#define MHOST "YOUR_IOTDA_MQTT_ENDPOINT"
#define MPORT 8883

// Optional: ServerChan (Server酱) SendKey for WeChat notifications.
// Leave as the placeholder to disable notifications.
#define SCT_KEY "YOUR_SERVERCHAN_SENDKEY"

#endif
