#ifndef QUEUE_H
#define QUEUE_H

#include "cube.h"
#include <stdbool.h>

/* Opaque FIFO queue of move_t values. Construct with queue_create, free
 * with queue_destroy. */
typedef struct queue queue_t;

typedef enum
{
  QUEUE_OK = 0,
  QUEUE_EMPTY,
  QUEUE_OOM,
} queue_status_t;

/* Ownership: queue_create returns a heap-allocated queue. The caller must
 * release it with queue_destroy. Returns NULL on allocation failure. */
queue_t *queue_create (void);

/* NULL-safe. Frees every remaining node and the queue itself. */
void queue_destroy (queue_t *q);

/* Frees every remaining node. The queue stays valid (empty) and reusable. */
void queue_clear (queue_t *q);

bool queue_is_empty (const queue_t *q);

queue_status_t queue_push (queue_t *q, move_t value);
queue_status_t queue_pop (queue_t *q, move_t *out);
queue_status_t queue_peek (const queue_t *q, move_t *out);

#endif // QUEUE_H
