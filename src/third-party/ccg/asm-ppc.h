/* Dynamic assembler for PowerPC
 *
 * Copyright (C) 1999 Ian Piumarta <ian.piumarta@inria.fr>
 * See the file COPYRIGHT for details.
 *
 * Last edited: Fri Mar 26 02:29:57 1999 by piumarta (Ian Piumarta) on pingu
 */

#ifndef __ccg_asm_ppc_h
#define __ccg_asm_ppc_h

#if !defined(PPC) && !defined(_POWER) && !defined(_IBMR2)
# warning:
# warning: FOREIGN ARCHITECTURE SELECTED
# warning:
#endif

#include "alpha"

typedef unsigned int insn;

#include "asm-common.h"

#define cr0	0
#define cr1	1
#define cr2	2
#define cr3	3
#define cr4	4
#define cr5	5
#define cr6	6
#define cr7	7

#define lt	0
#define gt	1
#define eq	2
#define so	3


#define _UL(X)		((unsigned long)(X))
#define _SL(X)		((long)(X))

#define _ck_s(W,I)	(_siP(W,I) ? (_UL(I) & _MASK(W)) : ASMFAIL(  "signed integer `"#I"' too large for "#W"-bit field"))
#define _ck_u(W,I)    	(_uiP(W,I) ? (_UL(I) & _MASK(W)) : ASMFAIL("unsigned integer `"#I"' too large for "#W"-bit field"))

#define __ck_d(W,D)	(_siP(W,D) ? (_UL(D) & (_MASK(W)-3)) : ASMFAIL(    "displacement `"#D"' too large for "#W"-bit field"))
#define _ck_d(W,I)    	((asm_pass==1) ? ((_UL(I)-_UL(asm_pc)) & (_MASK(W)-3)) : __ck_d(W,_UL(I)-_UL(asm_pc)))

#define _s16(I)         _ck_s(16,I)
				
#define _u1(I)          _ck_u( 1,I)
#define _u5(I)          _ck_u( 5,I)
#define _u6(I)          _ck_u( 6,I)
#define _u9(I)          _ck_u( 9,I)
#define _u10(I)         _ck_u(10,I)
#define _u16(I)         _ck_u(16,I)

#define _d16(D)		_ck_d(16,D)
#define _d26(D)		_ck_d(26,D)

#define _HI(I)          (_SL(            I) >>     (16))
#define _LO(I)          (_SL((short)(_UL(I) & _MASK(16))))

#define _GEN(X)		((*asm_pc= (X)), ++asm_pc)

/* primitive instruction forms [1, Section A.4] */

#define _I(   OP,         BD,AA,LK )	_GEN((_u6(OP)<<26)|                                              _d26(BD)|   (_u1(AA)<<1)|_u1(LK))
#define _B(   OP,BO,BI,   BD,AA,LK )  	_GEN((_u6(OP)<<26)|(_u5(BO)<<21)|(_u5(BI)<<16)|                  _d16(BD)|   (_u1(AA)<<1)|_u1(LK))
#define _D(   OP,RD,RA,         DD )  	_GEN((_u6(OP)<<26)|(_u5(RD)<<21)|(_u5(RA)<<16)|                  _s16(DD)                        )
#define _X(   OP,RD,RA,RB,   XO,RC )  	_GEN((_u6(OP)<<26)|(_u5(RD)<<21)|(_u5(RA)<<16)|( _u5(RB)<<11)|              (_u10(XO)<<1)|_u1(RC))
#define _XL(  OP,BO,BI,      XO,LK )  	_GEN((_u6(OP)<<26)|(_u5(BO)<<21)|(_u5(BI)<<16)|( _u5(00)<<11)|              (_u10(XO)<<1)|_u1(LK))
#define _XFX( OP,RD,         SR,XO )  	_GEN((_u6(OP)<<26)|(_u5(RD)<<21)|              (_u10(SR)<<11)|              (_u10(XO)<<1)|_u1(00))
#define _XO(  OP,RD,RA,RB,OE,XO,RC )  	_GEN((_u6(OP)<<26)|(_u5(RD)<<21)|(_u5(RA)<<16)|( _u5(RB)<<11)|(_u1(OE)<<10)|( _u9(XO)<<1)|_u1(RC))
#define _M(   OP,RS,RA,SH,MB,ME,RC )  	_GEN((_u6(OP)<<26)|(_u5(RS)<<21)|(_u5(RA)<<16)|( _u5(SH)<<11)|(_u5(MB)<< 6)|( _u5(ME)<<1)|_u1(RC))

