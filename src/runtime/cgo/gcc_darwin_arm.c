// Copyright 2014 The Go Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <string.h> /* for strerror */
#include <pthread.h>
#include <signal.h>
#include "libcgo.h"

#define magic1 (0xe696c4f4U)

// The first key should be at 258.
#define PTHREAD_KEYS_MAX 512

// inittls allocates a thread-local storage slot for g.
//
// It finds the first available slot using pthread_key_create and uses
// it as the offset value for runtime.tlsg.
static void
inittls(void **tlsg, void **tlsbase)
{
	pthread_key_t k;
	int i, err;

	err = pthread_key_create(&k, nil);
	if(err != 0) {
		fprintf(stderr, "runtime/cgo: pthread_key_create failed: %d\n", err);
		abort();
	}
	fprintf(stderr, "k = %d\n", (int)k); // debug
	pthread_setspecific(k, (void*)magic1);
	for (i=0; i<PTHREAD_KEYS_MAX; i++) {
		if (*(tlsbase+i) == (void*)magic1) {
			*tlsg = (void*)(i*sizeof(void *));
			pthread_setspecific(k, 0);
			return;
		}
	}
	fprintf(stderr, "runtime/cgo: could not find pthread key.\n");
	abort();
}

static void *threadentry(void*);
void (*setg_gcc)(void*);

void
_cgo_sys_thread_start(ThreadStart *ts)
{
	pthread_attr_t attr;
	sigset_t ign, oset;
	pthread_t p;
	size_t size;
	int err;

	sigfillset(&ign);
	pthread_sigmask(SIG_SETMASK, &ign, &oset);

	pthread_attr_init(&attr);
	size = 0;
	pthread_attr_getstacksize(&attr, &size);
	// Leave stacklo=0 and set stackhi=size; mstack will do the rest.
	ts->g->stackhi = size;
	err = pthread_create(&p, &attr, threadentry, ts);

	pthread_sigmask(SIG_SETMASK, &oset, nil);

	if (err != 0) {
		fprintf(stderr, "runtime/cgo: pthread_create failed: %s\n", strerror(err));
		abort();
	}
}

extern void crosscall_arm1(void (*fn)(void), void (*setg_gcc)(void*), void *g);
static void*
threadentry(void *v)
{
	ThreadStart ts;

	ts = *(ThreadStart*)v;
	free(v);

	crosscall_arm1(ts.fn, setg_gcc, (void*)ts.g);
	return nil;
}

void
x_cgo_init(G *g, void (*setg)(void*), void **tlsg, void **tlsbase)
{
	pthread_attr_t attr;
	size_t size;

	setg_gcc = setg;
	pthread_attr_init(&attr);
	pthread_attr_getstacksize(&attr, &size);
	g->stacklo = (uintptr)&attr - size + 4096;
	pthread_attr_destroy(&attr);

	inittls(tlsg, tlsbase);
}
