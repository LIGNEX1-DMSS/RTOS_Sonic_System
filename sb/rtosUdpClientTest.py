import socket
import time
import random
import threading

# STM32 보드의 IP 및 포트 설정
STM32_IP = "192.168.1.3"
STM32_PORT = 7

# UDP 소켓 생성
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", 50000))  # 수신용 포트 바인딩
sock.settimeout(1)      # 짧은 타임아웃 설정 (수신 쓰레드에서 반복문 탈출용)

# 전역 시작 시간
start_time_lock = threading.Lock()
start_time_ms = time.time() * 1000  # 시작 시각(ms)

def sender():
    while True:
        # 경과 밀리초 계산
        with start_time_lock:
            elapsed_ms = int(time.time() * 1000 - start_time_ms)

        rand_value = random.randint(0, 100)
        message = f"{rand_value}({elapsed_ms})"
        sock.sendto(message.encode(), (STM32_IP, STM32_PORT))
        print(f"[TX] Sent: {message}")
        time.sleep(0.5)

def receiver():
    global start_time_ms
    while True:
        try:
            data, addr = sock.recvfrom(1024)
            msg = data.decode().strip()
            if msg == "sync":
                with start_time_lock:
                    start_time_ms = time.time() * 1000
                print(f"[RX] Received sync from {addr} → milies reset!")
        except socket.timeout:
            continue

try:
    send_thread = threading.Thread(target=sender, daemon=True)
    recv_thread = threading.Thread(target=receiver, daemon=True)

    send_thread.start()
    recv_thread.start()

    while True:
        time.sleep(1)

except KeyboardInterrupt:
    print("\nStopping UDP client.")
finally:
    sock.close()
