/* File that defines error codes in the kernel
 *
 * Error codes should always be defined to be an int type
 * and be positive. All success codes should be defined as
 * 0, and it is ok for error codes in different sections
 * to be defined to the same number, however no failure/error
 * code (other than ones that denote success) should be defined as the
 * same number
 */

#ifndef _ERROR_H
#define _ERROR_H

/* defines */

/* general error numbers */
#define SUCCESS 0
#define FAILURE 1

/* mem allocation errors */
#define MEM_ALLOC_SUCC 0
#define MEM_ALLOC_FAIL 1
#define MEM_FREE_SUCC 0
#define MEM_FREE_FAIL 2

/* process errors */
#define PROC_CREATE_SUCC 0
#define PROC_CREATE_FAIL 1
#define PROC_KILL_SUCC 0
#define PROC_KILL_FAIL 2
#define PROC_SUCCESS 0      // these two are for general process errrors
#define PROC_FAILURE 3      // that are one-offs or haven't been made yet

/* thread errors */
#define THREAD_SUCCESS 0
#define THREAD_FAILURE 1
#define THREAD_CREATE_FAIL 2
#define THREAD_KILL_SUCC 0
#define THREAD_KILL_FAIL 3


/* synchronization errors */
#define LOCK_INIT_FAIL 1
#define LOCK_INIT_SUCC 0
#define LOCK_ACQ_FAIL 2
#define LOCK_ACQ_SUCC 0
#define LOCK_REL_FAIL 3
#define LOCK_REL_SUCC 0

/* terminal errors */
#define TERM_SUCC 0
#define TERM_INIT_FAIL 1
#define TERM_WRITE_FAIL 2
#define TERM_OUT_FAIL 3
#define TERM_IN_FAIL 4

/* line discipline errors */
#define LINE_SUCC 0
#define LINE_INIT_FAIL 1
#define LINE_IN_FAIL 2
#define LINE_OUT_FAIL 3

#endif