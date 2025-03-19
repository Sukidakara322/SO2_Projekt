#ifndef COMPATPTHREAD_H
#define COMPATPTHREAD_H

// Tell the pthread header we already have a definition of timespec
#ifndef _TIMESPEC_DEFINED
#define _TIMESPEC_DEFINED
#endif

#include <pthread.h>

#endif