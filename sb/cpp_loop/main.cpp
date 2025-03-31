#include <Arduino.h>
#include <STM32Ethernet.h>
#include <Udp.h>

HardwareSerial Serial2(PD2, PC12);

// 네트워크 설정
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 3);     // 이 STM32 보드의 IP
unsigned int port = 51234;
EthernetUDP udp;

// 송신자 데이터 구조체
struct SenderData {
  IPAddress ip;
  int cnt;
  unsigned long millis;
  bool valid;
};

SenderData sender1;
SenderData sender2;

// 타이밍 변수
unsigned long lastCompTask = 0;
unsigned long lastSynTaskTime = 0;
unsigned long lastPcCmdTaskTime = 0;


// 초기화 함수
void resetSenderData(SenderData &s) {
  s.cnt = 0;
  s.millis = 0;
  s.valid = false;
}

// 수신된 메시지를 파싱하여 저장
void parseAndStorePacket(String message, IPAddress senderIp) {
  int commaIndex = message.indexOf(',');
  if (commaIndex == -1) return;

  int cnt = message.substring(0, commaIndex).toInt();
  int msIndex = message.indexOf("ms");
  if (msIndex == -1) return;

  unsigned long millisValue = message.substring(commaIndex + 1, msIndex).toInt();

  SenderData* target = nullptr;

  if (senderIp == IPAddress(192, 168, 1, 3)) {
    target = &sender1;
  } else if (senderIp == IPAddress(192, 168, 1, 4)) {
    target = &sender2;
  } else {
    return; // 다른 IP는 무시
  }

  target->ip = senderIp;
  target->cnt = cnt;
  target->millis = millisValue;
  target->valid = true;

  // Serial.print("[espRxTask] ");
  // Serial.print(senderIp);
  // Serial.print(" → cnt: ");
  // Serial.print(cnt);
  // Serial.print(", millis: ");
  // Serial.println(millisValue);

}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);

  Serial.println("STM32 UDP Monitor Booting...");
  Ethernet.begin(mac); // DHCP
  udp.begin(port);

  Serial.print("My IP: ");
  Serial.println(Ethernet.localIP());
  Serial.println("Listening for UDP packets...");

  resetSenderData(sender1);
  resetSenderData(sender2);
}

void loop() {
  unsigned long now = millis();

  // === espRxTask ===
  if (1) {
    int packetSize = udp.parsePacket();

    if (packetSize) {
        char packetBuffer[256];
        int len = udp.read(packetBuffer, sizeof(packetBuffer) - 1);
        if (len > 0) {
            packetBuffer[len] = '\0';
        }

        String receivedPacket = String(packetBuffer);
        IPAddress remoteIp = udp.remoteIP();

        Serial.print("\n[espRxTask] ");
        Serial.print(remoteIp);
        Serial.print("\t ");
        Serial.println(receivedPacket);

        parseAndStorePacket(receivedPacket, remoteIp);

    }
    delay(100);
  }

  // === compTask ===
  if (now - lastCompTask >= 1000) {
    lastCompTask = now;

    // -- data receive debuging.. --

    // Serial.print("[compTask] data1 : ");
    // Serial.print(sender1.cnt);
    // Serial.print(" data2 : ");
    // Serial.println(sender2.cnt);
    
    if (sender1.valid && sender2.valid) {
      Serial.println("\n[compTask] --- Comparison Start ---");

      // Serial.print("Sender1 IP: ");
      // Serial.print(sender1.ip);
      // Serial.print(", cnt: ");
      // Serial.print(sender1.cnt);
      // Serial.print(", millis: ");
      // Serial.println(sender1.millis);

      // Serial.print("Sender2 IP: ");
      // Serial.print(sender2.ip);
      // Serial.print(", cnt: ");
      // Serial.print(sender2.cnt);
      // Serial.print(", millis: ");
      // Serial.println(sender2.millis);

      int cntDiff = abs(sender1.cnt - sender2.cnt);
      unsigned long timeDiff = abs((long)(sender1.millis - sender2.millis));
      
      if(timeDiff < 2000){

        Serial.print("[compTask] Cnt diff: ");
        Serial.print(cntDiff);
        Serial.print(", Time diff: ");
        Serial.print(timeDiff);
        Serial.print(" ms");

        if(sender1.cnt > sender2.cnt){
          Serial.print("  >> A ");
        }
        else{
          Serial.print("  >> B");
        }
  
        // 초기화
        resetSenderData(sender1);
        resetSenderData(sender2);
      }

    }
  }

  // === synTask (동기화 신호 생성 및 esp로 송신) ===
  if (now - lastSynTaskTime >= 10000) {
    lastSynTaskTime = now;

    String syncSignal = "go";
    Serial.print("\n[synTask] go send");

    udp.beginPacket(IPAddress(192, 168, 1, 255), port);
    udp.write(syncSignal.c_str());
    udp.endPacket();
  }

  // === pcCmdTask (pc로 부터 발사 명령 받아 곧바로 esp 로 송신) ===
  if (now - lastPcCmdTaskTime >= 5000) {
    lastPcCmdTaskTime = now;
    String uartInput = Serial2.readStringUntil('\n');
    uartInput.trim(); // 개행 문자 제거 및 공백 제거

    if (uartInput == "1") {
      Serial.println("[pcCmdTask] FIRE 1");
      udp.beginPacket(IPAddress(192, 168, 1, 255), port);
      udp.write("FIRE_1_GO");
      udp.endPacket();
    } else if (uartInput == "2") {
      Serial.println("[pcCmdTask] FIRE 2");
      udp.beginPacket(IPAddress(192, 168, 1, 255), port);
      udp.write("FIRE_2_GO");
      udp.endPacket();
    } else if (uartInput == "3") {
      Serial.println("[pcCmdTask] FIRE 3");
      udp.beginPacket(IPAddress(192, 168, 1, 255), port);
      udp.write("FIRE_3_GO");
      udp.endPacket();
    } else {
      Serial.println("[pcCmdTask] FIRE");
      udp.beginPacket(IPAddress(192, 168, 1, 255), port);
      udp.write("FIRE");
      udp.endPacket();
    }
}
}