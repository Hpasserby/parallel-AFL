#ifndef NETWORKING_H
#define NETWORKING_H

#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

/* COMMAND in packet */
#define PUT_SEED    0   /* 上传新种子 */
#define GET_TASK    1   /* 请求fuzz任务 */
#define GET_SEED    2   /* 若本地未缓存种子 则请求下载 */
#define PUT_STATUS  3   /* 节点上传状态信息 */
#define SYNC_BITMAP 4   /* 请求同步位图 */
#define CHECK_DUP   5   /* 实验用 查询种子是否重复 */
#define INIT_INFO   6   /* 节点初始化 */
#define SYNC_EXTRAS 7   /* 同步auto extras */

#ifdef DUP_TEST

typedef struct exec_info {

  uint8_t mut_stage;
  uint8_t seed_hash[33];

} exec_info_t;

#endif

typedef struct extras_pack {

  uint64_t size;            /* 整个结构体大小 */
  uint64_t timestamp;       /* 时间戳 */
  uint8_t extras[];         /* extra_info列表 */

} extras_pack_t;

typedef struct extra_info {

  uint64_t length;          /* 该extras的长度 */ 
  uint64_t next_off;        /* 下一条extras的偏移 */
  uint8_t  data[];          /* extras内容 */

} extra_info_t;


typedef struct node_status {

  uint64_t delta_execs;     /* 执行次数增量 */
  struct {
    uint64_t size;          /* 仅data的长度 */
    uint64_t  data[0]; 
  }; 

} node_status_t;


typedef struct init_status {

  uint32_t havoc_div;
  uint32_t exec_tmout;

} init_status_t;


typedef struct seed_info {

  uint32_t size;            /* 总长度 */
  uint32_t flag;            /* 策略选择 */ 
  uint32_t seed_len;        /* 种子长度 */
  uint64_t handicap;
  uint64_t depth;
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

int recv_data(int fd, void* buf, size_t nr);
int send_data(int fd, void* buf, size_t nr);

packet_info_t* new_packet(uint32_t cmd, void *data, size_t data_len);
packet_info_t* recv_packet(int fd);
int send_packet(int fd, packet_info_t* pinfo);
void* packet_data(packet_info_t *pinfo);

#endif
