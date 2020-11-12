#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "work_queue.h"


/* nonblock
 *    - 0 当队列为空 pop函数阻塞
 *    - 1 当队列为空 pop函数返回负值
 *
 * timeout
 *    仅阻塞模式有效 仅针对pop函数 
 *    - 0   永久阻塞
 *    - 非0 阻塞超时时间（单位秒）
 */

queue_t* new_queue(int nonblock, uint32_t timeout) {
  
  queue_t *que;

  que = (queue_t*)malloc(sizeof(queue_t));
  if(que == NULL)
    return NULL;

  que->nonblock = nonblock;
  que->timeout = nonblock ? 0 : timeout;

  que->item_num = 0;
  que->head = que->tail = NULL;
  pthread_mutex_init(&que->push_mutex, NULL);
  pthread_mutex_init(&que->pop_mutex, NULL);

  if(!nonblock)
    pthread_mutex_lock(&que->pop_mutex);

  return que;

}

int push_queue(queue_t *que, void *task) {

  size_t item_num;
  queue_item_t* item;

  if(que == NULL)
    return -1;

  item = (queue_item_t*)malloc(sizeof(queue_item_t));
  if(item == NULL)
    return -1;

  item->task = task;
  item->next = NULL;

  pthread_mutex_lock(&que->push_mutex);

  item_num = que->item_num;

  if(item_num == 0) 
    que->head = que->tail = item;
  else {
    que->tail->next = item;
    que->tail = item;
  }
  
  ++que->item_num;

  pthread_mutex_unlock(&que->push_mutex);
  if(item_num == 0 && !que->nonblock)
    pthread_mutex_unlock(&que->pop_mutex);

  return 0;

}

void* pop_queue(queue_t *que) {
  
  int ret;
  size_t item_num, curr_num;
  void *task = NULL;
  queue_item_t *item;
  struct timespec tout;

  if(que == NULL)
    return NULL;

  if(que->timeout) {
   
    clock_gettime(CLOCK_REALTIME, &tout);
    tout.tv_sec += que->timeout;
    ret = pthread_mutex_timedlock(&que->pop_mutex, &tout);
    if(ret)
      return task;

  } else {
    pthread_mutex_lock(&que->pop_mutex);
  }
  
  item_num = que->item_num;
  if(item_num == 0)
    goto out;
  if(item_num == 1)
    pthread_mutex_lock(&que->push_mutex);
  
  item = que->head;
  if(que->item_num == 1)
    que->head = que->tail = NULL;
  else
    que->head = item->next;

  --que->item_num;
  curr_num = que->item_num;

  task = item->task;
  free(item);

  if(item_num == 1) {
    
    pthread_mutex_unlock(&que->push_mutex);
    if(curr_num == 0 && !que->nonblock)
      goto out_block; 

  }

out:
  pthread_mutex_unlock(&que->pop_mutex);
out_block:
  return task;

}


void delete_queue(queue_t *que) {
  
  queue_item_t *item, *next_item;

  pthread_mutex_lock(&que->push_mutex);

  item = que->head;
  while(item) {

    next_item = item->next;
    free(item);
    item = next_item;

  }

  free(que);

}
