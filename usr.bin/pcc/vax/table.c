/*	$OpenBSD: table.c,v 1.2 2007/10/27 14:19:18 ragge Exp $	*/
/*
 * Copyright(C) Caldera International Inc. 2001-2002. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code and documentation must retain the above
 * copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditionsand the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 * 	This product includes software developed or owned by Caldera
 *	International, Inc.
 * Neither the name of Caldera International, Inc. nor the names of other
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * USE OF THE SOFTWARE PROVIDED FOR UNDER THIS LICENSE BY CALDERA
 * INTERNATIONAL, INC. AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL CALDERA INTERNATIONAL, INC. BE LIABLE
 * FOR ANY DIRECT, INDIRECT INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OFLIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

# include "pass2.h"

# define WPTR TPTRTO|TINT|TLONG|TFLOAT|TDOUBLE|TPOINT|TUNSIGNED|TULONG
# define AWD SNAME|SOREG|SCON|STARNM|STARREG
/* tbl */
# define ANYSIGNED TPOINT|TINT|TLONG|TSHORT|TCHAR
# define ANYUSIGNED TUNSIGNED|TULONG|TUSHORT|TUCHAR
# define ANYFIXED ANYSIGNED|ANYUSIGNED
# define TWORD TINT|TUNSIGNED|TPOINT|TLONG|TULONG
/* tbl */
# define TBREG TLONGLONG|TULONGLONG|TDOUBLE

