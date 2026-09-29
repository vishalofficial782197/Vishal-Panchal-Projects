  #include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT11.h>
#include <SoftwareSerial.h>

// ================= LCD =================
LiquidCrystal_I2C lcd(0x27, 16, 2);
byte displayState = 0;

// ================= DHT =================
DHT11 dht11(2);

// ================= SENSOR PINS =================
#define waterPower 5
#define waterPin   A0
#define soilPower  4
#define soilPin    A1
#define PH_PIN     A3
#define LDR_DO_PIN 3

// ================= CALIBRATION =================
const int WATER_MAX_RAW = 368;
#define SOIL_DRY_RAW  820   // Raw value when soil is completely dry
#define SOIL_WET_RAW  320   // Raw value when soil is fully wet

// ================= RELAYS =================
#define RELAY_TEMP_FAN    6
#define RELAY_HUM_EXHAUST 7
#define RELAY_SOIL_PUMP   8
#define RELAY_LIGHT_LAMP  9

#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// ================= THRESHOLDS =================
#define TEMP_ON_THRESHOLD  32
#define HUM_ON_THRESHOLD   70
#define SOIL_LOW_THRESHOLD  40 

// ================= BUZZER =================
#define BUZZER_PIN 12

// ================= SERIAL =================
#define ARD_RX 10
#define ARD_TX 11
SoftwareSerial espSerial(ARD_RX, ARD_TX);

// =================================================
int mapPercent(int raw, int maxRaw) {
  long p = map(raw, 0, maxRaw, 0, 100);
  return constrain(p, 0, 100);
}

// =================================================
int readWater() {
  delay(10);
  int v = analogRead(waterPin);
  return mapPercent(v, WATER_MAX_RAW);
}

int readSoil() {
  int raw = analogRead(soilPin);
  int percent = map(raw, SOIL_DRY_RAW, SOIL_WET_RAW, 0, 100);
  return constrain(percent, 0, 100);
}

float readPH() {
  const int N = 10;
  int buf[N];

  // Take samples
  for (int i = 0; i < N; i++) {
    buf[i] = analogRead(PH_PIN);
    delay(10);
  }

  // Manual sort (ascending) — Arduino compatible
  for (int i = 0; i < N - 1; i++) {
    for (int j = i + 1; j < N; j++) {
      if (buf[i] > buf[j]) {
        int temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }

  // Average middle 6 values
  unsigned long avg = 0;
  for (int i = 2; i < 8; i++) {
    avg += buf[i];
  }

  float voltage = (avg * 5.0) / 1024.0 / 6.0;
  float phValue = 3.5 * voltage;   // calibration factor

  return phValue;
}


// ================= SETUP =================
void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  lcd.init();
  lcd.backlight();

  pinMode(waterPower, OUTPUT);
  pinMode(soilPower, OUTPUT);
  pinMode(LDR_DO_PIN, INPUT);

  pinMode(RELAY_TEMP_FAN, OUTPUT);
  pinMode(RELAY_HUM_EXHAUST, OUTPUT);
  pinMode(RELAY_SOIL_PUMP, OUTPUT);
  pinMode(RELAY_LIGHT_LAMP, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(RELAY_TEMP_FAN, RELAY_OFF);
  digitalWrite(RELAY_HUM_EXHAUST, RELAY_OFF);
  digitalWrite(RELAY_SOIL_PUMP, RELAY_OFF);
  digitalWrite(RELAY_LIGHT_LAMP, RELAY_OFF);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(waterPower, HIGH);
  digitalWrite(soilPower, HIGH);
}

// ================= LOOP =================
void loop() {

  int water = readWater();
  int soil  = readSoil();
  bool light = digitalRead(LDR_DO_PIN) == LOW;

  int T = -1, H = -1;
  if (dht11.readTemperatureHumidity(T, H) != 0) {
    T = -1; H = -1;
  }

  float ph = readPH();

  bool anyRelayON = false;

  if (T != -1 && T >= TEMP_ON_THRESHOLD) {
    digitalWrite(RELAY_TEMP_FAN, RELAY_ON);
    anyRelayON = true;
  } else digitalWrite(RELAY_TEMP_FAN, RELAY_OFF);

  if (H != -1 && H >= HUM_ON_THRESHOLD) {
    digitalWrite(RELAY_HUM_EXHAUST, RELAY_ON);
    anyRelayON = true;
  } else digitalWrite(RELAY_HUM_EXHAUST, RELAY_OFF);

  if (soil < SOIL_LOW_THRESHOLD) {
    digitalWrite(RELAY_SOIL_PUMP, RELAY_ON);
    anyRelayON = true;
  } else digitalWrite(RELAY_SOIL_PUMP, RELAY_OFF);

  if (!light) {
    digitalWrite(RELAY_LIGHT_LAMP, RELAY_ON);
    anyRelayON = true;
  } else digitalWrite(RELAY_LIGHT_LAMP, RELAY_OFF);

  digitalWrite(BUZZER_PIN, anyRelayON ? HIGH : LOW);

  int ALERT = anyRelayON ? 1 : 0;

  // ================= LCD =================
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T:"); lcd.print(T);
  lcd.print(" H:"); lcd.print(H);

  lcd.setCursor(0,1);
  if (displayState == 0) {
    lcd.print("W:"); lcd.print(water);
    lcd.print(" S:"); lcd.print(soil);
  } else {
    lcd.print("pH:"); lcd.print(ph,2);
    lcd.print(ALERT ? " ALERT" : " OK");
  }
  displayState = !displayState;

  // ================= SEND CSV =================
  String csv = String(T) + "," + String(H) + "," +
               String(water) + "," + String(soil) + "," +
               String(light) + "," + String(ph,2) + "," +
               String(ALERT);

  espSerial.println(csv);
  Serial.println(csv);

  delay(3000);
}
 