/* special purpose registers (form XFX) [1, Section 8.2, page 8-138] */

#define SPR_LR		((8<<5)|(0))

/* +++ intrinsic instructions */

#define ADDrrr(RD, RA, RB)		_X	(31, RD, RA, RB, 266, 0)
#define ADDIrri(RD, RA, IMM)		_D	(14, RD, RA, IMM)
#define ADDISrri(RD, RA, IMM)		_D	(15, RD, RA, IMM)

#define ANDrrr(RA, RS, RB)		_X	(31, RS, RA, RB,  28, 0)
#define ANDCrrr(RA, RS, RB)		_X	(31, RS, RA, RB,  60, 0)
#define ANDI_rri(RA, RS, IMM)		_D	(28, RS, RA, IMM)
#define ANDIS_rri(RA, RS, IMM)		_D	(29, RS, RA, IMM)

#define Bi(BD)				_I	(18, BD, 0, 0)
#define BAi(BD)				_I	(18, BD, 1, 0)
#define BLi(BD)				_I	(18, BD, 0, 1)
#define BLAi(BD)			_I	(18, BD, 1, 1)

#define BCiii(BO,BI,BD)			_B	(16, BO, BI, BD, 0, 0)
#define BCAiii(BO,BI,BD)		_B	(16, BO, BI, BD, 1, 0)
#define BCLiii(BO,BI,BD)		_B	(16, BO, BI, BD, 0, 1)
#define BCLAiii(BO,BI,BD)		_B	(16, BO, BI, BD, 1, 1)

#define BCCTRii(BO,BI)			_XL	(19, BO, BI, 528, 0)
#define BCCTRLii(BO,BI)			_XL	(19, BO, BI, 528, 1)

#define BCLRii(BO,BI)			_XL	(19, BO, BI,  16, 0)
#define BCLRLii(BO,BI)			_XL	(19, BO, BI,  16, 1)

#define CMPiirr(CR, LL, RA, RB)		_X	(31, ((CR)<<2)|(LL), RA, RB, 0, 0)
#define CMPIiiri(CR, LL, RA, IMM)	_D	(11, ((CR)<<2)|(LL), RA, IMM)

#define CMPLiirr(CR, LL, RA, RB)	_X	(31, ((CR)<<2)|(LL), RA, RB, 32, 0)
#define CMPLIiiri(CR, LL, RA, IMM)	_D	(10, ((CR)<<2)|(LL), RA, IMM)

#define CRANDiii(CRD,CRA,CRB)		_X	(19, CRD, CRA, CRB, 257, 0)
#define CRANDCiii(CRD,CRA,CRB)		_X	(19, CRD, CRA, CRB, 129, 0)
#define CREQViii(CRD,CRA,CRB)		_X	(19, CRD, CRA, CRB, 289, 0)
#define CRNANDiii(CRD,CRA,CRB)		_X	(19, CRD, CRA, CRB, 225, 0)
#define CRNORiii(CRD,CRA,CRB)		_X	(19, CRD, CRA, CRB,  33, 0)
#define CRORiii(CRD,CRA,CRB)		_X	(19, CRD, CRA, CRB, 449, 0)
#define CRORCiii(CRD,CRA,CRB)		_X	(19, CRD, CRA, CRB, 417, 0)
#define CRXORiii(CRD,CRA,CRB)		_X	(19, CRD, CRA, CRB, 193, 0)

#define DCBSTrr(RA,RB)			_X	(31, 00, RA, RB,  54, 0)

#define DIVWrrr(RD, RA, RB)		_XO	(31, RD, RA, RB, 0, 491, 0)
#define DIVW_rrr(RD, RA, RB)		_XO	(31, RD, RA, RB, 0, 491, 1)
#define DIVWOrrr(RD, RA, RB)		_XO	(31, RD, RA, RB, 1, 491, 0)
#define DIVWO_rrr(RD, RA, RB)		_XO	(31, RD, RA, RB, 1, 491, 1)

#define DIVWUrrr(RD, RA, RB)		_XO	(31, RD, RA, RB, 0, 459, 0)
#define DIVWU_rrr(RD, RA, RB)		_XO	(31, RD, RA, RB, 0, 459, 1)
#define DIVWUOrrr(RD, RA, RB)		_XO	(31, RD, RA, RB, 1, 459, 0)
#define DIVWUO_rrr(RD, RA, RB)		_XO	(31, RD, RA, RB, 1, 459, 1)

