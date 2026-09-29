#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ESP_Mail_Client.h>
#include <SoftwareSerial.h>

// ================= WIFI =================
#define WIFI_SSID     "M12"
#define WIFI_PASSWORD "11111110"

// ================= SOFTWARE SERIAL (ARDUINO DATA) =================
#define ARD_RX D5   // GPIO14
#define ARD_TX D6   // GPIO12 (not used, but required)
SoftwareSerial arduinoSerial(ARD_RX, ARD_TX);

// ================= THINGSPEAK =================
#define TS_API_KEY "Q833BNG7S4CI3YN4"
#define TS_SERVER  "http://api.thingspeak.com/update"
unsigned long lastTS = 0;
const unsigned long TS_INTERVAL = 20000;

// ================= GOOGLE SHEETS =================
const char* GS_HOST = "script.google.com";
const int   GS_PORT = 443;
String GAS_ID = "AKfycbw4tG_-U0sYCvoJT9WJUjzlEdRoTpc4Kfga6ol2EFnY_ct3by2ssGVey892JxOY4wpR";
unsigned long lastGS = 0;
const unsigned long GS_INTERVAL = 10000;

// ================= EMAIL =================
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT esp_mail_smtp_port_587
#define AUTHOR_EMAIL    "dbatumart2025@gmail.com"
#define AUTHOR_PASSWORD "wpoj mqfd kvoa gmuj"
#define RECIPIENT_EMAIL "meghapro11@gmail.com"

SMTPSession smtp;

// ================= CLIENTS =================
WiFiClient clientTS;
WiFiClientSecure clientGS;

// ================= DATA =================
int   T = -1, H = -1, water = -1, soil = -1, light = -1, ALERT = 0;
float ph = 0.0;

bool dataValid = false;
bool lastAlertState = false;
bool emailSent = false;

// =================================================
void setup() {
  Serial.begin(9600);            // USB debug
  arduinoSerial.begin(9600);     // Arduino data
  delay(2000);                   // allow ESP boot messages to finish

  while (arduinoSerial.available()) arduinoSerial.read();

  Serial.println("NodeMCU Started (SoftwareSerial Mode)");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.println(WiFi.localIP());

  clientGS.setInsecure();
  MailClient.networkReconnect(true);
  smtp.setTCPTimeout(60);   // prevent timeout
}

// =================================================
void loop() {

  // -------- RECEIVE FROM ARDUINO --------
  if (arduinoSerial.available()) {
    String s = arduinoSerial.readStringUntil('\n');
    s.trim();

    // Accept only printable ASCII
    bool validChars = true;
    for (int i = 0; i < s.length(); i++) {
      if (s.charAt(i) < 32 || s.charAt(i) > 126) {
        validChars = false;
        break;
      }
    }
    if (!validChars) return;

    Serial.println("RAW: " + s);

    // Must contain exactly 6 commas
    int commaCount = 0;
    for (int i = 0; i < s.length(); i++)
      if (s.charAt(i) == ',') commaCount++;

    if (commaCount != 6) {
      Serial.println("INVALID CSV DROPPED");
      return;
    }

    parseCSV(s);
  }

  // -------- CLOUD UPDATES --------
  if (dataValid && millis() - lastTS >= TS_INTERVAL) {
    sendToThingSpeak();
    lastTS = millis();
  }

  if (dataValid && millis() - lastGS >= GS_INTERVAL) {
    sendToGoogleSheets();
    lastGS = millis();
  }

  // -------- EMAIL (EDGE TRIGGERED) --------
  bool currentAlert = (ALERT == 1);

  if (currentAlert && !lastAlertState && !emailSent) {
    sendEmail();
    emailSent = true;
  }

  if (!currentAlert && lastAlertState) {
    emailSent = false;   // reset when normal
  }

  lastAlertState = currentAlert;
}

// =================================================
void parseCSV(String s) {

  int i1=s.indexOf(','), i2=s.indexOf(',',i1+1),
      i3=s.indexOf(',',i2+1), i4=s.indexOf(',',i3+1),
      i5=s.indexOf(',',i4+1), i6=s.indexOf(',',i5+1);

  T     = s.substring(0,i1).toInt();
  H     = s.substring(i1+1,i2).toInt();
  water = s.substring(i2+1,i3).toInt();
  soil  = s.substring(i3+1,i4).toInt();
  light = s.substring(i4+1,i5).toInt();
  ph    = s.substring(i5+1,i6).toFloat();
  ALERT = s.substring(i6+1).toInt();

  dataValid = true;

  Serial.println("CSV ACCEPTED");
}

// =================================================
void sendToThingSpeak() {

  String url = String(TS_SERVER) +
               "?api_key=" + TS_API_KEY +
               "&field1=" + T +
               "&field2=" + H +
               "&field3=" + water +
               "&field4=" + light +
               "&field5=" + soil +
               "&field6=" + String(ph,2);

  HTTPClient http;
  http.begin(clientTS, url);
  http.GET();
  http.end();

  Serial.println("ThingSpeak Updated");
}

// =================================================
void sendToGoogleSheets() {

  if (!clientGS.connect(GS_HOST, GS_PORT)) return;

  String url = "/macros/s/" + GAS_ID + "/exec" +
               "?temperature=" + String(T) +
               "&humidity=" + String(H) +
               "&water=" + String(water) +
               "&soil=" + String(soil) +
               "&light=" + String(light) +
               "&ph=" + String(ph,2);

  clientGS.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + GS_HOST + "\r\n" +
                 "Connection: close\r\n\r\n");

  Serial.println("Google Sheet Updated");
}

// =================================================
void sendEmail() {

  Session_Config cfg;
  cfg.server.host_name = SMTP_HOST;
  cfg.server.port = SMTP_PORT;
  cfg.login.email = AUTHOR_EMAIL;
  cfg.login.password = AUTHOR_PASSWORD;

  SMTP_Message msg;
  msg.sender.name = "Smart Greenhouse";
  msg.sender.email = AUTHOR_EMAIL;
  msg.subject = "🚨 GREENHOUSE ALERT";
  msg.addRecipient("Admin", RECIPIENT_EMAIL);

  msg.text.content =
    msg.text.content =
  "GREENHOUSE ALERT\n"
  "T:" + String(T) +
  " H:" + String(H) +
  " W:" + String(water) +
  " S:" + String(soil) +
  " pH:" + String(ph,2);


  smtp.connect(&cfg);
  MailClient.sendMail(&smtp, &msg);

  Serial.println("EMAIL ALERT SENT");
}
