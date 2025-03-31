import socket
import time

# 설정
UDP_PORT = 51234
BROADCAST_IP = "192.168.1.255"  # UDP 브로드캐스트 주소
INTERVAL = 1  # 초 단위 (1초마다 전송)

# UDP 소켓 생성 (수신용)
recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
recv_sock.bind(("0.0.0.0", UDP_PORT))

# UDP 소켓 생성 (브로드캐스트 송신용)
send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
send_sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

# 내 IP 주소 가져오기
hostname = socket.gethostname()
my_ip = socket.gethostbyname(hostname)

start_time = None
cnt = 0

print(f"Listening for 'go' on UDP port {UDP_PORT}...")

while True:
    # 수신 대기
    data, addr = recv_sock.recvfrom(1024)
    message = data.decode().strip()

    # "go"를 수신하면 millis() 시작
    if message == "go" and start_time is None:
        print("Received 'go'! Starting millis() counter...")
        start_time = time.time()  # 현재 시간 저장

    # millis()가 시작된 후 브로드캐스트 전송
    if start_time is not None:
        elapsed_time = int((time.time() - start_time) * 1000)  # millis() 계산
        msg = f"{cnt}, {elapsed_time} ms"
        
        # 브로드캐스트 전송
        send_sock.sendto(msg.encode(), (BROADCAST_IP, UDP_PORT))
        print(f"Sent: {msg}")
        
        cnt += 1
        time.sleep(INTERVAL)  # 일정 간격마다 전송
