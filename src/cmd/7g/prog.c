// Copyright 2014 The Go Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <u.h>
#include <libc.h>
#include "gg.h"
#include "opt.h"

enum {
	LeftRdwr = LeftRead | LeftWrite,
	RightRdwr = RightRead | RightWrite,
};

// This table gives the basic information about instruction
// generated by the compiler and processed in the optimizer.
// See opt.h for bit definitions.
//
// Instructions not generated need not be listed.
// As an exception to that rule, we typically write down all the
// size variants of an operation even if we just use a subset.
//
// The table is formatted for 8-space tabs.
static ProgInfo progtable[ALAST] = {
	[ATYPE]=	{Pseudo | Skip},
	[ATEXT]=	{Pseudo},
	[AFUNCDATA]=	{Pseudo},
	[APCDATA]=	{Pseudo},
	[AUNDEF]=	{Break},
	[AUSEFIELD]=	{OK},
	[ACHECKNIL]=	{LeftRead},
	[AVARDEF]=	{Pseudo | RightWrite},
	[AVARKILL]=	{Pseudo | RightWrite},

	// NOP is an internal no-op that also stands
	// for USED and SET annotations, not the Power opcode.
	[ANOP]=		{LeftRead | RightWrite},

	// Integer
	[AADD]=		{SizeQ | LeftRead | RegRead | RightWrite},
	[ASUB]=		{SizeQ | LeftRead | RegRead | RightWrite},
	[ANEG]=		{SizeQ | LeftRead | RegRead | RightWrite},
	[AAND]=		{SizeQ | LeftRead | RegRead | RightWrite},
	[AORR]=		{SizeQ | LeftRead | RegRead | RightWrite},
	[AEOR]=		{SizeQ | LeftRead | RegRead | RightWrite},
	[ASMULL]=	{SizeQ | LeftRead | RegRead | RightWrite},
	[ASMULH]=	{SizeL | LeftRead | RegRead | RightWrite},
	[AUMULH]=	{SizeL | LeftRead | RegRead | RightWrite},
	[ASDIV]=	{SizeQ | LeftRead | RegRead | RightWrite},
	[AUDIV]=	{SizeQ | LeftRead | RegRead | RightWrite},
	[ALSL]=		{SizeQ | LeftRead | RegRead | RightWrite},
	[ALSR]=		{SizeQ | LeftRead | RegRead | RightWrite},
	[AASR]=	{SizeQ | LeftRead | RegRead | RightWrite},
	[ACMP]=		{SizeQ | LeftRead | RightRead},
	[ATD]=		{SizeQ | RightRead},

	// Floating point.
	[AFADD]=	{SizeD | LeftRead | RegRead | RightWrite},
	[AFADDS]=	{SizeF | LeftRead | RegRead | RightWrite},
	[AFSUB]=	{SizeD | LeftRead | RegRead | RightWrite},
	[AFSUBS]=	{SizeF | LeftRead | RegRead | RightWrite},
	[AFMUL]=	{SizeD | LeftRead | RegRead | RightWrite},
	[AFMULS]=	{SizeF | LeftRead | RegRead | RightWrite},
	[AFDIV]=	{SizeD | LeftRead | RegRead | RightWrite},
	[AFDIVS]=	{SizeF | LeftRead | RegRead | RightWrite},
	[AFCTIDZ]=	{SizeF | LeftRead | RegRead | RightWrite},
	[AFCFID]=	{SizeF | LeftRead | RegRead | RightWrite},
	[AFCMPU]=	{SizeD | LeftRead | RightRead},
	[AFRSP]=	{SizeD | LeftRead | RightWrite | Conv},

	// Moves
	[AMOVB]=	{SizeB | LeftRead | RightWrite | Move | Conv},
	[AMOVBU]=	{SizeB | LeftRead | RightWrite | Move | Conv | PostInc},
	[AMOVBU]=	{SizeB | LeftRead | RightWrite | Move | Conv},
	[AMOVH]=	{SizeW | LeftRead | RightWrite | Move | Conv},
	[AMOVHU]=	{SizeW | LeftRead | RightWrite | Move | Conv | PostInc},
	[AMOVHU]=	{SizeW | LeftRead | RightWrite | Move | Conv},
	[AMOVW]=	{SizeL | LeftRead | RightWrite | Move | Conv},
	// there is no AMOVWU.
	[AMOVWUU]=	{SizeL | LeftRead | RightWrite | Move | Conv | PostInc},
	[AMOVWU]=	{SizeL | LeftRead | RightWrite | Move | Conv},
	[AMOV]=	{SizeQ | LeftRead | RightWrite | Move},
	[AMOVDU]=	{SizeQ | LeftRead | RightWrite | Move | PostInc},
	[AFMOVS]=	{SizeF | LeftRead | RightWrite | Move | Conv},
	[AFMOVD]=	{SizeD | LeftRead | RightWrite | Move},

	// Jumps
	[AB]=		{Jump | Break},
	[ABL]=		{Call},
	[ABEQ]=		{Cjmp},
	[ABNE]=		{Cjmp},
	[ABGE]=		{Cjmp},
	[ABLT]=		{Cjmp},
	[ABGT]=		{Cjmp},
	[ABLE]=		{Cjmp},
	[ARETURN]=	{Break},

	[ADUFFZERO]=	{Call},
	[ADUFFCOPY]=	{Call},
};

