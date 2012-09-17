/* Dynamic assembler support
 *
 * Copyright (C) 1999 Ian Piumarta <ian.piumarta@inria.fr>
 * See the file COPYRIGHT for details.
 *
 * Last edited: Thu Mar 25 14:22:39 1999 by piumarta (Ian Piumarta) on fricotin
 */

#ifndef __ccg_asm_common_h
#define __ccg_asm_common_h_

#include <stdio.h>
#include <stdlib.h>

#include "asm-cache.h"

#ifndef _ASM_LOCALPC
  static int   asm_pass;	/* 0 (single pass) or 1/2 (two pass) */
  static insn *asm_pc;
#endif

#define ASMFAIL(MSG) asmFail(MSG, __FILE__, __LINE__, __FUNCTION__)

static insn *asm_hwm= 0;

#ifndef  _ASM_APP_1
# define _ASM_APP_1	{ asm_pass= 0; {
#endif
#ifndef  _ASM_NOAPP_1
# define _ASM_NOAPP_1	} asm_hwm= asm_pc; }
#endif
#ifndef  _ASM_APP_2
# define _ASM_APP_2	{ insn *asm_org= asm_pc; for (asm_pass= 1; ((asm_pass < 3) && (asm_pc= asm_org)); ++asm_pass) {
#endif
#ifndef  _ASM_NOAPP_2
# define _ASM_NOAPP_2	if (asm_pass==1) asm_hwm= asm_pc; else if (asm_hwm!=asm_pc) ASMFAIL("phase error"); } }
#endif

#ifndef  _ASM_ORG
# define _ASM_ORG(O)	(asm_pc= (insn *)(O))
#endif
#ifndef  _ASM_LBL
# define _ASM_LBL(V)	static insn *V= 0
#endif
#ifndef  _ASM_DEF
# define _ASM_DEF(V)	(V= (((asm_pass==2)&&(asm_pc!=(V))) ? (insn *)ASMFAIL("phase error") : asm_pc))
#endif

static int asmFail(char *msg, char *file, int line, char *function)
{
  fprintf(stderr, "%s: In function `%s':\n", file, function);
  fprintf(stderr, "%s:%d: %s\n", file, line, msg);
  abort();
  return 0;
}

/* types */

typedef unsigned char  _uc;
typedef unsigned short _us;
typedef unsigned int   _ui;
typedef unsigned long  _ul;

/* integer predicates */

#define _MASK(N)	((_ui)((1<<(N)))-1)
#define _siP(N,I)	(!((((_ui)(I))^(((_ui)(I))<<1))&~_MASK(N)))
#define _uiP(N,I)	(!(((_ui)(I))&~_MASK(N)))

#endif /* __ccg_asm_common_h */
