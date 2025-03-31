# STM32 Ethernet (RMII) + LwIP + UDP 활성화

STM32 + LAN8720 기반 RMII 연결을 통해 Ethernet 통신을 구성하고, LwIP 스택을 이용하여 UDP 데이터를 수신/응답하는 

---

## ✅ 주요 특징

- **RMII 모드 이더넷 통신**
- **LwIP TCP/IP 스택 활용**
- **UDP 소켓 생성 및 수신 처리**
- **Mac, Windows에서 송신한 UDP 패킷 정상 수신**
- **ping 응답 가능**

---

## ⚙️ 설정 요약

### 1. RMII vs MII

| 항목 | MII | RMII |
|------|-----|------|
| 데이터 라인 수 | 16 | 7 |
| 클럭 | Tx/Rx 각각 25 MHz | 공유 50 MHz |
| 속도 | 10/100 Mbps | 10/100 Mbps |
| 핀 수 | 많음 | 적음 |
| STM32에서 | 잘 안 씀 | 주로 사용됨 |

> 대부분의 STM32 이더넷 PHY는 **RMII** 모드 지원  
> 50MHz 클럭 공유로 **하드웨어 설계 간소화**

---

### 2. CubeMX 설정

1. **"Pinout & Configuration" → `ETH` 활성화**
2. `ETH Mode`를 **"RMII"**로 설정
3. **Clock Configuration**에서 `ETH` 클럭을 **50 MHz**로 설정
4. `MCO1` 사용하여 PHY 칩에 클럭 공급
5. **LwIP 활성화**
6. **DHCP는 Disable** (공유기에서 자동 할당되므로)

---

## 📁 코드 구성

### `main.c` - 기본 구조

```c
extern struct netif gnetif;

while (1)
{
    MX_LWIP_Process();
    ethernetif_input(&gnetif);
    sys_check_timeouts();

    printf("Current IP Address: %s\n", ipaddr_ntoa(&gnetif.ip_addr));
}