#define EQVrrr(Ra,RS,RB)		_X	(31, RS, RA, RB, 284, 0)
#define EQV_rrr(Ra,RS,RB)		_X	(31, RS, RA, RB, 284, 1)

#define EXTSBrr(RA,RS)			_X	(31, RS, RA,  0, 954, 0)
#define EXTSB_rr(RA,RS)			_X	(31, RS, RA,  0, 954, 1)

#define EXTSHrr(RA,RS)			_X	(31, RS, RA,  0, 922, 0)
#define EXTSH_rr(RA,RS)			_X	(31, RS, RA,  0, 922, 1)

#define ICBIrr(RA,RB)			_X	(31, 00, RA, RB, 982, 0)

#define ISYNC()				_X	(19, 00, 00, 00, 150, 0)

#define LBZrm(RD,ID,RA)			_D	(34, RD, RA, ID)
#define LBZUrm(RD,ID,RA)		_D	(35, RD, RA, ID)
#define LBZUXrrr(RD,RA,RB)		_X	(31, RD, RA, RB, 119, 0)
#define LBZXrrr(RD,RA,RB)		_X	(31, RD, RA, RB,  87, 0)

#define LHArm(RD,ID,RA)			_D	(42, RD, RA, ID)
#define LHAUrm(RD,ID,RA)		_D	(43, RD, RA, ID)
#define LHAUXrrr(RD,RA,RB)		_X	(31, RD, RA, RB, 375, 0)
#define LHAXrrr(RD,RA,RB)		_X	(31, RD, RA, RB, 343, 0)
#define LHBRXrrr(RD,RA,RB)		_X	(31, RD, RA, RB, 790, 0)

#define LHZrm(RD,ID,RA)			_D	(40, RD, RA, ID)
#define LHZUrm(RD,ID,RA)		_D	(41, RD, RA, ID)
#define LHZUXrrr(RD,RA,RB)		_X	(31, RD, RA, RB, 311, 0)
#define LHZXrrr(RD,RA,RB)		_X	(31, RD, RA, RB, 279, 0)

#define LMWrm(RD,ID,RA)			_D	(46, RD, RA, ID)

#define LWBRXrrr(RD,RA,RB)		_X	(31, RD, RA, RB, 534, 0)

#define LWZrm(RD, DISP, RA)		_D	(32, RD, RA, DISP)
#define LWZUrm(RD, DISP, RA)		_D	(33, RD, RA, DISP)
#define LWZUXrrr(RD, RA, RB)		_X	(31, RD, RA, RB,  56, 0)
#define LWZXrrr(RD, RA, RB)		_X	(31, RD, RA, RB,  23, 0)

#define MCRFii(CD,CS)			_X	(19, ((CD)<<2), ((CS)<<2), 0, 0, 0)

#define MFCRr(RD)			_X	(31, RD, 0, 0, 19, 0)

#define MFSPRri(RD, SPR)		_XFX	(31, RD, (SPR)<<5, 339)
#define MTSPRir(SPR, RS)		_XFX	(31, RS, (SPR)<<5, 467)

#define MULHWrrr(RD,RA,RB)		_XO	(31, RD, RA, RB, 0,  75, 0)
#define MULHW_rrr(RD,RA,RB)		_XO	(31, RD, RA, RB, 0,  75, 1)
#define MULHWUrrr(RD,RA,RB)		_XO	(31, RD, RA, RB, 0,  11, 0)
#define MULHWU_rrr(RD,RA,RB)		_XO	(31, RD, RA, RB, 0,  11, 1)

#define MULLIrri(RD,RA,IM)		_D	(07, RD, RA, IM)

#define MULLWrrr(RD,RA,RB)		_XO	(31, RD, RA, RB, 0, 235, 0)
#define MULLW_rrr(RD,RA,RB)		_XO	(31, RD, RA, RB, 0, 235, 1)
#define MULLWOrrr(RD,RA,RB)		_XO	(31, RD, RA, RB, 1, 235, 0)
#define MULLWO_rrr(RD,RA,RB)		_XO	(31, RD, RA, RB, 1, 235, 1)

#define NANDrrr(RA,RS,RB)		_X	(31, RS, RA, RB, 476, 0)
#define NAND_rrr(RA,RS,RB)		_X	(31, RS, RA, RB, 476, 1)

