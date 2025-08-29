#include "queue.h"
#include <stdlib.h>

Queue Queue_make() {
  Queue queue;
  queue.front = NULL;
  queue.rear = NULL;
  return queue;
}

Node *Node_make(Rotation data) {
  Node *node = (Node *)malloc(sizeof(Node));
  node->data = data;
  node->next = NULL;
  return node;
}

bool Queue_isEmpty(Queue *queue) { return queue->front == NULL; }

void Queue_add(Queue *queue, Rotation data) {
  Node *node = Node_make(data);
  if (Queue_isEmpty(queue)) {
    queue->front = node;
    queue->rear = node;
  } else {
    queue->rear->next = node;
    queue->rear = node;
  }
}

Rotation Queue_pop(Queue *queue) {
  if (Queue_isEmpty(queue)) {
    return -1;
  }
  Node *node = queue->front;
  Rotation data = node->data;
  queue->front = node->next;
  if (queue->front == NULL) {
    queue->rear = NULL;
  }
  free(node);
  return data;
}

Rotation Queue_peek(Queue *queue) {
  if (Queue_isEmpty(queue)) {
    return -1;
  }
  return queue->front->data;
}

void Queue_clear(Queue *queue) {
  while (!Queue_isEmpty(queue)) {
    Queue_pop(queue);
  }
}
