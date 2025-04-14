## RTOS Sonic System

### 현재 진행 중
STM32F429 MCU에서 FreeRTOS (CMSIS-RTOS2 버전)을 이용하여 가상 센서 데이터 생성, 데이터 비교, 결과 발송, UART 메시지 수신을 구현한 시스템
**TaskCmd - UDP 송신 추가 필요**

| 테스트 | 기능 |
|----------|------|
| TaskGen1, TaskGen2 | 각각 센서 1, 2 데이터 생성 (변수 ID에 따라 변경) |
| TaskComp | 처리: 최신 데이터 2개 목록 및 비교 |
| TaskTx | 데이터 차이 계산 및 UART 발송 |
| TaskCmd | PC에서 수신한 UART 메시지 수신 및 ACK/NACK 발송 |

### RTOS 자원
- **Queue**
  - `sensorQueue`: 센서 데이터 전달
  - `compInputQueue`: 발송 대상 데이터 2개 목록
- **Mutex**
  - `uartMutex`: UART 접근 동기화
- **Semaphore**
  - `notifySem`: UART 메시지 수신 마칠 때 필요

### UART 메시지 형식

- PC에서 UART로 발송하는 메시지는 마지막에 `;`가 포함되어야 함
  ```
  A;
  B;
  ```
- 여러 메시지 형식:
  - `A;` → 반응: `ACK_FIRE:A\r\n`
  - `B;` → 반응: `ACK_FIRE:B\r\n`
  - 기타 → 반응: `NACK_FIRE\r\n`

## 개발 환경

- MCU: STM32F429
- IDE: STM32CubeIDE
- UART: USART1 (Baudrate 115200)
- RTOS: FreeRTOS (CMSIS-RTOS2 wrapper)
