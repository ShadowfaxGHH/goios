// Copyright 2015 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "textflag.h"

//
// System call support for ARM64, Darwin
//

// func Syscall(syscall uintptr, a1, a2, a3 uintptr) (r1, r2, err uintptr)
TEXT	·Syscall(SB),NOSPLIT,$0-56
	BL	runtime·entersyscall(SB)
	MOV	syscall+0(FP), R16
	MOV	a1+8(FP), R0
	MOV	a2+16(FP), R1
	MOV	a3+24(FP), R2
	SVC	$0x80
	BCC	ok
	MOV	$-1, R1
	MOV	R1, r1+32(FP)	// r1
	MOV	$0, R2
	MOV	R2, r2+40(FP)	// r2
	MOV	R0, errno+48(FP)	// errno
	BL	runtime·exitsyscall(SB)
	RETURN
ok:
	MOV	R0, r1+32(FP) // r1
	MOV	R1, r2+40(FP)	// r2
	MOV	$0, R0
	MOV	R0, errno+48(FP)	// errno
	BL	runtime·exitsyscall(SB)
	RETURN

// func RawSyscall(trap uintptr, a1, a2, a3 uintptr) (r1, r2, err uintptr)
TEXT ·RawSyscall(SB),NOSPLIT,$0-56
	MOV	syscall+0(FP), R16	// syscall entry
	MOV	a1+8(FP), R0
	MOV	a2+16(FP), R1
	MOV	a3+24(FP), R2
	SVC	$0x80
	BCC	ok
	MOV	$-1, R1
	MOV	R1, r1+32(FP)	// r1
	MOV	$0, R2
	MOV	R2, r2+40(FP)	// r2
	MOV	R0, errno+48(FP)	// errno
	RETURN
ok:
	MOV	R0, r1+32(FP) // r1
	MOV	R1, r2+40(FP)	// r2
	MOV	$0, R0
	MOV	R0, errno+48(FP)	// errno
	RETURN

// func Syscall6(trap uintptr, a1, a2, a3, a4, a5, a6 uintptr) (r1, r2, err uintptr)
TEXT	·Syscall6(SB),NOSPLIT,$0-80
	BL	runtime·entersyscall(SB)
	MOV	syscall+0(FP), R16	// syscall entry
	MOV	a1+8(FP), R0
	MOV	a2+16(FP), R1
	MOV	a3+24(FP), R2
	MOV	a4+32(FP), R3
	MOV	a5+40(FP), R4
	MOV	a6+48(FP), R5
	SVC	$0x80
	BCC	ok
	MOV	$-1, R1
	MOV	R1, r1+56(FP)	// r1
	MOV	$0, R2
	MOV	R2, r2+64(FP)	// r2
	MOV	R0, errno+72(FP)	// errno
	BL	runtime·exitsyscall(SB)
	RETURN
ok:
	MOV	R0, r1+56(FP) // r1
	MOV	R1, r2+64(FP)	// r2
	MOV	$0, R0
	MOV	R0, errno+72(FP)	// errno
	BL	runtime·exitsyscall(SB)
	RETURN

// func RawSyscall6(trap uintptr, a1, a2, a3, a4, a5, a6 uintptr) (r1, r2, err uintptr)
TEXT	·RawSyscall6(SB),NOSPLIT,$0-80
	MOV	trap+0(FP), R16	// syscall entry
	MOV	a1+8(FP), R0
	MOV	a2+16(FP), R1
	MOV	a3+24(FP), R2
	MOV	a4+32(FP), R3
	MOV	a5+40(FP), R4
	MOV	a6+48(FP), R5
	SVC	$0x80
	BCC	ok
	MOV	$-1, R1
	MOV	R1, r1+56(FP)	// r1
	MOV	$0, R2
	MOV	R2, r2+64(FP)	// r2
	MOV	R0, errno+72(FP)	// errno
	RETURN
ok:
	MOV	R0, r1+56(FP) // r1
	MOV	R1, r2+64(FP)	// r2
	MOV	$0, R0
	MOV	R0, errno+72(FP)	// errno
	RETURN

// Actually Syscall7
TEXT	·Syscall9(SB),NOSPLIT,$0-104
	BL	runtime·entersyscall(SB)
	MOV	syscall+0(FP), R16	// syscall entry
	MOV	a1+8(FP), R0
	MOV	a2+16(FP), R1
	MOV	a3+24(FP), R2
	MOV	a4+32(FP), R3
	MOV	a5+40(FP), R4
	MOV	a6+48(FP), R5
	MOV	a7+56(FP), R6
	//MOV	a8+64(FP), R7
	//MOV	a9+72(FP), R8
	SVC	$0x80
	BCC	ok
	MOV	$-1, R1
	MOV	R1, r1+80(FP)	// r1
	MOV	$0, R2
	MOV	R2, r2+88(FP)	// r2
	MOV	R0, errno+96(FP)	// errno
	BL	runtime·exitsyscall(SB)
	RETURN
ok:
	MOV	R0, r1+80(FP) // r1
	MOV	R1, r2+88(FP)	// r2
	MOV	$0, R0
	MOV	R0, errno+96(FP)	// errno
	BL	runtime·exitsyscall(SB)
	RETURN