#define NEGrr(RD,RA)			_XO	(31, RD, RA, 0, 0, 104, 0)
#define NEG_rr(RD,RA)			_XO	(31, RD, RA, 0, 0, 104, 1)
#define NEGOrr(RD,RA)			_XO	(31, RD, RA, 0, 1, 104, 0)
#define NEGO_rr(RD,RA)			_XO	(31, RD, RA, 0, 1, 104, 1)

#define NORrrr(RA,RS,RB)		_X	(31, RS, RA, RB, 124, 0)
#define NOR_rrr(RA,RS,RB)		_X	(31, RS, RA, RB, 124, 1)

#define ORrrr(RA,RS,RB)			_X	(31, RS, RA, RB, 444, 0)
#define OR_rrr(RA,RS,RB)		_X	(31, RS, RA, RB, 444, 1)
#define ORCrrr(RA,RS,RB)		_X	(31, RS, RA, RB, 412, 0)
#define ORC_rrr(RA,RS,RB)		_X	(31, RS, RA, RB, 412, 1)
#define ORIrri(RA,RS,IM)		_D	(24, RS, RA, IM)
#define ORISrri(RA,RS,IM)		_D	(25, RS, RA, IM)

#define RLWIMIrriii(RA,RS,SH,MB,ME)	_M	(20, RS, RA, SH, MB, ME, 0)
#define RLWIMI_rriii(RA,RS,SH,MB,ME)	_M	(20, RS, RA, SH, MB, ME, 1)

#define RLWINMrriii(RA,RS,SH,MB,ME)	_M	(21, RS, RA, SH, MB, ME, 0)
#define RLWINM_rriii(RA,RS,SH,MB,ME)	_M	(21, RS, RA, SH, MB, ME, 1)

#define RLWNMrrrii(RA,RS,RB,MB,ME)	_M	(23, RS, RA, RB, MB, ME, 0)
#define RLWNM_rrrii(RA,RS,RB,MB,ME)	_M	(23, RS, RA, RB, MB, ME, 1)

#define SLWrrr(RA,RS,RB)		_X	(31, RS, RA, RB,  24, 0)
#define SLW_rrr(RA,RS,RB)		_X	(31, RS, RA, RB,  24, 1)

#define SRAWrrr(RA,RS,RB)		_X	(31, RS, RA, RB, 792, 0)
#define SRAW_rrr(RA,RS,RB)		_X	(31, RS, RA, RB, 792, 1)

#define SRAWIrri(RD, RS, SH)		_X	(31, RS, RD, SH, 824, 0)
#define SRAWI_rri(RD, RS, SH)		_X	(31, RS, RD, SH, 824, 1)

#define SRWrrr(RA,RS,RB)		_X	(31, RS, RA, RB, 536, 0)
#define SRW_rrr(RA,RS,RB)		_X	(31, RS, RA, RB, 536, 1)

#define STBrm(RS,ID,RA)			_D	(38, RS, RA, ID)
#define STBUrm(RS,ID,RA)		_D	(39, RS, RA, ID)
#define STBUXrrr(RD,RA,RB)		_X	(31, RS, RA, RB, 247, 0)
#define STBXrrr(RD,RA,RB)		_X	(31, RS, RA, RB, 215, 0)

#define STHrm(RS,ID,RA)			_D	(44, RS, RA, ID)
#define STHUrm(RS,ID,RA)		_D	(45, RS, RA, ID)
#define STHBRXrrr(RS,RA,RB)		_X	(31, RS, RA, RB, 918, 0)
#define STHUXrrr(RS,RA,RB)		_X	(31, RS, RA, RB, 439, 0)
#define STHXrrr(RS,RA,RB)		_X	(31, RS, RA, RB, 407, 0)

#define STMWrm(RD,ID,RA)		_D	(47, RD, RA, ID)

#define STWrm(RS,ID,RA)			_D	(36, RS, RA, ID)
#define STWBRXrrr(RS,RA,RB)		_X	(31, RS, RA, RB, 662, 0)
#define STWCXrrr(RS,RA,RB)		_X	(31, RS, RA, RB, 150, 0)
#define STWCX_rrr(RS,RA,RB)		_X	(31, RS, RA, RB, 150, 1)
#define STWUrm(RS,ID,RA)		_D	(37, RS, RA, ID)
#define STWUXrrr(RS,RA,RB)		_X	(31, RS, RA, RB, 183, 0)
#define STWXrrr(RS,RA,RB)		_X	(31, RS, RA, RB, 151, 0)

