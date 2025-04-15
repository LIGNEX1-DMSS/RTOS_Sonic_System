# 음파전 체계 UDP 통신 프로토콜 구축

- **Board**: Nucleo-F767ZI  
- **Tool**: STM32CubeIDE  
- **Protocol**: UDP (Broadcast 기반 통신)

---

## 🛰 UDP 방식 선택 이유

| 항목                | 설명                                                                 |
|---------------------|----------------------------------------------------------------------|
| 통제소 다수 존재       | 여러 클라이언트가 동시에 데이터를 송신해야 하는 구조                            |
| 고속 실시간 송신 필요   | TCP의 연결 설정/해제 오버헤드로 인해 실시간 전송에 부적합                          |
| 데이터의 순간성 중시   | 현재 시점의 데이터만 중요하며, 과거 데이터는 불필요                              |
| 신뢰성보다 속도 우선    | 재전송, 순서 보장, 체크섬 등이 필요 없기 때문에 UDP가 더 효율적                     |

---

## 🧩 시스템 구성

### 📡 SERVER (STM32)

- 구성: `LwIP + ETH + FreeRTOS + udp_echoserver.c`
- 기능:
  - `ping`으로 네트워크 연결 확인
  - **수신**: 브로드캐스트 수신
  - **송신**: 수신한 클라이언트 IP 기억 → 해당 IP로 `sync` 신호 전송 (동시 송신)

### 📲 CLIENT (PC)

- 송신 데이터: `"센서값(경과밀리초)"` 형식  
  예: `58(1325)` → 1.325초 경과 시점에 `58`이라는 값 전송
- **송신**: 500ms 주기로 센서 데이터를 브로드캐스트
- **수신**: 서버의 `sync` 수신 시 `miles` 초기화 (0으로)

---

## ⚙️ STM32CubeIDE 설정

| 항목                   | 설정 값                                      |
|------------------------|----------------------------------------------|
| Code Generation Option | 사용                                          |
| System Timer           | TIM6                                          |
| Ethernet Interface     | RMII                                          |
| FreeRTOS 설정           | CMSIS_V1, MINIMAL_STACK_SIZE = 256 words     |
|                        | TOTAL_HEAP_SIZE = 32768 bytes                |
| LwIP 설정               | DHCP 비활성화, Static IP 사용                |
| 클럭 설정               | HSE 사용, HCLK = 168 MHz                     |

### ⚠️ 주의 사항
- `LWIP + ETH + RTOS`를 동시에 활성화 시 컴파일 경고 발생  
- `MX_LWIP_Init()` 호출 시 **하드폴트 발생**

→ 위 설정을 통해 문제 해결

---

## 📚 Reference

- [[STM32 HAL] RTOS + LwIP TCP Echo Server (by eziya76)](https://m.blog.naver.com/eziya76/221867311729?recommendTrackingCode=2)

