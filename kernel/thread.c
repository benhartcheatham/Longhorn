/* May want to make the stack limited to a page or two and put them at
  a specified interval like in linux. This would allow getting rid of
  the current pointer and just doing some quick math with esp to get
  the running thread.
  
  For synchronization, I think synchronizing around the to thread lists
  is correct, but to get around schedule blocking, it should try to acquire the 
  semaphore/lock and then schedule the thread that holds the lock if it can't
  
  Priority scehduling is also something I will want to implement now */

/* includes */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <synch.h>
#include <kerrors.h>
#include "thread.h"
#include "proc.h"
#include "kalloc.h"
#include "port_io.h"

/* defines */
#define MAX_THREAD_TICKS 8

/* globals */
static struct list ready_threads;
static struct list blocked_threads;
static struct list dying_threads;
static struct thread *idle_t;
static bool tids[MAX_TID];
static uint8_t thread_ticks = 0;

/* structs */
struct thread_func_frame {
    void *eip;
    thread_function *function;
    void *aux;
};

struct tail_frame {
    void (*eip) (void);
};

struct stack_frame {
    uint32_t ebp, ebx, edi, esi;
    void (*eip) (void);
};

/* prototypes */
static uint32_t allocate_tid();
static void thread_execute(thread_function *func, void *aux);
static void schedule();
extern void first_switch_entry();
static void idle(void *aux);
// static size_t num_threads();
// static void print_ready();

/* external functions */
extern void switch_threads(struct thread *current_thread, struct thread *next_thread);

/* functions */

/* initialization functions */

/** initializes the threading subsystem
 * 
 * @param init: pointer to the initial process
 */
void init_threads(struct process *init) {
    list_init(&ready_threads);
    list_init(&blocked_threads);
    list_init(&dying_threads);

    thread_create(0, "idle", init, 0, idle, NULL);
    idle_t = init->threads[0];
    THREAD_CUR()->state = THREAD_BLOCKED;
    
    // idle_t->state = THREAD_BLOCKED;
    // list_delete(&ready_threads, &idle_t->node);
    // list_insert(&blocked_threads, &idle_t->node);
}

/* thread state functions */

/** creates a thread under the given process
 * 
 * @param priority: unused
 * @param name: name of thread
 * @param proc: process to create this thread under
 * @param sthread: pointer to store thread info into (should be in proc)
 * @param func: function thread should run when scheduled
 * @param aux: func parameters and any extra info
 * 
 * @return tid of the created thread, -1 if creation failed
 */
int thread_create(uint8_t priority, char *name, struct process *proc, uint32_t child_num, thread_function func, void *aux) {
    uint8_t *s = (uint8_t *) palloc_mult(STACK_SIZE / PG_SIZE);

    if (s == NULL)
        return -1;
    
    // setup the thread struct at the bottom of the page (lowest addr)
    struct thread_info *ti = (struct thread_info *) s;

    // if the max amount of threads on the system is already met don't allow creation
    if (ti->t.tid == MAX_TID + 1) {
        kfree((void *) s);
        return -1;
    }

    ti->t.tid = allocate_tid();
    ti->t.state = THREAD_READY;
    sprintf(ti->t.name, "%s", name);
    ti->t.priority = priority;
    ti->t.pid = proc->pid;
    ti->t.child_num = child_num;
    
    // add a pointer to the parent process after thread struct
    ti->p = proc;
    proc->threads[child_num] = &ti->t;

    s += STACK_SIZE;

    // setup arguments thread_execute
    s -= sizeof(struct thread_func_frame);
    struct thread_func_frame *f = (struct thread_func_frame *) s;
    f->eip = NULL;
    f->function = func;
    f->aux = (void *) aux;

    // setup to call thread_execute
    s -= sizeof(struct tail_frame);
    struct tail_frame *tf = (struct tail_frame *) s;
    tf->eip = (void (*) (void)) thread_execute;

    // setup for the first switch of a thread
    s -= sizeof(struct stack_frame);
    struct stack_frame *sf = (struct stack_frame *) s;
    sf->eip = first_switch_entry;
    sf->ebp = 0;

    ti->t.esp = (uint32_t *) s;

    ti->t.state = THREAD_READY;

    list_init(&ti->t.waiters);
    ti->t.wait_code = 0;

    ti->t.magic = THREAD_MAGIC;
    
    disable_interrupts();
    list_insert(&ready_threads, &ti->t.node);
    enable_interrupts();

    return ti->t.tid;
}

