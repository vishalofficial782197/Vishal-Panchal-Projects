/* ESP32-CAM Async MJPEG stream + Servo + Flash + Telegram
   - Uses AsyncWebServer to avoid blocking the main server during streaming
   - Commands: /start, /ip, /left, /right, /flash, /photo
   - Replace BOT_TOKEN locally (DO NOT share)
   - Install: ESPAsyncWebServer, AsyncTCP, UniversalTelegramBot, ESP32Servo
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>
#include <UniversalTelegramBot.h>
#include "esp_camera.h"

// ----------------- USER CONFIG (EDIT LOCALLY) -----------------
const char* WIFI_SSID = "M12";
const char* WIFI_PASSWORD = "11111110";

// Put your bot token here locally (DO NOT share it publicly)
const char* BOT_TOKEN = "YOUR_BOT_TOKEN_HERE";

// Optional admin chat id (string) to receive boot notifications
const char* ADMIN_CHAT_ID = nullptr; // set to "123456789" to enable

// Local uploaded file path (your tooling will transform to public URL)
const char* UPLOADED_FILE_PATH = "/mnt/data/Screenshot (153).png";
// ---------------------------------------------------------------

const int SERVO_PIN = 13;
const int FLASH_PIN = 4;
const int SERVO_MIN_ANGLE = 0;
const int SERVO_MAX_ANGLE = 180;
const int STEP_DEGREE = 10;

AsyncWebServer server(80);
Servo camServo;
int currentAngle = 90;
bool flashOn = false;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

const unsigned long BOT_POLL_INTERVAL = 2000;
unsigned long lastBotPoll = 0;

// Camera config (AI-Thinker)
camera_config_t config = {
  .pin_pwdn  = 32,
  .pin_reset = -1,
  .pin_xclk = 0,
  .pin_sscb_sda = 26,
  .pin_sscb_scl = 27,
  .pin_d7 = 35,
  .pin_d6 = 34,
  .pin_d5 = 39,
  .pin_d4 = 36,
  .pin_d3 = 21,
  .pin_d2 = 19,
  .pin_d1 = 18,
  .pin_d0 = 5,
  .pin_vsync = 25,
  .pin_href = 23,
  .pin_pclk = 22,
  .xclk_freq_hz = 20000000,
  .ledc_timer = LEDC_TIMER_0,
  .ledc_channel = LEDC_CHANNEL_0,
  .pixel_format = PIXFORMAT_JPEG,
  .frame_size = FRAMESIZE_QVGA, // use QVGA for smoother frames (change to VGA if you prefer)
  .jpeg_quality = 12,
  .fb_count = 2,
  .grab_mode = CAMERA_GRAB_LATEST
};

// ---------- camera init ----------
void setupCamera(){
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed 0x%x\n", err);
    while(true) delay(1000);
  }
}

// ---------- Async MJPEG stream generator ----------
#include <functional>

// We'll implement a chunked response using beginChunkedResponse
AsyncWebServerResponse* mjpegStreamResponse(AsyncWebServerRequest *request) {
  // This lambda is called repeatedly to fill chunks
  auto callback = [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
    // index is ignored here, it's used by Async lib to track stream progress
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      // no frame -> return 0 to end stream (or we could wait & try again)
      return 0;
    }

    // prepare header for this frame
    String part = "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: " + String(fb->len) + "\r\n\r\n";
    size_t headerLen = part.length();
    size_t totalNeeded = headerLen + fb->len + 2; // +2 for \r\n after frame

    if (totalNeeded > maxLen) {
      // unlikely for reasonable buffer sizes, drop frame
      esp_camera_fb_return(fb);
      return 0;
    }

    // copy header
    memcpy(buffer, part.c_str(), headerLen);
    // copy image
    memcpy(buffer + headerLen, fb->buf, fb->len);
    // append CRLF
    buffer[headerLen + fb->len] = '\r';
    buffer[headerLen + fb->len + 1] = '\n';

    // done
    size_t wrote = headerLen + fb->len + 2;
    esp_camera_fb_return(fb);
    return wrote;
  };

  AsyncWebServerResponse *response = request->beginChunkedResponse("multipart/x-mixed-replace; boundary=frame", callback);
  return response;
}

// ---------- Web UI (same UI you provided, using /stream) ----------
const char index_html[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32-CAM Live</title>
  <style>
    body { font-family: Arial, Helvetica, sans-serif; text-align:center; margin:0; padding:8px; background:#111; color:#eee; }
    .container { max-width:700px; margin: 0 auto; }
    #video { width:560px; max-width:100%; height:auto; border:4px solid #222; box-shadow: 0 0 10px rgba(0,0,0,0.5); display:block; margin:0 auto; }
    .controls { margin-top:10px; display:flex; justify-content:center; gap:10px; align-items:center; }
    .btn { width:72px; height:48px; font-size:18px; border-radius:8px; border:none; cursor:pointer; display:flex; align-items:center; justify-content:center;}
    #left { background:#f39c12; color:#111;}
    #right { background:#27ae60; color:#fff;}
    #flash { background:#3498db; color:#fff;}
    #statusRow { margin-top:10px; display:flex; justify-content:space-between; color:#ddd; font-size:16px; }
    #debug { margin-top:8px; color:#aaa; font-size:12px; min-height:18px;}
  </style>
</head>
<body>
  <div class="container">
    <h2>ESP32-CAM Live</h2>
    <img id="video" src="/stream" alt="stream">
    <div class="controls" aria-label="controls">
      <button id="left" class="btn" type="button" title="Left">&lt;</button>
      <button id="flash" class="btn" type="button" title="Flash">FL</button>
      <button id="right" class="btn" type="button" title="Right">&gt;</button>
    </div>
    <div id="statusRow">
      <div>Position: <span id="angle">90</span>°</div>
      <div>Flash: <span id="flashState">OFF</span></div>
    </div>
    <div id="debug" aria-live="polite">Debug: ready</div>
  </div>

  <script>
    document.addEventListener('DOMContentLoaded', function(){
      document.getElementById('left').textContent = '<';
      document.getElementById('right').textContent = '>';
      document.getElementById('flash').textContent = 'FL';
    });

    async function fetchJson(url, opts) {
      try {
        const res = await fetch(url, opts);
        if (!res.ok) {
          const txt = await res.text();
          console.error('Fetch failed', url, res.status, txt);
          document.getElementById('debug').textContent = 'Network error: ' + res.status;
          return null;
        }
        return await res.json();
      } catch (err) {
        console.error('Fetch exception', url, err);
        document.getElementById('debug').textContent = 'Fetch exception: ' + err;
        return null;
      }
    }

    async function servoAction(dir, e){
      if (e) e.preventDefault();
      document.getElementById('debug').textContent = 'Sending ' + dir + '...';
      const data = await fetchJson('/servo?dir=' + encodeURIComponent(dir));
      if (data && typeof data.angle === 'number') {
        document.getElementById('angle').textContent = data.angle;
        document.getElementById('debug').textContent = 'Moved to ' + data.angle + '°';
      } else {
        document.getElementById('debug').textContent = 'Servo request failed (see console)';
      }
    }

    async function flashToggle(e){
      if (e) e.preventDefault();
      document.getElementById('debug').textContent = 'Toggling flash...';
      const data = await fetchJson('/flash', { method: 'POST' });
      if (data && typeof data.flash === 'boolean') {
        document.getElementById('flashState').textContent = data.flash ? 'ON' : 'OFF';
        document.getElementById('debug').textContent = 'Flash ' + (data.flash ? 'ON' : 'OFF');
      } else {
        document.getElementById('debug').textContent = 'Flash toggle failed (see console)';
      }
    }

    document.getElementById('left').addEventListener('click', function(e){ servoAction('left', e); });
    document.getElementById('right').addEventListener('click', function(e){ servoAction('right', e); });
    document.getElementById('flash').addEventListener('click', function(e){ flashToggle(e); });

    // poll status every 2s to keep UI in sync
    setInterval(async ()=>{
      const s = await fetchJson('/servo?dir=status');
      if (s && typeof s.angle === 'number') document.getElementById('angle').textContent = s.angle;
      const f = await fetchJson('/flash?status=1');
      if (f && typeof f.flash === 'boolean') document.getElementById('flashState').textContent = f.flash ? 'ON' : 'OFF';
    }, 2000);
  </script>
</body>
</html>
)rawliteral";

// ---------- endpoints: servo / flash / capture fallback ----------
void handleServo(AsyncWebServerRequest *request){
  String angleStr;
  if (request->hasParam("dir")) {
    String dir = request->getParam("dir")->value();
    if (dir == "left") {
      currentAngle -= STEP_DEGREE;
      if (currentAngle < SERVO_MIN_ANGLE) currentAngle = SERVO_MIN_ANGLE;
      camServo.write(currentAngle);
      Serial.printf("Servo left -> %d\n", currentAngle);
    } else if (dir == "right") {
      currentAngle += STEP_DEGREE;
      if (currentAngle > SERVO_MAX_ANGLE) currentAngle = SERVO_MAX_ANGLE;
      camServo.write(currentAngle);
      Serial.printf("Servo right -> %d\n", currentAngle);
    }
  }
  String json = String("{\"angle\":") + String(currentAngle) + String("}");
  request->send(200, "application/json; charset=utf-8", json);
}

void handleFlash(AsyncWebServerRequest *request){
  if (request->method() == HTTP_POST) {
    flashOn = !flashOn;
    digitalWrite(FLASH_PIN, flashOn ? HIGH : LOW);
    Serial.printf("Flash toggled -> %d\n", flashOn);
  }
  String json = String("{\"flash\":") + (flashOn ? "true" : "false") + String("}");
  request->send(200, "application/json; charset=utf-8", json);
}

void handleCapture(AsyncWebServerRequest *request){
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    request->send(500, "text/plain", "Camera capture failed");
    return;
  }
  AsyncWebServerResponse *response = request->beginResponse("image/jpeg", fb->len, (const char*)fb->buf);
  response->addHeader("Content-Length", String(fb->len));
  request->send(response);
  esp_camera_fb_return(fb);
}

// ---------- Telegram binary helpers ----------
camera_fb_t *photo_fb = nullptr;
bool photo_pending = false;
bool isMoreDataAvailable() {
  if (photo_pending) { photo_pending = false; return true; }
  return false;
}
byte *getNextBuffer() { if (photo_fb) return photo_fb->buf; return nullptr; }
int getNextBufferLen() { if (photo_fb) return photo_fb->len; return 0; }

// ---------- Telegram message handler ----------
void handleNewMessages(int numNew) {
  Serial.printf("handleNewMessages: %d\n", numNew);
  for (int i = 0; i < numNew; ++i) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String fromName = bot.messages[i].from_name;
    if (fromName == "") fromName = "Guest";
    Serial.printf("Message from %s: %s\n", fromName.c_str(), text.c_str());

    String cmd = text;
    cmd.toLowerCase();

    if (cmd == "/start") {
      String welcome = "ESP32-CAM bot\nCommands:\n/photo - take and send photo\n/flash - toggle flash\n/left - servo left\n/right - servo right\n/ip - get device IP";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (cmd == "/ip") {
      String ip = WiFi.localIP().toString();
      String link = "http://" + ip + "/";
      bot.sendMessage(chat_id, "IP: " + ip + "\nOpen: " + link, "");
    }
    else if (cmd == "/left") {
      currentAngle -= STEP_DEGREE;
      if (currentAngle < SERVO_MIN_ANGLE) currentAngle = SERVO_MIN_ANGLE;
      camServo.write(currentAngle);
      bot.sendMessage(chat_id, "Moved left -> angle: " + String(currentAngle), "");
      Serial.printf("Servo left -> %d\n", currentAngle);
    }
    else if (cmd == "/right") {
      currentAngle += STEP_DEGREE;
      if (currentAngle > SERVO_MAX_ANGLE) currentAngle = SERVO_MAX_ANGLE;
      camServo.write(currentAngle);
      bot.sendMessage(chat_id, "Moved right -> angle: " + String(currentAngle), "");
      Serial.printf("Servo right -> %d\n", currentAngle);
    }
    else if (cmd == "/flash") {
      flashOn = !flashOn;
      digitalWrite(FLASH_PIN, flashOn ? HIGH : LOW);
      bot.sendMessage(chat_id, String("Flash ") + (flashOn ? "ON" : "OFF"), "");
      Serial.printf("Flash toggled -> %d\n", flashOn);
    }
    else if (cmd == "/photo") {
      if (photo_fb) { esp_camera_fb_return(photo_fb); photo_fb = nullptr; }
      photo_fb = esp_camera_fb_get();
      if (!photo_fb) {
        bot.sendMessage(chat_id, "Camera capture failed", "");
        Serial.println("Camera capture failed");
        continue;
      }
      photo_pending = true;
      Serial.println("Sending photo (binary)...");
      bot.sendPhotoByBinary(chat_id, "image/jpeg", photo_fb->len,
                            isMoreDataAvailable, nullptr,
                            getNextBuffer, getNextBufferLen);
      Serial.println("Photo sent");
      esp_camera_fb_return(photo_fb);
      photo_fb = nullptr;
      photo_pending = false;
    } else {
      // ignore
    }
  }
}

// ---------- setup & loop ----------
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("ESP32-CAM Async MJPEG + Telegram");

  pinMode(FLASH_PIN, OUTPUT);
  digitalWrite(FLASH_PIN, flashOn ? HIGH : LOW);

  camServo.attach(SERVO_PIN);
  camServo.write(currentAngle);

  setupCamera();

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("Connecting to WiFi '%s' ...\n", WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println();
  Serial.print("Connected! IP: "); Serial.println(WiFi.localIP());

  // Telegram TLS (use setCACert for production)
  secured_client.setInsecure();

  // notify admin if set (and send uploaded file path as photo URL)
  if (ADMIN_CHAT_ID != nullptr) {
    String ip = WiFi.localIP().toString();
    String link = "http://" + ip + "/";
    bot.sendMessage(ADMIN_CHAT_ID, "ESP32-CAM online!\nIP: " + ip + "\nOpen: " + link, "");
    // send uploaded local path (tooling will transform)
    bot.sendPhoto(ADMIN_CHAT_ID, String(UPLOADED_FILE_PATH), "Boot screenshot (local path as URL)");
  }

  // Async routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html; charset=utf-8", index_html);
  });

  server.on("/stream", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = mjpegStreamResponse(request);
    request->send(response);
  });

  server.on("/capture", HTTP_GET, handleCapture);
  server.on("/servo", HTTP_GET, handleServo);
  server.on("/flash", HTTP_ANY, handleFlash);

  server.begin();
  Serial.println("Async HTTP server started");

  lastBotPoll = millis();
}

void loop() {
  // Poll Telegram
  if (millis() - lastBotPoll > BOT_POLL_INTERVAL) {
    int numNew = bot.getUpdates(bot.last_message_received + 1);
    if (numNew) handleNewMessages(numNew);
    lastBotPoll = millis();
  }
  delay(2);
}
