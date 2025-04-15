import socket
import time
import threading

# 설정
UDP_PORT = 51234
BROADCAST_IP = "192.168.1.255"
INTERVAL = 1  # 초 단위

# 본인 IP 주소 구하기
def get_own_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # 연결되지 않아도 IP를 알아낼 수 있는 임시 목적지
        s.connect(("8.8.8.8", 80))
        return s.getsockname()[0]
    finally:
        s.close()

own_ip = get_own_ip()
print(f"My IP address: {own_ip}")

# 소켓 설정
recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
recv_sock.bind(("0.0.0.0", UDP_PORT))
recv_sock.settimeout(0.5)

send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
send_sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

# millis 관련 변수
start_time = time.time()
cnt = 0
lock = threading.Lock()

def broadcast_loop():
    global cnt, start_time
    while True:
        with lock:
            elapsed_time = int((time.time() - start_time) * 1000)
            msg = f"{cnt}, {elapsed_time} ms"
            send_sock.sendto(msg.encode(), (BROADCAST_IP, UDP_PORT))
            print(f"Sent: {msg}")
            cnt += 1
        time.sleep(INTERVAL)

# 브로드캐스트 스레드 시작
threading.Thread(target=broadcast_loop, daemon=True).start()

print(f"Listening for messages on UDP port {UDP_PORT}...")

# 메시지 수신 루프
while True:
    try:
        data, addr = recv_sock.recvfrom(1024)
        sender_ip = addr[0]

        # 본인이 보낸 메시지면 무시
        if sender_ip == own_ip:
            continue

        message = data.decode().strip()
        print(f"received :  {message} ")

        if message == "go":
            with lock:
                start_time = time.time()
                cnt = 0
    except socket.timeout:
        continue
