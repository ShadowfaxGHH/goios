// Copyright 2015 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "go_asm.h"
#include "go_tls.h"
#include "funcdata.h"
#include "textflag.h"
#include "tls_arm64.h"

// TODO(minux): The runtime.tlsg name is too magic, so I have to
// avoid it and choose runtime.tls_g.

TEXT runtime·load_g(SB),NOSPLIT,$0
	MOVB	runtime·iscgo(SB), R0
	CMP	$0, R0
	BEQ	nocgo

	MRS	TPIDR, R0
	// Darwin sometimes have unaligned pointers
	AND	$0xfffffffffffffff8, R0

#ifdef TLSG_IS_VARIABLE
	MOV	runtime·tls_g(SB), R27
#else
	// TODO(minux): use real TLS relocation, instead of hard-code for Linux
	MOV	$0x10, R27
#endif
	ADD	R27, R0 // TODO: use (R27)(R0)
	MOV	(R0), g

nocgo:
	RETURN

TEXT runtime·save_g(SB),NOSPLIT,$0
	MOVB	runtime·iscgo(SB), R0
	CMP	$0, R0
	BEQ	nocgo

	MRS	TPIDR, R0
	// Darwin sometimes have unaligned pointers
	AND	$0xfffffffffffffff8, R0

#ifdef TLSG_IS_VARIABLE
	MOV	runtime·tls_g(SB), R27
#else
	// TODO(minux): use real TLS relocation, instead of hard-code for Linux
	MOV	$0x10, R27
#endif
	ADD	R27, R0 // TODO: use (R27)(R0)
	MOV	g, (R0)

nocgo:
	RETURN

#ifdef TLSG_IS_VARIABLE
GLOBL runtime·tls_g+0(SB), NOPTR, $8
#endif