#define SUBFrrr(RD, RA, RB)		_XO	(31, RD, RA, RB, 0, 40, 0)
#define SUBF_rrr(RD, RA, RB)		_XO	(31, RD, RA, RB, 0, 40, 1)
#define SUBFOrrr(RD, RA, RB)		_XO	(31, RD, RA, RB, 1, 40, 0)
#define SUBFO_rrr(RD, RA, RB)		_XO	(31, RD, RA, RB, 1, 40, 1)

#define SYNC()				_X	(31, 00, 00, 00, 598, 0)

#define TWirr(TO,RA,RB)			_X	(31, TO, RA, RB,   4, 0)
#define TWIiri(TO,RA,IM)		_D	(03, TO, RA, IM)

#define XORrrr(RA,RS,RB)		_X	(31, RS, RA, RB, 316, 0)
#define XOR_rrr(RA,RS,RB)		_X	(31, RS, RA, RB, 316, 1)
#define XORIrri(RA,RS,IM)		_D	(26, RS, RA, IM)
#define XORISrri(RA,RS,IM)		_D	(27, RS, RA, IM)

/* simplified mnemonics [1, Appendix F] */

#define SUBIrri(RD,RA,IM)		ADDIrri(RD,RA,-(IM))	/* [1, Section F.2.1] */
#define SUBISrri(RD,RA,IM)		ADDISrri(RD,RA,-(IM))
#define SUBICrri(RD,RA,IM)		ADDICrri(RD,RA,-(IM))
#define SUBIC_rri(RD,RA,IM)		ADDIC_rri(RD,RA,-(IM))

#define SUBrrr(RD,RA,RB)		SUBFrrr(RD,RB,RA)	/* [1, Section F.2.2] */
#define SUBOrrr(RD,RA,RB)		SUBFOrrr(RD,RB,RA)
#define SUB_rrr(RD,RA,RB)		SUBF_rrr(RD,RB,RA)
#define SUBCrrr(RD,RA,RB)		SUBFCrrr(RD,RB,RA)
#define SUBCOrrr(RD,RA,RB)		SUBFCOrrr(RD,RB,RA)
#define SUBC_rrr(RD,RA,RB)		SUBFC_rrr(RD,RB,RA)

#define CMPWIiri(C,RA,IM)		CMPIiiri(C,0,RA,IM)	/* [1, Table F-2] */
#define CMPWirr(C,RA,RB)		CMPiirr(C,0,RA,RB)
#define CMPLWIiri(C,RA,IM)		CMPLIiiri(C,0,RA,IM)
#define CMPLWirr(C,RA,RB)		CMPLiirr(C,0,RA,RB)

#define CMPWIri(RA,IM)			CMPWIiri(0,RA,IM)	/* with implicit cr0 */
#define CMPWrr(RA,RB)			CMPWirr(0,RA,RB)
#define CMPLWIri(RA,IM)			CMPLWIiri(0,RA,IM)
#define CMPLWrr(RA,RB)			CMPLWirr(0,RA,RB)

#define EXTLWIrrii(RA,RS,N,B)		RLWINMrriii(RA, RS,            B,	0,     (N)-1)	/* [1, Table F-3] */
#define EXTRWIrrii(RA,RS,N,B)		RLWINMrriii(RA, RS,      (B)+(N),  32-(N),        31)
#define INSLWIrrii(RA,RS,N,B)		RLWIMIrriii(RA, RS,       32-(B),	B, (B)+(N)-1)
#define INSRWIrrii(RA,RS,N,B)		RLWIMIrriii(RA, RS, 32-((B)+(N)),	B, (B)+(N)-1)
#define ROTLWIrri(RA,RS,N)		RLWINMrriii(RA, RS,            N,	0,        31)
#define ROTRWIrri(RA,RS,N)		RLWINMrriii(RA, RS,       32-(N),	0,        31)
#define ROTLWrrr(RA,RS,RB)		RLWNMrrrii( RA, RS,           RB,	0,        31)
#define SLWIrri(RA,RS,N)		RLWINMrriii(RA, RS,            N,	0,    31-(N))
#define SRWIrri(RA,RS,N)		RLWINMrriii(RA, RS,       32-(N),	N,        31)
#define CLRLWIrri(RA,RS,N)		RLWINMrriii(RA, RS,            0,	N,        31)
#define CLRRWIrri(RA,RS,N)		RLWINMrriii(RA, RS,            0,	0,    31-(N))
#define CLRLSLWIrrii(RA,RS,B,N)		RLWINMrriii(RA, RS,            N, (B)-(N),    31-(N))

