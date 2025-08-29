#include <WiFiNINA.h>
#include <PubSubClient.h>

// ---------- USER SETTINGS ----------
const char* WIFI_SSID     = "Ok Bro";
const char* WIFI_PASSWORD = "        ";

// Task sheet: use EMQX public broker and port 1883
// Broker: broker.emqx.io, Port: 1883  (per SIT210 3.3D instructions)
const char* MQTT_SERVER = "broker.emqx.io";
const uint16_t MQTT_PORT = 1883;

// Topic per task sheet
const char* TOPIC = "SIT210/wave"; // publish + subscribe

// Put YOUR NAME here exactly as you want in the payload
const char* YOUR_NAME = "Jaideep Gera";

// ---------- PINS ----------
#define TRIG_PIN   5
#define ECHO_PIN  4   // Use a voltage divider on Echo (5V -> ~3.0-3.3V)
#define LED_PIN    2   // or use LED_BUILTIN

// ---------- ULTRASONIC / GESTURE TUNING ----------
const unsigned long MAX_ECHO_TIME = 30000UL; // ~5m timeout
const float SOUND_SPEED_CM_PER_US = 0.0343f;

// Gesture thresholds
const int NEAR_WAVE_CM = 15;  // near threshold for wave
const int NEAR_PAT_CM  = 12;  // near threshold for pat
const unsigned long PAT_MAX_MS   = 200;  // quick tap
const unsigned long WAVE_MIN_MS  = 300;  // linger a bit
const unsigned long WAVE_MAX_MS  = 1500; // but not too long

// ---------- GLOBALS ----------
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

enum NearState { FAR, NEAR };
NearState state = FAR;
unsigned long nearStartMs = 0;
int lastDistance = 999;

void flashQuickWave() {
  // 3 quick flashes: 200ms ON, 100ms OFF
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH); delay(200);
    digitalWrite(LED_PIN, LOW);  delay(100);
  }
}

void flashSlowPat() {
  // 2 slow pulses: 600ms ON, 400ms OFF
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_PIN, HIGH); delay(600);
    digitalWrite(LED_PIN, LOW);  delay(400);
  }
}

long readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, MAX_ECHO_TIME);
  if (duration == 0) {
    return 999; // timeout => very far
  }
  float cm = (duration * SOUND_SPEED_CM_PER_US) / 2.0f;
  return (long)cm;
}

void publishWave() {
  char msg[64];
  snprintf(msg, sizeof(msg), "WAVE: %s", YOUR_NAME);
  mqttClient.publish(TOPIC, msg);
  Serial.print("[PUB] "); Serial.println(msg);
}

void publishPat() {
  char msg[64];
  snprintf(msg, sizeof(msg), "PAT: %s", YOUR_NAME);
  mqttClient.publish(TOPIC, msg);
  Serial.print("[PUB] "); Serial.println(msg);
}

void handleGestureLogic(int d) {
  bool nearForPat = (d <= NEAR_PAT_CM);
  bool nearForWave = (d <= NEAR_WAVE_CM);

  switch (state) {
    case FAR:
      if (nearForPat || nearForWave) {
        state = NEAR;
        nearStartMs = millis();
      }
      break;
    case NEAR: {
      // We’re near; check how long
      unsigned long held = millis() - nearStartMs;
      if (!nearForWave && !nearForPat) {
        // Hand moved away: classify based on how long it stayed near
        if (held <= PAT_MAX_MS) {
          publishPat();
        } else if (held >= WAVE_MIN_MS && held <= WAVE_MAX_MS) {
          publishWave();
        }
        state = FAR;
      }
    } break;
  }

  lastDistance = d;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Copy payload to a safe, null-terminated buffer
  static char buf[128];
  unsigned int n = (length < sizeof(buf) - 1) ? length : sizeof(buf) - 1;
  memcpy(buf, payload, n);
  buf[n] = '\0';

  Serial.print("[SUB] Topic: "); Serial.print(topic);
  Serial.print(" | Msg: "); Serial.println(buf);

  // LED reaction per requirement:
  // - Any message: flash 3x (wave)
  // - If message starts with "PAT:", flash differently
  if (strncmp(buf, "PAT:", 4) == 0) {
    flashSlowPat();
  } else {
    // Default for “WAVE:” or any other text
    flashQuickWave();
  }
}

void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Connecting to WiFi: "); Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start > 20000) { // 20s timeout
      Serial.println("\nWiFi connect retry...");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      start = millis();
    }
  }
  Serial.print("\nWiFi connected. IP: ");
  Serial.println(WiFi.localIP());
}

void ensureMQTT() {
  if (mqttClient.connected()) return;

  Serial.print("Connecting to MQTT: ");
  Serial.print(MQTT_SERVER); Serial.print(":"); Serial.println(MQTT_PORT);

  while (!mqttClient.connected()) {
    String clientId = "Nano33IoT-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("MQTT connected.");
      mqttClient.subscribe(TOPIC); // subscribe per task sheet
      Serial.print("Subscribed to "); Serial.println(TOPIC);
    } else {
      Serial.print("Failed, rc="); Serial.print(mqttClient.state());
      Serial.println(" retrying in 2 seconds...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  ensureWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
}

void loop() {
  // Keep network + MQTT alive
  ensureWiFi();
  ensureMQTT();
  mqttClient.loop();

  // Read distance and run gesture detection
  int d = (int)readDistanceCm();
  handleGestureLogic(d);

  delay(30); // ~33 Hz sampling
}