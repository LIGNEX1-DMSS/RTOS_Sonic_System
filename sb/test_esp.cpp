#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <atomic>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 7
#define ESP32_IP "192.168.1.4"  // ESP32의 IP 주소로 수정

std::atomic<bool> running(true);

// 메시지 송신 함수
void send_message(const char* msg) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        return;
    }

    sockaddr_in esp32_addr{};
    esp32_addr.sin_family = AF_INET;
    esp32_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, ESP32_IP, &esp32_addr.sin_addr);

    sendto(sock, msg, strlen(msg), 0,
           (sockaddr*)&esp32_addr, sizeof(esp32_addr));

    std::cout << "[SEND] '" << msg << "' sent to ESP32" << std::endl;
    close(sock);
}

// UDP 수신 루프
void receive_loop() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in recv_addr{}, sender_addr{};
    socklen_t addr_len = sizeof(sender_addr);
    char buffer[128];

    recv_addr.sin_family = AF_INET;
    recv_addr.sin_addr.s_addr = INADDR_ANY;
    recv_addr.sin_port = htons(PORT);

    if (bind(sock, (sockaddr*)&recv_addr, sizeof(recv_addr)) < 0) {
        perror("bind failed");
        return;
    }

    std::cout << "[INFO] UDP Server started on port " << PORT << std::endl;

    while (running.load()) {
        int len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                           (sockaddr*)&sender_addr, &addr_len);
        if (len > 0) {
            buffer[len] = '\0';
            std::cout << "[RECV] " << buffer << std::endl;
        }
    }

    close(sock);
}

// 주기적으로 sync, fire 전송
void command_loop() {
    int sync_counter = 0;
    int fire_counter = 0;

    while (running.load()) {
        if (sync_counter >= 5) {
            send_message("sync");
            sync_counter = 0;
        }

        if (fire_counter >= 10) {
            send_message("fire");
            fire_counter = 0;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
        sync_counter++;
        fire_counter++;
    }
}

int main() {
    std::thread recv_thread(receive_loop);
    std::thread send_thread(command_loop);

    recv_thread.join();
    send_thread.join();

    return 0;
}