#define BTii(C,D)			BC(12, C, D)		/* [1, Table F-5] */
#define BFii(C,D)			BC( 4, C, D)
#define BDNZi(D)			BC(16, 0, D)
#define BDNZTii(C,D)			BC( 8, C, D)
#define BDNZFii(C,D)			BC( 0, C, D)
#define BDZi(D)				BC(18, 0, D)
#define BDZTii(C,D)			BC(10, C, D)
#define BDZFii(C,D)			BC( 2, C, D)
		

#define BLR()				BCLRii(20, 0)		/* [1, Table F-6] */
#define BLRL()				BCLRLii(20, 0)
		

#define BLTLRi(CR)			BCLRii(12, ((CR)<<2)+0)	/* [1, Table F-10] */
#define BLELRi(CR)			BCLRii( 4  ((CR)<<2)+1)
#define BEQLRi(CR)			BCLRii(12, ((CR)<<2)+2)
#define BGELRi(CR)			BCLRii( 4, ((CR)<<2)+0)
#define BGTLRi(CR)			BCLRii(12, ((CR)<<2)+1)
#define BNLLRi(CR)			BCLRii( 4, ((CR)<<2)+0)
#define BNELRi(CR)			BCLRii( 4, ((CR)<<2)+2)
#define BNGLRi(CR)			BCLRii( 4, ((CR)<<2)+1)
#define BSOLRi(CR)			BCLRii(12, ((CR)<<2)+3)
#define BNSLRi(CR)			BCLRii( 4, ((CR)<<2)+3)
#define BUNLRi(CR)			BCLRii(12, ((CR)<<2)+3)
#define BNULRi(CR)			BCLRii( 4, ((CR)<<2)+3)
		
#define BLTLRLi(CR)			BCLRLii(12, ((CR)<<2)+0)	/* [1, Table F-10] */
#define BLELRLi(CR)			BCLRLii( 4, ((CR)<<2)+1)
#define BEQLRLi(CR)			BCLRLii(12, ((CR)<<2)+2)
#define BGELRLi(CR)			BCLRLii( 4, ((CR)<<2)+0)
#define BGTLRLi(CR)			BCLRLii(12, ((CR)<<2)+1)
#define BNLLRLi(CR)			BCLRLii( 4, ((CR)<<2)+0)
#define BNELRLi(CR)			BCLRLii( 4, ((CR)<<2)+2)
#define BNGLRLi(CR)			BCLRLii( 4, ((CR)<<2)+1)
#define BSOLRLi(CR)			BCLRLii(12, ((CR)<<2)+3)
#define BNSLRLi(CR)			BCLRLii( 4, ((CR)<<2)+3)
#define BUNLRLi(CR)			BCLRLii(12, ((CR)<<2)+3)
#define BNULRLi(CR)			BCLRLii( 4, ((CR)<<2)+3)
		
#define BLTCTRi(CR)			BCCTRii(12, ((CR)<<2)+0)	/* [1, Table F-10] */
#define BLECTRi(CR)			BCCTRii( 4  ((CR)<<2)+1)
#define BEQCTRi(CR)			BCCTRii(12, ((CR)<<2)+2)
#define BGECTRi(CR)			BCCTRii( 4, ((CR)<<2)+0)
#define BGTCTRi(CR)			BCCTRii(12, ((CR)<<2)+1)
#define BNLCTRi(CR)			BCCTRii( 4, ((CR)<<2)+0)
#define BNECTRi(CR)			BCCTRii( 4, ((CR)<<2)+2)
#define BNGCTRi(CR)			BCCTRii( 4, ((CR)<<2)+1)
#define BSOCTRi(CR)			BCCTRii(12, ((CR)<<2)+3)
#define BNSCTRi(CR)			BCCTRii( 4, ((CR)<<2)+3)
#define BUNCTRi(CR)			BCCTRii(12, ((CR)<<2)+3)
#define BNUCTRi(CR)			BCCTRii( 4, ((CR)<<2)+3)
		
