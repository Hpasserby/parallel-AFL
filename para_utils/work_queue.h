#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <pthread.h>
#include <stdint.h>


typedef struct queue_item {

  void* task;
  struct queue_item *next;

} queue_item_t;


typedef struct queue {
    
  volatile size_t item_num;
  queue_item_t *head;
  queue_item_t *tail;
  int nonblock;
  uint32_t timeout;

  pthread_mutex_t push_mutex;
  pthread_mutex_t pop_mutex;

} queue_t;


queue_t* new_queue(int nonblock, uint32_t timeout);
int push_queue(queue_t *q, void *task);
void* pop_queue(queue_t *q);
void delete_queue(queue_t *q);


#endif
