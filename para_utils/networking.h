#ifndef NETWORKING_H
#define NETWORKING_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

/* COMMAND in packet */
#define PUT_SEED    0   /* 上传新种子 */
#define GET_TASK    1   /* 请求fuzz任务 */
#define GET_SEED    2   /* 若本地未缓存种子 则请求下载 */
#define SYNC_BITMAP 3   /* 请求同步位图 */
#define SYNC_EXTRAS 4   /* 同步extras */

/* 策略选择 */
#define M_BITFLIP   5
#define M_ARITH     4
#define M_INTEREST  3
#define M_EXTRAS    2
#define M_HAVOC     1
#define M_SPLICE    0


typedef struct seed_info {

  uint32_t size;            /* 总长度 */
  uint32_t flag;            /* 策略选择 */ 
  uint32_t seed_len;        /* 种子长度 */
  uint8_t content[0];

} seed_info_t;


typedef struct packet_info {

  uint32_t size;            /* 总长度 */
  uint32_t opcode;          /* 操作码 */
  uint8_t data[0]; 

} packet_info_t;


typedef struct tcp_socket_info {
  
  int sfd;
  socklen_t sock_len;
  struct sockaddr_in sock_addr;

}tcp_socket_info;

tcp_socket_info* get_tcp_socket(const char *ip, uint32_t port);
int get_tcp_server(uint32_t port);
int get_tcp_client(const char *ip, uint32_t port);

packet_info_t* new_packet(uint32_t cmd, void *data, size_t data_len);
packet_info_t* recv_packet(int fd);
int send_packet(int fd, packet_info_t* pinfo);
void* packet_data(packet_info_t *pinfo);

#endif