static void
initproginfo(void)
{
	static int initialized;
	int addvariant[] = {V_CC, V_V, V_CC|V_V};
	int as, as2, i, variant;

	if(initialized)
		return;
	initialized = 1;

	// Perform one-time expansion of instructions in progtable to
	// their CC, V, and VCC variants
	for(as=0; as<nelem(progtable); as++) {
		if(progtable[as].flags == 0)
			continue;
		variant = as2variant(as);
		for(i=0; i<nelem(addvariant); i++) {
			as2 = variant2as(as, variant | addvariant[i]);
			if(as2 != 0 && progtable[as2].flags == 0)
				progtable[as2] = progtable[as];
		}
	}
}

void
proginfo(ProgInfo *info, Prog *p)
{
	initproginfo();

	*info = progtable[p->as];
	if(info->flags == 0) {
		*info = progtable[AADD];
		fatal("proginfo: unknown instruction %P", p);
	}

	if((info->flags & RegRead) && p->reg == NREG) {
		info->flags &= ~RegRead;
		info->flags |= /*CanRegRead |*/ RightRead;
	}

	if((p->from.type == D_OREG || p->from.type == D_CONST) && p->from.reg != NREG) {
		info->regindex |= RtoB(p->from.reg);
		if(info->flags & PostInc) {
			info->regset |= RtoB(p->from.reg);
		}
	}
	if((p->to.type == D_OREG || p->to.type == D_CONST) && p->to.reg != NREG) {
		info->regindex |= RtoB(p->to.reg);
		if(info->flags & PostInc) {
			info->regset |= RtoB(p->to.reg);
		}
	}

	if(p->from.type == D_CONST && p->from.sym != nil && (info->flags & LeftRead)) {
		info->flags &= ~LeftRead;
		info->flags |= LeftAddr;
	}

	if(p->as == ADUFFZERO) {
		info->reguse |= (1<<D_R0) | RtoB(3);
		info->regset |= RtoB(3);
	}
	if(p->as == ADUFFCOPY) {
		// TODO(austin) Revisit when duffcopy is implemented
		info->reguse |= RtoB(3) | RtoB(4) | RtoB(5);
		info->regset |= RtoB(3) | RtoB(4);
	}
}