/** blocks a thread
 * 
 * @param thread: thread to block
 */
void thread_block(struct thread *thread) {
    disable_interrupts();

    struct list_node *node = list_delete(&ready_threads, &thread->node);

    // running threads aren't in the ready list, so we
    // need to account for that case
    if (node == NULL && thread->state != THREAD_RUNNING) {
        enable_interrupts();
        return;
    }
    
    thread->state = THREAD_BLOCKED;
    list_insert(&blocked_threads, node);

    enable_interrupts();

    schedule();
}

/** unblocks a thread and sets it to ready to run 
 * 
 * @param thread: thread to unblock
 */
void thread_unblock(struct thread *thread) {
    disable_interrupts();

    struct list_node *node = list_delete(&blocked_threads, &thread->node);

    if (node == NULL) {
        enable_interrupts();
        return;
    }
    
    thread->state = THREAD_READY;
    list_insert(&ready_threads, node);
    enable_interrupts();
}

/** function called at the end of the current thread's lifecycle 
 * 
 * @param ret: pointer to where to store return code of thread
 */
void thread_exit(int *ret) {
    struct thread *t = THREAD_CUR();
    disable_interrupts();

    list_delete(&ready_threads, &t->node);
    t->state = THREAD_DYING;


    if (ret != NULL)
        *ret = THREAD_KILL_SUCC;
    
    thread_notify(true, THREAD_KILL_SUCC);

    thread_yield();
}

/** wait on a thread to finish
 * this function will block until t notifies
 * the calling thread or t dies
 * 
 * @param t: thread to wait on
 * 
 * @return -THREAD_FAILURE when wait isn't successful,
 *         return code of t on success
 */
int thread_join(struct thread *t) {
    if (t == NULL)
        return -THREAD_FAILURE;

    list_insert(&t->waiters, &THREAD_CUR()->node);
    thread_block(THREAD_CUR());

    return THREAD_CUR()->wait_code;
}

/** notifes either the first thread waiting on this thread
 * or all threads waiting on this thread to wake
 * 
 * @param all: whether to notify the first or all threads
 * @param ret: code to return to waiting thread(s)
 * 
 * @return -1 on failure, 0 on success
 */
int thread_notify(bool all, int ret) {
    list_t *waiters = &THREAD_CUR()->waiters;
    list_node_t *node = list_pop(waiters);

    if (node == NULL)
        return -1;
    
    struct thread *wait_t;
    do {
        wait_t = LIST_ENTRY(node, struct thread, node);
        wait_t->wait_code = ret;
        thread_unblock(wait_t);
        node = list_pop(waiters);
    } while (all && node != NULL);

    return 0;
}

/** cleans up thread t
 * t is assumed to be in no lists and safe to deallocate
 * 
 * @param t: thread to cleanup
 * 
 * @return -1 if cleanup was unsuccessful, 0 otherwise
 */
static int thread_cleanup(struct thread *t) {
    if (t == NULL || t->state != THREAD_DYING)
        return -1;
    
    struct process *t_proc = get_thread_proc(t);
    t_proc->threads[t->child_num] = NULL;
    t_proc->num_live_threads--;

    if (t_proc->num_live_threads == 0) {
        proc_cleanup(t_proc);
        pfree(t_proc);
    }

    pfree_mult((void *) ((uint32_t) t->esp / STACK_SIZE), STACK_SIZE / PG_SIZE);
    return 0;
}

/** kills thread thread
 * If thread is currently running, this function call is equivalent
 * to calling thread_exit()
 * THIS DOESN'T RELEASE THE LOCKS HELD BY THE THREAD, NEEDS TO BE UPDATED 
 * 
 * @param thread: thread to kill
 * 
 * @return 0 if the thread is not running and thread is killed successfully,
 *         -1 if the thread is not running and is not killed successfully,
 *          this function will not return when thread is running
 */
