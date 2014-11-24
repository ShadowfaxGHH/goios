// Copyright 2009 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

typedef struct MachoHdr MachoHdr;
struct MachoHdr {
	uint32	cpu;
	uint32	subcpu;
};

typedef struct MachoSect MachoSect;
struct MachoSect {
	char*	name;
	char*	segname;
	uint64	addr;
	uint64	size;
	uint32	off;
	uint32	align;
	uint32	reloc;
	uint32	nreloc;
	uint32	flag;
	uint32	res1;
	uint32	res2;
};

typedef struct MachoSeg MachoSeg;
struct MachoSeg {
	char*	name;
	uint64	vsize;
	uint64	vaddr;
	uint64	fileoffset;
	uint64	filesize;
	uint32	prot1;
	uint32	prot2;
	uint32	nsect;
	uint32	msect;
	MachoSect	*sect;
	uint32	flag;
};

typedef struct MachoLoad MachoLoad;
struct MachoLoad {
	uint32	type;
	uint32	ndata;
	uint32	*data;
};

MachoHdr*	getMachoHdr(void);
MachoSeg*	newMachoSeg(char*, int);
MachoSect*	newMachoSect(MachoSeg*, char*, char*);
MachoLoad*	newMachoLoad(uint32, uint32);
int	machowrite(void);
void	machoinit(void);
void	machosymorder(void);
void	machoemitreloc(void);
int	machoreloc1(Reloc*, vlong);

/*
 * Total amount of space to reserve at the start of the file
 * for Header, PHeaders, and SHeaders.
 * May waste some.
 */
#define	INITIAL_MACHO_HEADR	4*1024

enum {
	MACHO_CPU_AMD64 = (1<<24)|7,
	MACHO_CPU_386 = 7,
	MACHO_SUBCPU_X86 = 3,
	MACHO_CPU_ARM = 12,
	MACHO_SUBCPU_ARM = 0,

	MACHO32SYMSIZE = 12,
	MACHO64SYMSIZE = 16,
	
	MACHO_X86_64_RELOC_UNSIGNED = 0,
	MACHO_X86_64_RELOC_SIGNED = 1,
	MACHO_X86_64_RELOC_BRANCH = 2,
	MACHO_X86_64_RELOC_GOT_LOAD = 3,
	MACHO_X86_64_RELOC_GOT = 4,
	MACHO_X86_64_RELOC_SUBTRACTOR = 5,
	MACHO_X86_64_RELOC_SIGNED_1 = 6,
	MACHO_X86_64_RELOC_SIGNED_2 = 7,
	MACHO_X86_64_RELOC_SIGNED_4 = 8,
	
	MACHO_ARM_RELOC_VANILLA = 0,	/* generic relocation as discribed above */
	MACHO_ARM_RELOC_PAIR = 1,	/* the second relocation entry of a pair */
	MACHO_ARM_RELOC_SECTDIFF = 2,	/* a PAIR follows with subtract symbol value */
	MACHO_ARM_RELOC_LOCAL_SECTDIFF = 3, /* like ARM_RELOC_SECTDIFF, but the symbol referenced was local.  */
	MACHO_ARM_RELOC_PB_LA_PTR = 4, /* prebound lazy pointer */
	MACHO_ARM_RELOC_BR24 = 5,	/* 24 bit branch displacement (to a word address) */
	MACHO_ARM_THUMB_RELOC_BR22 = 6, /* 22 bit branch displacement (to a half-word address) */
	MACHO_ARM_THUMB_32BIT_BRANCH = 7,
	MACHO_ARM_RELOC_HALF = 8,
	MACHO_ARM_RELOC_HALF_SECTDIFF = 9,
	
	MACHO_GENERIC_RELOC_VANILLA = 0,
	
	MACHO_FAKE_GOTPCREL = 100,
};

void	domacho(void);
vlong	domacholink(void);
void	asmbmacho(void);
void	machoadddynlib(char*);
