/*-------------------------------------------------------------------------
*
* Copyright (C) 2012-2015 Intel Corporation All Rights Reserved.
*
* The usage of this file is according to the terms and conditions of 
* the Intel (R) Integrated Sensor Solution FDK.
*
*-------------------------------------------------------------------------
*/

/**
 * @ingroup sys
 * @defgroup types types
 * See http://pubs.opengroup.org/onlinepubs/009696699/basedefs/sys/types.h.html for full documentation for this file.\n
*/

#ifndef _TYPES_H
#define _TYPES_H

/*
 * Both POSIX.1-2008 and C99 require stddef.h to define size_t.
 * POSIX.1-2008 also requires sys/types.h to define size_t. To avoid
 * double definition, stddef.h is included here.
 */
#include <stddef.h>

/** @addtogroup types
@{
*/


/* blkcnt_t */                       /* Used for file block counts                       @release ISH2*/
/* blksize_t */                      /* Used for block sizes                             @release ISH2*/
typedef long int clock_t;			 /**< Used for system times in clock ticks or CLOCKS_PER_SEC; see <time.h>                  @release ISH2*/
typedef int clockid_t;				 /**< Used for clock ID type in the clock and timer functions                                  @release ISH2*/
/* dev_t */                          /* Used for device IDs                              @release ISH2*/
/* gid_t */                          /* Used for group IDs                               @release ISH2*/
typedef int id_t;					 /**< Used as a general identifier; can be used to contain at least a pid_t, uid_t, or gid_t     */
/* ino_t */                          /* Used for file serial numbers                     @release ISH2*/
typedef unsigned int mode_t;	     /**< Used for some file  attributes                   @release ISH2*/
/* nlink_t */                        /* Used for link counts                             @release ISH2*/
typedef long int off_t;				 /**< Used for file sizes                              @release ISH2*/
/* pid_t */                          /* Used for process IDs and process group IDs       @release ISH2*/
typedef int pid_t;
/* size_t */                         /* Used for sizes of objects, comes from stddef.h   @release ISH2*/
typedef int ssize_t;				 /**< Used for a count of bytes or an error indication @release ISH2*/
typedef long int time_t;			 /**< Used for time in seconds                         @release ISH2*/
typedef void *timer_t;				 /**< Used for timer ID returned by timer_create()     @release ISH2*/
/* uid_t */                          /* Used for user IDs                                @release ISH2*/
typedef int uid_t;					 /**< @release ISH2*/

typedef int suseconds_t;

/* 
 * Types related to multi-threading support. Their internal structure
 * is not disclosed to applications.
 */
#define __SIZE_OF_pthread_attr_t        12      /**< @release ISH2*/
#define __SIZE_OF_pthread_barrier_t     12		/**< @release ISH2*/
#define __SIZE_OF_pthread_barrierattr_t 4		/**< @release ISH2*/
#define __SIZE_OF_pthread_cond_t        8		/**< @release ISH2*/
#define __SIZE_OF_pthread_condattr_t    4		/**< @release ISH2*/
#define __SIZE_OF_pthread_mutex_t       4		/**< @release ISH2*/
#define __SIZE_OF_pthread_mutexattr_t   4		/**< @release ISH2*/
#define __SIZE_OF_pthread_rwlock_t      20		/**< @release ISH2*/
#define __SIZE_OF_pthread_rwlockattr_t  4		/**< @release ISH2*/

typedef struct {
      int internal[__SIZE_OF_pthread_attr_t / sizeof(int)];
} pthread_attr_t;				/**< Used to identify a thread attribute object       @release ISH2*/

typedef struct {
      int internal[__SIZE_OF_pthread_barrier_t / sizeof(int)];
} pthread_barrier_t;		     /**< Used to identify a barrier                       @release ISH2*/

typedef struct {
      int internal[__SIZE_OF_pthread_barrierattr_t / sizeof(int)];
} pthread_barrierattr_t;	     /**< Used to define a barrier attributes object       @release ISH2*/

typedef struct {
      int internal[__SIZE_OF_pthread_cond_t / sizeof(int)];
} pthread_cond_t;				/**< Used for condition variables                     @release ISH2*/

typedef struct {
      int internal[__SIZE_OF_pthread_condattr_t / sizeof(int)];
} pthread_condattr_t;		     /**< Used to identify a condition attribute object    @release ISH2*/

typedef unsigned pthread_key_t;	 /**< Used for thread-specific data keys               @release ISH2*/

typedef struct {
      int internal[__SIZE_OF_pthread_mutex_t / sizeof(int)];
} pthread_mutex_t;				 /**< Used for mutexes                                 @release ISH2*/

typedef struct {
      int internal[__SIZE_OF_pthread_mutexattr_t / sizeof(int)];
} pthread_mutexattr_t;		     /**< Used to identify a mutex attribute object        @release ISH2*/

typedef int pthread_once_t;	     /**< Used for dynamic package initialization          @release ISH2*/

typedef struct {
      int internal[__SIZE_OF_pthread_rwlock_t / sizeof(int)];
} pthread_rwlock_t;				 /**< Used for read-write locks                        @release ISH2*/

typedef struct {
      int internal[__SIZE_OF_pthread_rwlockattr_t / sizeof(int)];
} pthread_rwlockattr_t;		     /**< Used for read-write lock attributes              @release ISH2*/

/* pthread_spinlock_t */         /* Used to identify a spin lock                     @release ISH2*/

typedef unsigned long int pthread_t; /**< Used to identify a thread                        @release ISH2*/


/** @} */

#endif /* _TYPES_H */
