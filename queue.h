#ifndef QUEUE_H
#define QUEUE_H

#include "cube.h"

typedef struct Node {
  Rotation data;
  struct Node *next;
} Node;

typedef struct Queue {
  Node *front;
  Node *rear;
} Queue;

Queue Queue_make();

Node *Node_make(Rotation data);

bool Queue_isEmpty(Queue *queue);

void Queue_add(Queue *queue, Rotation data);

Rotation Queue_pop(Queue *queue);

Rotation Queue_peek(Queue *queue);

void Queue_clear(Queue *queue);

#endif // !QUEUE_H
