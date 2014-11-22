// Copyright 2014 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "textflag.h"

TEXT _rt0_arm_darwin(SB),7,$-4
	// disable runfast mode of vfp
	EOR		R12, R12
	WORD	$0xeee1ca10 // fmxr	fpscr, ip
	// prepare arguments for main (_rt0_go)
	MOVW	(R13), R0	// argc
	MOVW	$4(R13), R1		// argv
	MOVM.DB.W [R0-R1], (R13)
	MOVW	$main(SB), R4
	B		(R4)

TEXT main(SB),NOSPLIT,$-8
	MOVW	$runtime·rt0_go(SB), R4
	B		(R4)