struct optab  table[] = {
/* First entry must be an empty entry */
{ -1, FOREFF, SANY, TANY, SANY, TANY, 0, 0, "", },

{ PCONV,	INAREG|INAREG,
	SAREG|AWD,	TCHAR|TSHORT,
	SANY,	TPOINT,
		NAREG|NASL,	RESC1,
		"	cvtZLl	AL,A1\n", },

{ PCONV,	INAREG|INAREG,
	SAREG|AWD,	TUCHAR|TUSHORT,
	SANY,	TPOINT,
		NAREG|NASL,	RESC1,
		"	movzZLl	AL,A1\n", },

{ SCONV,	INBREG|FORCC,
	SAREG,	TDOUBLE,
	SANY,	TDOUBLE,
		0,	RLEFT,
		"", },

{ SCONV,	INBREG|FORCC,
	SAREG|AWD,	TANY,
	SANY,	TFLOAT|TDOUBLE,
		NAREG|NASL,	RESC1|RESCC,
		"	cvtZLd	AL,A1\n", },

{ SCONV,	INAREG|FORCC,
	SAREG|AWD,	TFLOAT|TDOUBLE,
	SANY,	ANYFIXED,
		NAREG|NASL,	RESC1|RESCC,
		"	cvtZLZF	AL,A1\n", },

{ SCONV,	INAREG|FORCC,
	SAREG|SNAME|SCON|STARNM,	TANY,
	SANY,	ANYUSIGNED,
		NAREG|NASL,	RESC1|RESCC,
		"	movzZRl	AL,A1\n", },

{ SCONV,	INAREG|FORCC,
	SSOREG,	TANY,
	SANY,	ANYUSIGNED,
		NAREG|NASL,	RESC1|RESCC,
		"	movzZRl	AL,A1\n", },

{ SCONV,	INAREG|FORCC,
	SAREG|SNAME|SCON|STARNM,	TANY,
	SANY,	TANY,
		NAREG|NASL,	RESC1|RESCC,
		"	cvtZRl	AL,A1\n", },

{ SCONV,	INAREG|FORCC,
	SSOREG,	TANY,
	SANY,	TANY,
		NAREG|NASL,	RESC1|RESCC,
		"	cvtZRl	AL,A1\n", },

#if 0
{ INIT,	FOREFF,
	SCON,	TANY,
	SANY,	TWORD,
		0,	RNOP,
		"	.long	CL\n", },

{ INIT,	FOREFF,
	SCON,	TANY,
	SANY,	TSHORT|TUSHORT,
		0,	RNOP,
		"	.word	CL\n", },

{ INIT,	FOREFF,
	SCON,	TANY,
	SANY,	TCHAR|TUCHAR,
		0,	RNOP,
		"	.byte	CL\n", },
#endif

{ GOTO,	FOREFF,
	SCON,	TANY,
	SANY,	TANY,
		0,	RNOP,
		"	jbr	LL\n", },

{ GOTO,	FOREFF,
	SAREG,	TANY,
	SANY,	TANY,
		0,	RNOP,
		"	jmp	(AL)\n", },

{ STARG,	INTEMP,
	SCON|SAREG,	TANY,
	SANY,	TANY,
		NTEMP+2*NAREG,	RESC3,
		"ZS", },

#if 0
{ STASG,	FORARG,
	SNAME|SOREG,	TANY,
	SCON|SAREG,	TANY,
		0,	RNULL,
		"	subl2	ZT,sp\nZS", },
#endif

{ STASG,	FOREFF,
	SNAME|SOREG,	TANY,
	SCON|SAREG,	TANY,
		0,	RNOP,
		"ZS", },

{ STASG,	INAREG,
	SNAME|SOREG,	TANY,
	SCON,	TANY,
		NAREG,	RESC1,
		"ZS	movl	AR,A1\n", },

{ STASG,	INAREG,
	SNAME|SOREG,	TANY,
	SAREG,	TANY,
		0,	RRIGHT,
		"	pushl	AR\nZS	movl	(sp)+,AR\n", },

{ FLD,	INAREG|INAREG,
	SANY,	TANY,
	SFLD,	ANYSIGNED,
		NAREG|NASR,	RESC1,
		"	extv	H,S,AR,A1\n", },

{ FLD,	INAREG|INAREG,
	SANY,	TANY,
	SFLD,	ANYUSIGNED,
		NAREG|NASR,	RESC1,
		"	extzv	H,S,AR,A1\n", },

#if 0
{ FLD,	FORARG,
	SANY,	TANY,
	SFLD,	ANYSIGNED,
		0,	RNULL,
		"	extv	H,S,AR,-(sp)\n", },

{ FLD,	FORARG,
	SANY,	TANY,
	SFLD,	ANYUSIGNED,
		0,	RNULL,
		"	extzv	H,S,AR,-(sp)\n", },
#endif

{ OPLOG,	FORCC,
	SAREG|AWD,	TWORD,
	SAREG|AWD,	TWORD,
		0,	RESCC,
		"	cmpl	AL,AR\n", },

{ OPLOG,	FORCC,
	SAREG|AWD,	TSHORT|TUSHORT,
	SAREG|AWD,	TSHORT|TUSHORT,
		0,	RESCC,
		"	cmpw	AL,AR\n", },

{ OPLOG,	FORCC,
	SAREG|AWD,	TCHAR|TUCHAR,
	SAREG|AWD,	TCHAR|TUCHAR,
		0,	RESCC,
		"	cmpb	AL,AR\n", },

{ OPLOG,	FORCC,
	SAREG|AWD,	TSHORT|TUSHORT,
	SSCON,	TANY,
		0,	RESCC,
		"	cmpw	AL,AR\n", },

{ OPLOG,	FORCC,
	SAREG|AWD,	TCHAR|TUCHAR,
	SCCON,	TANY,
		0,	RESCC,
		"	cmpb	AL,AR\n", },

{ OPLOG,	FORCC,
	SAREG|AWD,	TDOUBLE,
	SAREG|AWD,	TDOUBLE,
		0,	RESCC,
		"	cmpd	AL,AR\n", },

{ OPLOG,	FORCC,
	SAREG|AWD,	TFLOAT|TDOUBLE,
	SAREG|AWD,	TFLOAT|TDOUBLE,
		0,	RESCC,
		"	cmpf	AL,AR\n", },

{ CCODES,	INAREG|INAREG,
	SANY,	TANY,
	SANY,	TANY,
		NAREG,	RESC1,
		"	movl	$1,A1\nZN", },

/*
 * Subroutine calls.
 */

{ CALL,		FOREFF,
	SCON,	TANY,
	SANY,	TANY,
		0,	0,
		"	calls	ZC,CL\n", },

{ UCALL,	FOREFF,
	SCON,	TANY,
	SANY,	TANY,
		0,	0,
		"	calls	$0,CL\n", },

{ CALL,		INAREG,
	SCON,	TANY,
	SANY,	TANY,
		NAREG|NASL,	RESC1, /* should be register 0 */
		"	calls	ZC,CL\n", },

{ UCALL,	INAREG,
	SCON,	TANY,
	SANY,	TANY,
		NAREG|NASL,	RESC1, /* should be register 0 */
		"	calls	$0,CL\n", },

{ UCALL,	INAREG|FOREFF,
	SAREG,	TANY,
	SANY,	TWORD|TCHAR|TUCHAR|TSHORT|TUSHORT|TFLOAT|TDOUBLE,
		NAREG|NASL,	RESC1,	/* should be 0 */
		"	calls	ZC,(AL)\n", },

{ UCALL,	INAREG|FOREFF,
	SNAME,	TANY,
	SANY,	TANY,
		NAREG|NASL,	RESC1,	/* really reg 0 */
		"	calls	ZC,*AL\n", },

{ UCALL,	INAREG|FOREFF,
	SSOREG,	TANY,
	SANY,	TANY,
		NAREG|NASL,	RESC1,	/* really reg 0 */
		"	calls	ZC,*AL\n", },

/*
 * Function arguments
 */
{ FUNARG,	FOREFF,
	SNCON,	TWORD|TPOINT,
	SANY,	TWORD|TPOINT,
		0,	RNULL,
		"	pushab AL\n" },

{ FUNARG,	FOREFF,
	SCON|SAREG|SNAME|SOREG,	TWORD|TPOINT,
	SANY,	TWORD|TPOINT,
		0,	RNULL,
		"	pushl AL\n" },

#if 0
{ ASG RS,	INAREG|FOREFF|FORCC,
	SAREG,	TWORD,
	SCON,	TINT,
		0,	RLEFT|RESCC,
		"	extzv	AR,ZU,AL,AL\n", },

{ ASG RS,	INAREG|FOREFF|FORCC,
	SAREG,	TWORD,
	SAREG,	ANYFIXED,
		NAREG,	RLEFT|RESCC,
		"	subl3	AR,$32,A1\n	extzv	AR,A1,AL,AL\n", },

{ ASG RS,	INAREG|FOREFF|FORCC,
	SAREG,	TWORD,
	SAREG|AWD,	TWORD,
		NAREG,	RLEFT|RESCC,
		"	subl3	AR,$32,A1\n	extzv	AR,A1,AL,AL\n", },
#endif

{ RS,	INAREG|INAREG|FORCC,
	SAREG,	TWORD,
	SCON,	TINT,
		NAREG|NASL,	RESC1|RESCC,
		"	extzv	AR,ZU,AL,A1\n", },

#if 0
{ ASG LS,	INAREG|FOREFF|FORCC,
	SAREG|AWD,	TWORD,
	SAREG|AWD,	ANYSIGNED|ANYUSIGNED,
		0,	RLEFT|RESCC,
		"	ashl	AR,AL,AL\n", },
#endif

{ LS,	INAREG|INAREG|FORCC,
	SAREG|AWD,	TWORD,
	SAREG|AWD,	ANYSIGNED|ANYUSIGNED,
		NAREG|NASL|NASR,	RESC1|RESCC,
		"	ashl	AR,AL,A1\n", },

#if 0
{ INCR,	FOREFF,
	SAREG|AWD,	TANY,
	SANY,	TANY,
		0,	RLEFT,
		"	ZE\n", },

{ DECR,	FOREFF,
	SAREG|AWD,	TANY,
	SCON,	TANY,
		0,	RLEFT,
		"	ZE\n", },

{ INCR,	INAREG|INAREG,
	SAREG|AWD,	TANY,
	SCON,	TANY,
		NAREG,	RESC1,
		"	ZD\n", },

{ DECR,	INAREG|INAREG,
	SAREG|AWD,	TANY,
	SCON,	TANY,
		NAREG,	RESC1,
		"	ZD\n", },
#endif

{ ASSIGN,	INBREG|FOREFF,
	SBREG|AWD,	TBREG,
	SBREG|AWD,	TBREG,
		0,	RDEST,
		"	movq AR,AL\n", },

{ ASSIGN,	INAREG|FOREFF|FORCC,
	SAREG|AWD,	TANY,
	SAREG|AWD,	TANY,
		0,	RDEST|RESCC,
		"	ZA\n", },

{ ASSIGN,	INAREG|FOREFF|FORCC,
	SFLD,	TANY,
	SAREG|AWD,	TWORD,
		0,	RDEST|RESCC,
		"	insv	AR,H,S,AL\n", },

{ ASSIGN,	INAREG|FOREFF|FORCC,
	SAREG|AWD,	TWORD,
	SFLD,	ANYSIGNED,
		0,	RDEST|RESCC,
		"	extv	H,S,AR,AL\n", },

{ ASSIGN,	INAREG|FOREFF|FORCC,
	SAREG|AWD,	TWORD,
	SFLD,	ANYUSIGNED,
		0,	RDEST|RESCC,
		"	extzv	H,S,AR,AL\n", },

/* dummy UNARY MUL entry to get U* to possibly match OPLTYPE */
{ UMUL,	FOREFF,
	SCC,	TANY,
	SCC,	TANY,
		0,	RNULL,
		"	HELP HELP HELP\n", },

#if 0
{ REG,	FORARG,
	SANY,	TANY,
	SAREG,	TDOUBLE|TFLOAT,
		0,	RNULL,
		"	movZR	AR,-(sp)\n", },
#endif

{ REG,	INTEMP,
	SANY,	TANY,
	SAREG,	TDOUBLE,
		2*NTEMP,	RESC1,
		"	movd	AR,A1\n", },

{ REG,	INTEMP,
	SANY,	TANY,
	SAREG,	TANY,
		NTEMP,	RESC1,
		"	movZF	AR,A1\n", },

{ OPLEAF,	FOREFF,
	SANY,	TANY,
	SAREG|AWD,	TANY,
		0,	RLEFT,
		"", },

{ OPLTYPE,	INAREG|INAREG,
	SANY,	TANY,
	SANY,	TFLOAT|TDOUBLE,
		2*NAREG|NASR,	RESC1,
		"	ZA\n", },

{ OPLTYPE,	INAREG|INAREG,
	SANY,	TANY,
	SANY,	TANY,
		NAREG|NASR,	RESC1,
		"	ZA\n", },

{ OPLTYPE,	FORCC,
	SANY,	TANY,
	SANY,	TANY,
		0,	RESCC,
		"	tstZR	AR\n", },

#if 0
{ OPLTYPE,	FORARG,
	SANY,	TANY,
	SANY,	TWORD,
		0,	RNULL,
		"	pushl	AR\n", },

{ OPLTYPE,	FORARG,
	SANY,	TANY,
	SANY,	TCHAR|TSHORT,
		0,	RNULL,
		"	cvtZRl	AR,-(sp)\n", },

{ OPLTYPE,	FORARG,
	SANY,	TANY,
	SANY,	TUCHAR|TUSHORT,
		0,	RNULL,
		"	movzZRl	AR,-(sp)\n", },

{ OPLTYPE,	FORARG,
	SANY,	TANY,
	SANY,	TDOUBLE,
		0,	RNULL,
		"	movd	AR,-(sp)\n", },

{ OPLTYPE,	FORARG,
	SANY,	TANY,
	SANY,	TFLOAT,
		0,	RNULL,
		"	cvtfd	AR,-(sp)\n", },
#endif

{ UMINUS,	INAREG|FORCC,
	SAREG|AWD,	TINT|TUNSIGNED|TLONG|TULONG|TDOUBLE,
	SANY,	TANY,
		NAREG|NASL,	RESC1|RESCC,
		"	mnegZL	AL,A1\n", },

{ COMPL,	INAREG|FORCC,
	SAREG|AWD,	TINT|TUNSIGNED,
	SANY,	TANY,
		NAREG|NASL,	RESC1|RESCC,
		"	mcomZL	AL,A1\n", },

{ COMPL,	INAREG|FORCC,
	SAREG|AWD,	ANYSIGNED|ANYUSIGNED,
	SANY,	TANY,
		NAREG|NASL,	RESC1|RESCC,
		"	cvtZLl	AL,A1\n	mcoml	A1,A1\n", },

{ AND,	FORCC,
	SAREG|AWD,	TWORD,
	SCON,	TWORD,
		0,	RESCC,
		"	bitl	ZZ,AL\n", },

{ AND,	FORCC,
	SAREG|AWD,	TSHORT|TUSHORT,
	SSCON,	TWORD,
		0,	RESCC,
		"	bitw	ZZ,AL\n", },

{ AND,	FORCC,
	SAREG|AWD,	TCHAR|TUCHAR,
	SCCON,	TWORD,
		0,	RESCC,
		"	bitb	ZZ,AL\n", },

#if 0
{ ASG AND,	INAREG|FOREFF|FORCC,
	SAREG,	ANYFIXED,
	SCON,	TWORD,
		0,	RLEFT|RESCC,
		"	bicl2	AR,AL\n", },

{ ASG OPMUL,	INAREG|FOREFF|FORCC,
	SAREG|AWD,	TINT|TUNSIGNED|TLONG|TULONG,
	SAREG|AWD,	TINT|TUNSIGNED|TLONG|TULONG,
		0,	RLEFT|RESCC,
		"	OL2	AR,AL\n", },
#endif

{ OPMUL,	INAREG|INAREG|FORCC,
	SAREG,	TINT|TUNSIGNED|TLONG|TULONG,
	SAREG|AWD,	TINT|TUNSIGNED|TLONG|TULONG,
		0,	RLEFT|RESCC,
		"	OL2	AR,AL\n", },

{ OPMUL,	INAREG|INAREG|FORCC,
	SAREG|AWD,	TINT|TUNSIGNED|TLONG|TULONG,
	SAREG|AWD,	TINT|TUNSIGNED|TLONG|TULONG,
		NAREG|NASL|NASR,	RESC1|RESCC,
		"	OL3	AR,AL,A1\n", },

#if 0
{ ASG MOD,	INAREG|INAREG|FOREFF|FORCC,
	SAREG,	TINT|TUNSIGNED|TLONG|TULONG,
	SAREG|AWD,	TINT|TUNSIGNED|TLONG|TULONG,
		NAREG,	RLEFT|RESCC,
		"	divl3	AR,AL,A1\n	mull2	AR,A1\n	subl2	A1,AL\n", },
#endif

{ MOD,	INAREG|INAREG,
	SAREG|AWD,	TINT|TUNSIGNED|TLONG|TULONG,
	SAREG|AWD,	TINT|TUNSIGNED|TLONG|TULONG,
		NAREG,	RESC1,
		"	divl3	AR,AL,A1\n	mull2	AR,A1\n	subl3	A1,AL,A1\n", },

#if 0
{ ASG PLUS,	INAREG|FOREFF|FORCC,
	SAREG|AWD,	ANYSIGNED|ANYUSIGNED,
	SONE,	TINT|TLONG,
		0,	RLEFT|RESCC,
		"	incZL	AL\n", },

{ ASG MINUS,	INAREG|FOREFF|FORCC,
	SAREG|AWD,	ANYSIGNED|ANYUSIGNED,
	SONE,	TINT|TLONG,
		0,	RLEFT|RESCC,
		"	decZL	AL\n", },
#endif

{ PLUS,	INAREG|INAREG|FORCC,
	SAREG,	ANYFIXED,
	SONE,	TWORD,
		0,	RLEFT|RESCC,
		"	incZL	AL\n", },

{ MINUS,	INAREG|INAREG|FORCC,
	SAREG,	ANYFIXED,
	SONE,	TWORD,
		0,	RLEFT|RESCC,
		"	decZL	AL\n", },

#if 0
{ ASG OPSIMP,	INAREG|FOREFF|FORCC,
	SAREG|AWD,	TWORD,
	SAREG|AWD,	TWORD,
		0,	RLEFT|RESCC,
		"	OL2	AR,AL\n", },

{ ASG OPSIMP,	INAREG|FOREFF|FORCC,
	AWD,	TSHORT|TUSHORT,
	SAREG|AWD,	TSHORT|TUSHORT,
		0,	RLEFT|RESCC,
		"	OW2	AR,AL\n", },

{ ASG OPSIMP,	INAREG|FOREFF|FORCC,
	AWD,	TSHORT|TUSHORT,
	SSCON,	TWORD,
		0,	RLEFT|RESCC,
		"	OW2	AR,AL\n", },

{ ASG OPSIMP,	INAREG|FOREFF|FORCC,
	AWD,	TCHAR|TUCHAR,
	SAREG|AWD,	TCHAR|TUCHAR,
		0,	RLEFT|RESCC,
		"	OB2	AR,AL\n", },

{ ASG OPSIMP,	INAREG|FOREFF|FORCC,
	AWD,	TCHAR|TUCHAR,
	SCCON,	TWORD,
		0,	RLEFT|RESCC,
		"	OB2	AR,AL\n", },
#endif

{ OPSIMP,	INAREG|INAREG|FORCC,
	SAREG,	ANYFIXED,
	SAREG|AWD,	TWORD,
		0,	RLEFT|RESCC,
		"	OL2	AR,AL\n", },

{ OPSIMP,	INAREG|INAREG|FORCC,
	SAREG|AWD,	TWORD,
	SAREG|AWD,	TWORD,
		NAREG|NASL|NASR,	RESC1|RESCC,
		"	OL3	AR,AL,A1\n", },

#if 0
{ ASG OPFLOAT,	INAREG|FOREFF|FORCC,
	SAREG|AWD,	TDOUBLE,
	SAREG|AWD,	TDOUBLE,
		0,	RLEFT|RESCC,
		"	OD2	AR,AL\n", },

{ ASG OPFLOAT,	INAREG|FOREFF|FORCC,
	SAREG|AWD,	TFLOAT,
	SAREG|AWD,	TFLOAT,
		0,	RLEFT|RESCC,
		"	OF2	AR,AL\n", },

{ ASG OPFLOAT,	INAREG|FOREFF|FORCC,
	SAREG|AWD,	TDOUBLE,
	SAREG|AWD,	TFLOAT,
		NAREG|NASR,	RLEFT|RESCC,
		"	cvtfd	AR,A1\n	OD2	A1,AL\n", },

{ ASG OPFLOAT,	INAREG|INAREG|FOREFF|FORCC,
	SAREG|AWD,	TFLOAT,
	SAREG|AWD,	TDOUBLE,
		NAREG,	RLEFT|RESC1|RESCC,
		"	cvtfd	AL,A1\n	OD2	AR,A1\n	cvtdf	A1,AL\n", },
#endif

{ OPFLOAT,	INAREG|INAREG|FORCC,
	SAREG,	TDOUBLE,
	SAREG|AWD,	TDOUBLE,
		0,	RLEFT|RESCC,
		"	OD2	AR,AL\n", },

{ OPFLOAT,	INAREG|INAREG|FORCC,
	SAREG|AWD,	TDOUBLE,
	SAREG|AWD,	TDOUBLE,
		NAREG|NASL|NASR,	RESC1|RESCC,
		"	OD3	AR,AL,A1\n", },

{ OPFLOAT,	INAREG|INAREG|FORCC,
	SAREG|AWD,	TFLOAT,
	SAREG|AWD,	TDOUBLE,
		NAREG|NASL,	RESC1|RESCC,
		"	cvtfd	AL,A1\n	OD2	AR,A1\n", },

{ OPFLOAT,	INAREG|INAREG|FORCC,
	SAREG|AWD,	TDOUBLE,
	SAREG|AWD,	TFLOAT,
		NAREG|NASR,	RESC1|RESCC,
		"	cvtfd	AR,A1\n	OD3	A1,AL,A1\n", },

{ OPFLOAT,	INAREG|INAREG|FORCC,
	SAREG|AWD,	TFLOAT,
	SAREG|AWD,	TFLOAT,
		NAREG|NASL|NASR,	RESC1|RESCC,
		"	OF3	AR,AL,A1\n	cvtfd	A1,A1\n", },

	/* Default actions for hard trees ... */

# define DF(x) FORREW,SANY,TANY,SANY,TANY,REWRITE,x,""

{ UMUL, DF( UMUL ), },

{ ASSIGN, DF(ASSIGN), },

{ STASG, DF(STASG), },

{ OPLEAF, DF(NAME), },

{ OPLOG,	FORCC,
	SANY,	TANY,
	SANY,	TANY,
		REWRITE,	BITYPE,
		"", },

{ OPUNARY, DF(UMINUS), },

{ OPANY, DF(BITYPE), },

{ FREE,	FREE,	FREE,	FREE,	FREE,	FREE,	FREE,	FREE,	"help; I'm in trouble\n" }
};