// Instruction variants table.  Initially this contains entries only
// for the "base" form of each instruction.  On the first call to
// as2variant or variant2as, we'll add the variants to the table.
static int varianttable[ALAST][4] = {
	[AADD]=		{AADD,		AADDCC,		AADDV,		AADDVCC},
	[AADDC]=	{AADDC,		AADDCCC,	AADDCV,		AADDCVCC},
	[AADDE]=	{AADDE,		AADDECC,	AADDEV,		AADDEVCC},
	[AADDME]=	{AADDME,	AADDMECC,	AADDMEV,	AADDMEVCC},
	[AADDZE]=	{AADDZE,	AADDZECC,	AADDZEV,	AADDZEVCC},
	[AAND]=		{AAND,		AANDCC,		0,		0},
	[AANDN]=	{AANDN,		AANDNCC,	0,		0},
	[ACNTLZD]=	{ACNTLZD,	ACNTLZDCC,	0,		0},
	[ACNTLZW]=	{ACNTLZW,	ACNTLZWCC,	0,		0},
	[ADIVD]=	{ADIVD,		ADIVDCC,	ADIVDV,		ADIVDVCC},
	[ADIVDU]=	{ADIVDU,	ADIVDUCC,	ADIVDUV,	ADIVDUVCC},
	[ADIVW]=	{ADIVW,		ADIVWCC,	ADIVWV,		ADIVWVCC},
	[ADIVWU]=	{ADIVWU,	ADIVWUCC,	ADIVWUV,	ADIVWUVCC},
	[AEQV]=		{AEQV,		AEQVCC,		0,		0},
	[AEXTSB]=	{AEXTSB,	AEXTSBCC,	0,		0},
	[AEXTSH]=	{AEXTSH,	AEXTSHCC,	0,		0},
	[AEXTSW]=	{AEXTSW,	AEXTSWCC,	0,		0},
	[AFABS]=	{AFABS,		AFABSCC,	0,		0},
	[AFADD]=	{AFADD,		AFADDCC,	0,		0},
	[AFADDS]=	{AFADDS,	AFADDSCC,	0,		0},
	[AFCFID]=	{AFCFID,	AFCFIDCC,	0,		0},
	[AFCTID]=	{AFCTID,	AFCTIDCC,	0,		0},
	[AFCTIDZ]=	{AFCTIDZ,	AFCTIDZCC,	0,		0},
	[AFCTIW]=	{AFCTIW,	AFCTIWCC,	0,		0},
	[AFCTIWZ]=	{AFCTIWZ,	AFCTIWZCC,	0,		0},
	[AFDIV]=	{AFDIV,		AFDIVCC,	0,		0},
	[AFDIVS]=	{AFDIVS,	AFDIVSCC,	0,		0},
	[AFMADD]=	{AFMADD,	AFMADDCC,	0,		0},
	[AFMADDS]=	{AFMADDS,	AFMADDSCC,	0,		0},
	[AFMOVD]=	{AFMOVD,	AFMOVDCC,	0,		0},
	[AFMSUB]=	{AFMSUB,	AFMSUBCC,	0,		0},
	[AFMSUBS]=	{AFMSUBS,	AFMSUBSCC,	0,		0},
	[AFMUL]=	{AFMUL,		AFMULCC,	0,		0},
	[AFMULS]=	{AFMULS,	AFMULSCC,	0,		0},
	[AFNABS]=	{AFNABS,	AFNABSCC,	0,		0},
	[AFNEG]=	{AFNEG,		AFNEGCC,	0,		0},
	[AFNMADD]=	{AFNMADD,	AFNMADDCC,	0,		0},
	[AFNMADDS]=	{AFNMADDS,	AFNMADDSCC,	0,		0},
	[AFNMSUB]=	{AFNMSUB,	AFNMSUBCC,	0,		0},
	[AFNMSUBS]=	{AFNMSUBS,	AFNMSUBSCC,	0,		0},
	[AFRES]=	{AFRES,		AFRESCC,	0,		0},
	[AFRSP]=	{AFRSP,		AFRSPCC,	0,		0},
	[AFRSQRTE]=	{AFRSQRTE,	AFRSQRTECC,	0,		0},
	[AFSEL]=	{AFSEL,		AFSELCC,	0,		0},
	[AFSQRT]=	{AFSQRT,	AFSQRTCC,	0,		0},
	[AFSQRTS]=	{AFSQRTS,	AFSQRTSCC,	0,		0},
	[AFSUB]=	{AFSUB,		AFSUBCC,	0,		0},
	[AFSUBS]=	{AFSUBS,	AFSUBSCC,	0,		0},
	[AMTFSB0]=	{AMTFSB0,	AMTFSB0CC,	0,		0},
	[AMTFSB1]=	{AMTFSB1,	AMTFSB1CC,	0,		0},
	[AMULHD]=	{AMULHD,	AMULHDCC,	0,		0},
	[AMULHDU]=	{AMULHDU,	AMULHDUCC,	0,		0},
	[AMULHW]=	{AMULHW,	AMULHWCC,	0,		0},
	[AMULHWU]=	{AMULHWU,	AMULHWUCC,	0,		0},
	[AMULLD]=	{AMULLD,	AMULLDCC,	AMULLDV,	AMULLDVCC},
	[AMULLW]=	{AMULLW,	AMULLWCC,	AMULLWV,	AMULLWVCC},
	[ANAND]=	{ANAND,		ANANDCC,	0,		0},
	[ANEG]=		{ANEG,		ANEGCC,		ANEGV,		ANEGVCC},
	[ANOR]=		{ANOR,		ANORCC,		0,		0},
	[AOR]=		{AOR,		AORCC,		0,		0},
	[AORN]=		{AORN,		AORNCC,		0,		0},
	[AREM]=		{AREM,		AREMCC,		AREMV,		AREMVCC},
	[AREMD]=	{AREMD,		AREMDCC,	AREMDV,		AREMDVCC},
	[AREMDU]=	{AREMDU,	AREMDUCC,	AREMDUV,	AREMDUVCC},
	[AREMU]=	{AREMU,		AREMUCC,	AREMUV,		AREMUVCC},
	[ARLDC]=	{ARLDC,		ARLDCCC,	0,		0},
	[ARLDCL]=	{ARLDCL,	ARLDCLCC,	0,		0},
	[ARLDCR]=	{ARLDCR,	ARLDCRCC,	0,		0},
	[ARLDMI]=	{ARLDMI,	ARLDMICC,	0,		0},
	[ARLWMI]=	{ARLWMI,	ARLWMICC,	0,		0},
	[ARLWNM]=	{ARLWNM,	ARLWNMCC,	0,		0},
	[ASLD]=		{ASLD,		ASLDCC,		0,		0},
	[ASLW]=		{ASLW,		ASLWCC,		0,		0},
	[ASRAD]=	{ASRAD,		ASRADCC,	0,		0},
	[ASRAW]=	{ASRAW,		ASRAWCC,	0,		0},
	[ASRD]=		{ASRD,		ASRDCC,		0,		0},
	[ASRW]=		{ASRW,		ASRWCC,		0,		0},
	[ASUB]=		{ASUB,		ASUBCC,		ASUBV,		ASUBVCC},
	[ASUBC]=	{ASUBC,		ASUBCCC,	ASUBCV,		ASUBCVCC},
	[ASUBE]=	{ASUBE,		ASUBECC,	ASUBEV,		ASUBEVCC},
	[ASUBME]=	{ASUBME,	ASUBMECC,	ASUBMEV,	ASUBMEVCC},
	[ASUBZE]=	{ASUBZE,	ASUBZECC,	ASUBZEV,	ASUBZEVCC},
	[AXOR]=		{AXOR,		AXORCC,		0,		0},
};

static void
initvariants(void)
{
	static int initialized;
	int i, j;

	if(initialized)
		return;
	initialized = 1;

	for(i=0; i<nelem(varianttable); i++) {
		if(varianttable[i][0] == 0) {
			// Instruction has no variants
			varianttable[i][0] = i;
			continue;
		}
		// Copy base form to other variants
		if(varianttable[i][0] == i) {
			for(j=0; j<nelem(varianttable[i]); j++)
				memmove(&varianttable[varianttable[i][j]], &varianttable[i], sizeof(varianttable[i]));
		}
	}
}

// as2variant returns the variant (V_*) flags of instruction as.
int
as2variant(int as)
{
	int i;
	initvariants();
	for(i=0; i<nelem(varianttable[as]); i++)
		if(varianttable[as][i] == as)
			return i;
	fatal("as2variant: instruction %A is not a variant of itself", as);
	return 0;
}

// variant2as returns the instruction as with the given variant (V_*) flags.
// If no such variant exists, this returns 0.
int
variant2as(int as, int flags)
{
	initvariants();
	return varianttable[as][flags];
}
