#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "networking.h"

int recv_data(int fd, void* buf, size_t nr) {
  
  size_t received = 0;

  while (received < nr) {
    
    int nbytes;

    nbytes = recv(fd, (char*)buf + received, nr - received, 0);
    if (nbytes < 0) {
      if (errno == EWOULDBLOCK ||
          errno == EAGAIN || errno == EINTR)
        continue;
      return nbytes;
    } else if (nbytes == 0) {
      return nbytes;
    }

    received += nbytes;

  }

  return received;

}


int send_data(int fd, void* buf, size_t nr) {
  
  size_t sent = 0;

  while (sent < nr) {
  
    int nbytes;

    nbytes = send(fd, (char*)buf + sent, nr - sent, 0);
    if (nbytes < 0) {
      if (errno == EWOULDBLOCK ||
          errno == EAGAIN || errno == EINTR)
        continue;
      return nbytes;
    } else if (nbytes == 0) {
      return nbytes;
    }

    sent += nbytes;

  }
  
  return sent;

}


packet_info_t* recv_packet(int fd) {

  int ret = -1;
  size_t pkt_sz, data_len;
  packet_info_t hdr, *pinfo;
  
  pkt_sz = sizeof(sizeof(packet_info_t));

  ret = recv_data(fd, &hdr, pkt_sz);
  if (ret <= 0)
    return NULL;

  pinfo = (packet_info_t*)malloc(hdr.size);
  if (pinfo == NULL)
    return NULL;
  
  memcpy(pinfo, &hdr, sizeof(hdr));

  data_len = pinfo->size - sizeof(packet_info_t);
  if(data_len > 0) {

    ret = recv_data(fd, pinfo->data, data_len);
    if (ret <= 0)
      return NULL;

  }

  return pinfo;

}


int send_packet(int fd, packet_info_t* pinfo) {

   return send_data(fd, pinfo, pinfo->size);

}


packet_info_t* new_packet(uint32_t cmd, void *data, size_t data_len) {

  size_t pkt_sz;
  packet_info_t *pinfo;

  pkt_sz = sizeof(packet_info_t) + data_len;

  pinfo = (packet_info_t*)malloc(pkt_sz);
  if(pinfo == NULL)
    return NULL;

  memset(pinfo, 0, pkt_sz);

  pinfo->size = pkt_sz;
  pinfo->opcode = cmd;
  if(data)
    memcpy(pinfo->data, data, data_len);

  return pinfo;

}


void* packet_data(packet_info_t* pinfo) {
  
  if (pinfo->size == sizeof(packet_info_t))
    return NULL;

  return pinfo->data;

}


tcp_socket_info* get_tcp_socket(const char *ip, uint32_t port) {

  int sock_fd;
  tcp_socket_info* si;

  si = (tcp_socket_info*)malloc(sizeof(tcp_socket_info));
  if(si == NULL)
    return NULL;

  if(ip == NULL)
    ip = "0.0.0.0";

  memset(si, 0, sizeof(tcp_socket_info));
  si->sock_addr.sin_family       = AF_INET;
  si->sock_addr.sin_port         = htons(port);
  si->sock_addr.sin_addr.s_addr  = inet_addr(ip);
  
  si->sock_len = sizeof(si->sock_addr);
  
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(sock_fd < 0) {
    free(si);
    return NULL;
  }

  si->sfd = sock_fd;

  return si;

}


/* 获取一个服务端socket文件描述符
 * ip     - 监听的ip地址
 * port   - 监听的端口号
 */

int get_tcp_server(const char *ip, uint32_t port) {
  
  int ret = -1, on = 1;
  tcp_socket_info* si;

  si = get_tcp_socket(ip, port);
  if(si == NULL)
    goto out;

  ret = setsockopt(si->sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  if(ret < 0)
    goto out;

  ret = bind(si->sfd, (struct sockaddr*)&si->sock_addr, si->sock_len);
  if(ret < 0)
    goto out;

  ret = listen(si->sfd, 10);
  if(ret < 0)
    goto out;

  ret = si->sfd;

out:
  free(si);
  return ret;

}


/* 获取一个客户端问价描述符
 * ip     - 连接的ip地址
 * port   - 连接的端口号
 */

int get_tcp_client(const char *ip, uint32_t port) {
  
  int ret = -1, delay = 1;
  tcp_socket_info* si;

  si = get_tcp_socket(ip, port);
  if(si == NULL)
    goto out;

  for (;;) {

    ret = connect(si->sfd, (struct sockaddr*)&si->sock_addr, si->sock_len);
    if(ret == 0)
      break;
    
    delay <<= 1;
    if(delay > 16)
      break;
    sleep(delay);

  }

  if(ret < 0)
    goto out;

  ret = si->sfd;

out:
  free(si);
  return ret;

}
