#include <WiFi.h>
#include <WiFiUdp.h>


// led test
#if 0
#define LED_PIN 2  // 내장 또는 외부 LED 연결 핀

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);  // LED ON
  delay(1000);                  // 1초 대기
  digitalWrite(LED_PIN, LOW);   // LED OFF
  delay(1000);                  // 1초 대기
}

#endif

// ***
#if 01

const char* ssid = "netis";
const char* password = "dmssdmss";

WiFiUDP udp;
const int localPort = 7;
const IPAddress stm32_ip(192,168,1,3);
const int stm32_port = 7;

// 초음파 센서 핀
#define echoPin 18
#define trigPin 5

// LED 핀
#define LED_PIN 2 

// 시간 관련 변수
unsigned long time_offset = 0;
unsigned long last_send_time = 0;
unsigned long led_on_time = 0;

const unsigned long send_interval = 300;    // 거리 전송 주기(ms)
const unsigned long led_duration = 1000;    // LED 유지 시간(ms)

bool udp_connected = false;

// ──────────────── LED 제어 함수 ────────────────
void handleLedOn() {
    digitalWrite(LED_PIN, HIGH);
}

void handleLedOff() {
    digitalWrite(LED_PIN, LOW);
}

// ──────────────── 초기 패킷 전송 ────────────────
void send_initial_packet() {
    const char* init_msg = "hello";
    udp.beginPacket(stm32_ip, stm32_port);
    udp.write((const uint8_t*)init_msg, strlen(init_msg));
    udp.endPacket();
    Serial.println("[SEND] Initial hello sent to STM32 to register as client.");
}

// ──────────────── SETUP ────────────────
void setup() {
    Serial.begin(115200);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(LED_PIN, OUTPUT);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("[INFO] WiFi connected. IP: " + WiFi.localIP().toString());

    udp.begin(localPort);
    Serial.println("[INFO] UDP listener started.");

    send_initial_packet();
}

// ──────────────── LOOP ────────────────
void loop() {
    // 수신처리 
    char incoming[64];
    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
        udp.read(incoming, sizeof(incoming) - 1);
        incoming[packetSize] = '\0';

        if (strcmp(incoming, "sync") == 0) {
            if (!udp_connected) {
                udp_connected = true;
                Serial.println("[INFO] UDP connection established with STM32.");
            }
            Serial.println("[RECV] Received sync signal. Resetting time_offset.");
            time_offset = millis();
        }
        else if (strcmp(incoming, "fire") == 0) {
            Serial.println("[RECV] Received fire signal. LED ON for 1 sec.");
            handleLedOn();
            led_on_time = millis();
        }
        else {
            Serial.printf("[RECV] Unknown message: %s\n", incoming);
        }
    }

    // LED 자동 OFF 처리: 항상 확인
    if (led_on_time > 0 && millis() - led_on_time >= led_duration) {
        handleLedOff();
        led_on_time = 0;
        Serial.println("[INFO] LED OFF after timeout.");
    }

    // 거리 측정 & 전송
    if (udp_connected && millis() - last_send_time >= send_interval) {
        last_send_time = millis();

        long duration;
        int distance;

        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);

        duration = pulseIn(echoPin, HIGH, 30000); // 30ms 타임아웃
        if (duration == 0) {
            Serial.println("[WARN] Timeout: no echo received.");
            return;
        }

        distance = duration * 0.0344 / 2;
        unsigned long relative_time = millis() - time_offset;

        char msg[32];
        snprintf(msg, sizeof(msg), "%d(%lu)", distance, relative_time);

        udp.beginPacket(stm32_ip, stm32_port);
        udp.write((const uint8_t*)msg, strlen(msg));
        udp.endPacket();

        Serial.printf("[SEND] Distance data sent: %s\n", msg);
    }

    delay(10);  // CPU 사용량 줄이기
}

# endif