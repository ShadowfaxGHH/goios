// Copyright 2015 The Go Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package runtime

import "unsafe"

type sigctxt struct {
	info *siginfo
	ctxt unsafe.Pointer
}

func (c *sigctxt) regs() *regs64 { return &(*ucontext)(c.ctxt).uc_mcontext.ss }
func (c *sigctxt) x0() uint64        { return c.regs().x[0] }
func (c *sigctxt) x1() uint64        { return c.regs().x[1] }
func (c *sigctxt) x2() uint64        { return c.regs().x[2] }
func (c *sigctxt) x3() uint64        { return c.regs().x[3] }
func (c *sigctxt) x4() uint64        { return c.regs().x[4] }
func (c *sigctxt) x5() uint64        { return c.regs().x[5] }
func (c *sigctxt) x6() uint64        { return c.regs().x[6] }
func (c *sigctxt) x7() uint64        { return c.regs().x[7] }
func (c *sigctxt) x8() uint64        { return c.regs().x[8] }
func (c *sigctxt) x9() uint64        { return c.regs().x[9] }
func (c *sigctxt) x10() uint64       { return c.regs().x[10] }
func (c *sigctxt) x11() uint64       { return c.regs().x[11] }
func (c *sigctxt) x12() uint64       { return c.regs().x[12] }
func (c *sigctxt) x13() uint64       { return c.regs().x[13] }
func (c *sigctxt) x14() uint64       { return c.regs().x[14] }
func (c *sigctxt) x15() uint64       { return c.regs().x[15] }
func (c *sigctxt) x16() uint64       { return c.regs().x[16] }
func (c *sigctxt) x17() uint64       { return c.regs().x[17] }
func (c *sigctxt) x18() uint64       { return c.regs().x[18] }
func (c *sigctxt) x19() uint64       { return c.regs().x[19] }
func (c *sigctxt) x20() uint64       { return c.regs().x[20] }
func (c *sigctxt) x21() uint64       { return c.regs().x[21] }
func (c *sigctxt) x22() uint64       { return c.regs().x[22] }
func (c *sigctxt) x23() uint64       { return c.regs().x[23] }
func (c *sigctxt) x24() uint64       { return c.regs().x[24] }
func (c *sigctxt) x25() uint64       { return c.regs().x[25] }
func (c *sigctxt) x26() uint64       { return c.regs().x[26] }
func (c *sigctxt) x27() uint64       { return c.regs().x[27] }
func (c *sigctxt) x28() uint64       { return c.regs().x[28] }
func (c *sigctxt) x29() uint64       { return c.regs().fp }
func (c *sigctxt) lr() uint64        { return c.regs().lr }
func (c *sigctxt) sp() uint64        { return c.regs().sp }
func (c *sigctxt) pc() uint64        { return c.regs().pc }
func (c *sigctxt) fault() uint64     { return uint64(uintptr(unsafe.Pointer(c.info.si_addr))) }

func (c *sigctxt) sigcode() uint64 { return uint64(c.info.si_code) }
func (c *sigctxt) sigaddr() uint64 { return uint64(uintptr(unsafe.Pointer(c.info.si_addr))) }

func (c *sigctxt) set_pc(x uint64)  { c.regs().pc = x }
func (c *sigctxt) set_sp(x uint64)  { c.regs().sp = x }
func (c *sigctxt) set_lr(x uint64)  { c.regs().lr = x }
func (c *sigctxt) set_r28(x uint64) { c.regs().x[28] = x }

func (c *sigctxt) set_sigaddr(x uint64) {
	c.info.si_addr = (*byte)(unsafe.Pointer(uintptr(x)))
}
