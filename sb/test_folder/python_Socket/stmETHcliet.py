import socket
import time

# STM32 보드의 IP 및 포트 설정
STM32_IP = "192.168.1.3"
STM32_PORT = 7

# UDP 소켓 생성
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

try:
    while True:
        message = "Hello STM32"
        sock.sendto(message.encode(), (STM32_IP, STM32_PORT))
        print(f"Sent: {message}")
        time.sleep(1)  # 1초 대기
except KeyboardInterrupt:
    print("\nStopping UDP sender.")
finally:
    sock.close()
