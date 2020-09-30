#ifndef _SYNCH_H
#define _SYNCH_H

/* includes */
#include "list.h"
#include "../kernel/thread.h"

/* defines */

/* structs */

struct semaphore {
    int val;
    list_t waiters;
};

struct lock {
    struct semaphore;
    struct thread *owner;
};

/* typedefs */

/* functions */


#endif