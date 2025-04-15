#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"  // FreeRTOS API

#define UDP_SERVER_PORT    7

#define MAX_CLIENTS 10

typedef struct {
    ip_addr_t ip;
    u16_t port;
} client_info_t;

static client_info_t clients[MAX_CLIENTS];
static int client_count = 0;

// ★ 추가: 센서 A/B 데이터 저장용
static int gen1data = 0, tm1data = 0;
static int gen2data = 0, tm2data = 0;

void parse_and_store_data(const char* data_str, const ip_addr_t* addr, u16_t port)
{
    int gen_val = 0, time_val = 0;
    sscanf(data_str, "%d(%d)", &gen_val, &time_val);

    if (client_count >= 2) {
        client_info_t* a = &clients[client_count - 1];
        client_info_t* b = &clients[client_count - 2];

        if (ip_addr_cmp(addr, &a->ip) && port == a->port) {
            gen1data = gen_val;
            tm1data = time_val;
            printf("[Sensor A] %d(%d)\n", gen1data, tm1data);
        }
        else if (ip_addr_cmp(addr, &b->ip) && port == b->port) {
            gen2data = gen_val;
            tm2data = time_val;
            printf("[Sensor B] %d(%d)\n", gen2data, tm2data);
        } else {
            printf("[Ignored] Not recent A or B\n");
        }
    } else {
        printf("[Skipped] Not enough clients\n");
    }
}


void add_client(const ip_addr_t *addr, u16_t port) {
    for (int i = 0; i < client_count; i++) {
        if (ip_addr_cmp(&clients[i].ip, addr) && clients[i].port == port) {
            return;  // 이미 존재함
        }
    }

    if (client_count < MAX_CLIENTS) {
        ip_addr_set(&clients[client_count].ip, addr);
        clients[client_count].port = port;
        client_count++;
    }
}

void send_sync_to_all_clients()
{
    for (int i = 0; i < client_count; i++) {
        struct udp_pcb* upcb = udp_new();
        if (!upcb) continue;

        struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, strlen("sync"), PBUF_RAM);
        if (!p) {
            udp_remove(upcb);
            continue;
        }

        memcpy(p->payload, "sync", strlen("sync"));
        udp_sendto(upcb, p, &clients[i].ip, clients[i].port);
        pbuf_free(p);
        udp_remove(upcb);
    }
}

void udp_echoserver_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
//    printf("udp_echoserver_receive_callback\n");

    if (p != NULL) {
//        printf("Received UDP packet from %s:%d\n", ipaddr_ntoa(addr), port);
        printf("Data (%d bytes): %.*s\n", p->len, p->len, (char *)p->payload);

        add_client(addr, port);  // ★ 클라이언트 추가

        // ★ 수신 문자열 복사 및 파싱 호출
		char data_buf[32] = {0};
		strncpy(data_buf, (char *)p->payload, p->len < sizeof(data_buf)-1 ? p->len : sizeof(data_buf)-1);
		parse_and_store_data(data_buf, addr, port);

        udp_sendto(upcb, p, addr, port);
        pbuf_free(p);
    }
}



void udp_sync_sender_task(void *arg)
{
    while (1) {
        send_sync_to_all_clients();  // 모든 클라이언트에게 전송
        osDelay(3000);  // 3초 delay
    }
}

void udp_echoserver_init(void)
{
   struct udp_pcb *upcb = udp_new();
   if (upcb)
   {
      if (udp_bind(upcb, IP_ADDR_ANY, UDP_SERVER_PORT) == ERR_OK)
      {
         udp_recv(upcb, udp_echoserver_receive_callback, NULL);
      }
      else
      {
         udp_remove(upcb);
      }
   }

   // "sync"를 주기적으로 보내는 태스크 생성
   osThreadDef(udpSyncSender, udp_sync_sender_task, osPriorityLow, 0, 256);
   osThreadCreate(osThread(udpSyncSender), NULL);
}
