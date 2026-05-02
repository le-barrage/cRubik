#include "queue.h"

#include <stdlib.h>

struct queue_node
{
  Rotation data;
  struct queue_node *next;
};

struct queue
{
  struct queue_node *front;
  struct queue_node *rear;
};

queue_t *
queue_create (void)
{
  queue_t *q = malloc (sizeof (*q));
  if (q == NULL)
    return NULL;
  q->front = NULL;
  q->rear = NULL;
  return q;
}

void
queue_destroy (queue_t *q)
{
  if (q == NULL)
    return;
  queue_clear (q);
  free (q);
}

bool
queue_is_empty (const queue_t *q)
{
  return q->front == NULL;
}

queue_status_t
queue_push (queue_t *q, Rotation value)
{
  struct queue_node *node = malloc (sizeof (*node));
  if (node == NULL)
    return QUEUE_OOM;

  node->data = value;
  node->next = NULL;

  if (q->rear == NULL)
    q->front = node;
  else
    q->rear->next = node;
  q->rear = node;

  return QUEUE_OK;
}

queue_status_t
queue_pop (queue_t *q, Rotation *out)
{
  if (q->front == NULL)
    return QUEUE_EMPTY;

  struct queue_node *node = q->front;
  *out = node->data;
  q->front = node->next;
  if (q->front == NULL)
    q->rear = NULL;
  free (node);

  return QUEUE_OK;
}

queue_status_t
queue_peek (const queue_t *q, Rotation *out)
{
  if (q->front == NULL)
    return QUEUE_EMPTY;
  *out = q->front->data;
  return QUEUE_OK;
}

void
queue_clear (queue_t *q)
{
  Rotation discard;
  while (queue_pop (q, &discard) == QUEUE_OK)
    ;
}