#define BLTCTRLi(CR)			BCCTRLii(12, ((CR)<<2)+0)	/* [1, Table F-10] */
#define BLECTRLi(CR)			BCCTRLii( 4, ((CR)<<2)+1)
#define BEQCTRLi(CR)			BCCTRLii(12, ((CR)<<2)+2)
#define BGECTRLi(CR)			BCCTRLii( 4, ((CR)<<2)+0)
#define BGTCTRLi(CR)			BCCTRLii(12, ((CR)<<2)+1)
#define BNLCTRLi(CR)			BCCTRLii( 4, ((CR)<<2)+0)
#define BNECTRLi(CR)			BCCTRLii( 4, ((CR)<<2)+2)
#define BNGCTRLi(CR)			BCCTRLii( 4, ((CR)<<2)+1)
#define BSOCTRLi(CR)			BCCTRLii(12, ((CR)<<2)+3)
#define BNSCTRLi(CR)			BCCTRLii( 4, ((CR)<<2)+3)
#define BUNCTRLi(CR)			BCCTRLii(12, ((CR)<<2)+3)
#define BNUCTRLi(CR)			BCCTRLii( 4, ((CR)<<2)+3)
		

#define BLTLR()				BLTLRi(0)  	/* with implicit cr0 */
#define BLELR()				BLELRi(0)  
#define BEQLR()				BEQLRi(0)  
#define BGELR()				BGELRi(0)  
#define BGTLR()				BGTLRi(0)  
#define BNLLR()				BNLLRi(0)  
#define BNELR()				BNELRi(0)  
#define BNGLR()				BNGLRi(0)  
#define BSOLR()				BSOLRi(0)  
#define BNSLR()				BNSLRi(0)  
#define BUNLR()				BUNLRi(0)  
#define BNULR()				BNULRi(0)  
					            
#define BLTLRL()			BLTLRLi(0) 
#define BLELRL()			BLELRLi(0) 
#define BEQLRL()			BEQLRLi(0) 
#define BGELRL()			BGELRLi(0) 
#define BGTLRL()			BGTLRLi(0) 
#define BNLLRL()			BNLLRLi(0) 
#define BNELRL()			BNELRLi(0) 
#define BNGLRL()			BNGLRLi(0) 
#define BSOLRL()			BSOLRLi(0) 
#define BNSLRL()			BNSLRLi(0) 
#define BUNLRL()			BUNLRLi(0) 
#define BNULRL()			BNULRLi(0) 
					            
#define BLTCTR()			BLTCTRi(0) 
#define BLECTR()			BLECTRi(0) 
#define BEQCTR()			BEQCTRi(0) 
#define BGECTR()			BGECTRi(0) 
#define BGTCTR()			BGTCTRi(0) 
#define BNLCTR()			BNLCTRi(0) 
#define BNECTR()			BNECTRi(0) 
#define BNGCTR()			BNGCTRi(0) 
#define BSOCTR()			BSOCTRi(0) 
#define BNSCTR()			BNSCTRi(0) 
#define BUNCTR()			BUNCTRi(0) 
#define BNUCTR()			BNUCTRi(0) 
					            
#define BLTCTRL()			BLTCTRLi(0)
#define BLECTRL()			BLECTRLi(0)
#define BEQCTRL()			BEQCTRLi(0)
#define BGECTRL()			BGECTRLi(0)
#define BGTCTRL()			BGTCTRLi(0)
#define BNLCTRL()			BNLCTRLi(0)
#define BNECTRL()			BNECTRLi(0)
#define BNGCTRL()			BNGCTRLi(0)
#define BSOCTRL()			BSOCTRLi(0)
#define BNSCTRL()			BNSCTRLi(0)
#define BUNCTRL()			BUNCTRLi(0)
#define BNUCTRL()			BNUCTRLi(0)


#define BLTii(C,D)			BCiii(12, ((C)<<2)+0, D)	/* [1, Table F-11] */
#define BLEii(C,D)			BCiii( 4  ((C)<<2)+1, D)
#define BEQii(C,D)			BCiii(12, ((C)<<2)+2, D)
#define BGEii(C,D)			BCiii( 4, ((C)<<2)+0, D)
#define BGTii(C,D)			BCiii(12, ((C)<<2)+1, D)
#define BNLii(C,D)			BCiii( 4, ((C)<<2)+0, D)
#define BNEii(C,D)			BCiii( 4, ((C)<<2)+2, D)
#define BNGii(C,D)			BCiii( 4, ((C)<<2)+1, D)
#define BSOii(C,D)			BCiii(12, ((C)<<2)+3, D)
#define BNSii(C,D)			BCiii( 4, ((C)<<2)+3, D)
#define BUNii(C,D)			BCiii(12, ((C)<<2)+3, D)
#define BNUii(C,D)			BCiii( 4, ((C)<<2)+3, D)