int thread_kill(struct thread *thread) {
    if (thread == NULL)
        return -1;

    disable_interrupts();
    if (thread->state == THREAD_RUNNING) {
        thread_exit(NULL);
    }

    if (thread->state == THREAD_READY) {
        thread->state = THREAD_DYING;
    
        struct list_node *node = list_delete(&ready_threads, &thread->node);
        if (node == NULL) {
            enable_interrupts();
            return -1;
        }
    }

    if (thread->state == THREAD_BLOCKED) {
        thread->state = THREAD_DYING;

        struct list_node *node = list_delete(&blocked_threads, &thread->node);
        if (node == NULL) {
            enable_interrupts();
            return -1;
        }
    }

    thread_notify(true, -1);
    // list_insert(&dying_threads, node);
    thread_cleanup(thread);
    enable_interrupts();

    enable_interrupts();
    return 0;
}

/* scheduling functions */

/** yields the remainder of this thread's time slice */
void thread_yield() {
    schedule();
}

/** interrupt handler for the timer interrupt, also starts scheduling periodically
 * 
 * @param r: unused
 */
void timer_interrupt_handler(struct register_frame *r __attribute__ ((unused))) {
    thread_ticks++;
    THREAD_CUR()->ticks++;

    if (thread_ticks % MAX_THREAD_TICKS == 0) {
        schedule();
    }
}

/** finishes up the scheduling process and updates thread state */
void finish_schedule() {
    //finish the part after we switch threads in schedule()

    //set current thread to running
    struct thread *cur = THREAD_CUR();
    cur->state = THREAD_RUNNING;
    proc_set_active_thread(PROC_CUR(), cur->child_num);
    
    if (list_isEmpty(&dying_threads) == false) {
        disable_interrupts();
        struct thread *dying = LIST_ENTRY(list_pop(&dying_threads), struct thread, node);
        if (dying != NULL)
            thread_cleanup(dying);
    }

    enable_interrupts();
}

/* static functions */

/** executes the function the thread is created to do and kills the thread when done
 * 
 * @param func: fucntion for thread to run, given at creation
 * @param aux: parameters of func
 */
static void thread_execute(thread_function func, void *aux) {
    //have to reenable interrupts since there isn't a guaruntee we returned to the irq handler
    asm volatile("sti");
    func(aux);
    thread_exit(NULL);
}

/** schedules threads */
static void schedule() {
    disable_interrupts();

    list_node_t *next = list_size(&ready_threads) == 1 ? ready_threads.head.next : list_pop(&ready_threads);
    struct thread *next_thread = NULL;
    struct thread *current = THREAD_CUR();

    // if we have no ready threads, we schedule the idle thread
    // otherwise we just pull one of the top
    if (next == NULL) {
        next_thread = idle_t;
        idle_t->state = THREAD_READY;
    } else {
        next_thread = LIST_ENTRY(next, struct thread, node);
    }

    if (current->state == THREAD_RUNNING)
        current->state = THREAD_READY;

    if (current->state == THREAD_DYING)
        list_insert(&dying_threads, &current->node);
    
    if (current->state == THREAD_READY)
        list_insert_end(&ready_threads.tail, &current->node);

    if (current == next_thread && current->state == THREAD_DYING)
        thread_exit(NULL);

    // don't need to do any scheduling, just restore state and return
    if (current == next_thread || next_thread->state != THREAD_READY) {
        current->state = THREAD_RUNNING;
        enable_interrupts();
        return;
    }

    switch_threads(current, next_thread);

    finish_schedule();
}

/** function that the init thread runs after interrupts are enabled
 * 
 * @param aux: unused
 */
static void idle(void *aux __attribute__ ((unused))) {
    while (1) {
        thread_block(THREAD_CUR());
    };
}

/** allocates a thread id for when a thread is being created
 * 
 * @return tid of new thread
 */
static uint32_t allocate_tid() {
    int i;
    for (i = 0; i < MAX_TID; i++)
        if (tids[i] == false) {
            tids[i] = true;
            return i;
        }
    
    return MAX_TID + 1;
}

/* testing functions */

// static size_t num_threads() {
//     return list_size(&ready_threads) + list_size(&blocked_threads);
// }

// static void print_ready() {
//     struct list_node *node = ready_threads.head.next;

//     uint32_t i = 0;
//     while (node != &ready_threads.tail && node != NULL) {
//         struct thread *t = LIST_ENTRY(node, struct thread, node);
//         kprintf("name: %s state: %d num in list: %d\n", t->name, t->state, i);
//         i++;
//         node = node->next;
//     }
//     kprintf("\n");
// }