#define BLTi(D)				BLTii(0,D)	/* with implicit cr0 */
#define BLEi(D)				BLEii(0,D)
#define BEQi(D)				BEQii(0,D)
#define BGEi(D)				BGEii(0,D)
#define BGTi(D)				BGTii(0,D)
#define BNLi(D)				BNLii(0,D)
#define BNEi(D)				BNEii(0,D)
#define BNGi(D)				BNGii(0,D)
#define BSOi(D)				BSOii(0,D)
#define BNSi(D)				BNSii(0,D)
#define BUNi(D)				BUNii(0,D)
#define BNUi(D)				BNUii(0,D)

/* Note: there are many tens of other simplified branches that are not (yet?) defined here */

#define CRSETi(BX)			CREQViii(BX, BX, BX)	/* [1, Table F-15] */
#define CRCLRi(BX)			CRXORiii(BX, BX, BX)
#define CRMOVEii(BX,BY)			CRORiii(BX, BY, BY)
#define CRNOTii(BX,BY)			CRNORiii(BX, BY, BY)
		
#define MTLRr(RS)			MTSPRir(8, RS)		/* [1, Table F-20] */
#define MFLRr(RD)			MFSPRri(RD, 8)
#define MTCTRr(RS)			MTSPRir(9, RS)
#define MFCTRr(RD)			MFSPRri(RD, 9)
		
#define NOP()				ORIrri(0, 0, 0)		/* [1, Section F.9] */
#define LIri(RD,IM)			ADDIrri(RD, 0, IM)
#define LISri(RD,IM)			ADDISrri(RD, 0, IM)
#define LArm(RD,D,RA)			ADDIrri(RD, RA, D)
#define LArrr(RD,RB,RA)			ADDIrrr(RD, RA, RB)
#define MRrr(RA,RS)			ORrrr(RA, RS, RS)
#define NOTrr(RA,RS)			NORrrr(RA, RS, RS)

/* alternate parenthesised forms of extended indexed addresses */

#define LBZUXrx(RD,RA,RB)		LBZUXrrr(RD,RA,RB)
#define LBZXrx(RD,RA,RB)		LBZXrrr(RD,RA,RB)
#define LHAUXrx(RD,RA,RB)		LHAUXrrr(RD,RA,RB)
#define LHAXrx(RD,RA,RB)		LHAXrrr(RD,RA,RB)
#define LHBRXrx(RD,RA,RB)		LHBRXrrr(RD,RA,RB)
#define LHZUXrx(RD,RA,RB)		LHZUXrrr(RD,RA,RB)
#define LHZXrx(RD,RA,RB)		LHZXrrr(RD,RA,RB)
#define LWBRXrx(RD,RA,RB)		LWBRXrrr(RD,RA,RB)
#define LWZUXrx(RD, RA, RB)		LWZUXrrr(RD, RA, RB)
#define LWZXrx(RD, RA, RB)		LWZXrrr(RD, RA, RB)
#define STBUXrx(RD,RA,RB)		STBUXrrr(RD,RA,RB)
#define STBXrx(RD,RA,RB)		STBXrrr(RD,RA,RB)
#define STHBRXrx(RS,RA,RB)		STHBRXrrr(RS,RA,RB)
#define STHUXrx(RS,RA,RB)		STHUXrrr(RS,RA,RB)
#define STHXrx(RS,RA,RB)		STHXrrr(RS,RA,RB)
#define STWBRXrx(RS,RA,RB)		STWBRXrrr(RS,RA,RB)
#define STWCXrx(RS,RA,RB)		STWCXrrr(RS,RA,RB)
#define STWCX_rx(RS,RA,RB)		STWCX_rrr(RS,RA,RB)
#define STWUXrx(RS,RA,RB)		STWUXrrr(RS,RA,RB)
#define STWXrx(RS,RA,RB)		STWXrrr(RS,RA,RB)
#define LArx(RD,RB,RA)			LArrr(RD,RB,RA)	


/*** References:
 *
 * [1] "PowerPC Microprocessor Family: The Programming Environments For 32-Bit Microprocessors", Motorola, 1997.
 */


#endif /* __ccg_asm_ppc_h */
