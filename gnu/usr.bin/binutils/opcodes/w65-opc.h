/* Instruction opcode header for WDC 65816 
   (generated by the program sim/w65/gencode -a)

Copyright 2001 Free Software Foundation, Inc.

This file is part of the GNU Binutils and/or GDB, the GNU debugger.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

#define ADDR_IMMTOA              1	 /* #a      */
#define ADDR_IMMCOP              2	 /* #c      */
#define ADDR_IMMTOI              3	 /* #i      */
#define ADDR_ACC                 4	 /* A       */
#define ADDR_PC_REL              5	 /* r       */
#define ADDR_PC_REL_LONG         6	 /* rl      */
#define ADDR_IMPLIED             7	 /* i       */
#define ADDR_STACK               8	 /* s       */
#define ADDR_DIR                 9	 /* d       */
#define ADDR_DIR_IDX_X           10	 /* d,x     */
#define ADDR_DIR_IDX_Y           11	 /* d,y     */
#define ADDR_DIR_IND             12	 /* (d)     */
#define ADDR_DIR_IDX_IND_X       13	 /* (d,x)   */
#define ADDR_DIR_IND_IDX_Y       14	 /* (d),y   */
#define ADDR_DIR_IND_LONG        15	 /* [d]     */
#define ADDR_DIR_IND_IDX_Y_LONG  16	 /* [d],y   */
#define ADDR_ABS                 17	 /* a       */
#define ADDR_ABS_IDX_X           18	 /* a,x     */
#define ADDR_ABS_IDX_Y           19	 /* a,y     */
#define ADDR_ABS_LONG            20	 /* al      */
#define ADDR_ABS_IND_LONG        21	 /* [a]     */
#define ADDR_ABS_LONG_IDX_X      22	 /* al,x    */
#define ADDR_STACK_REL           23	 /* d,s     */
#define ADDR_STACK_REL_INDX_IDX  24	 /* (d,s),y */
#define ADDR_ABS_IND             25	 /* (a)     */
#define ADDR_ABS_IND_IDX         26	 /* (a,x)   */
#define ADDR_BLOCK_MOVE          27	 /* xyz     */
struct opinfo {
	int val;
	int code;
	char *name;
	int amode;
};
struct opinfo optable[257]={
#define O_adc 1
#define O_and 2
#define O_asl 3
#define O_bcc 4
#define O_bcs 5
#define O_beq 6
#define O_bit 7
#define O_bmi 8
#define O_bne 9
#define O_bpl 10
#define O_bra 11
#define O_brk 12
#define O_brl 13
#define O_bvc 14
#define O_bvs 15
#define O_clc 16
#define O_cld 17
#define O_cli 18
#define O_clv 19
#define O_cmp 20
#define O_cop 21
#define O_cpx 22
#define O_cpy 23
#define O_dec 24
#define O_dex 25
#define O_dey 26
#define O_eor 27
#define O_inc 28
#define O_inx 29
#define O_iny 30
#define O_jmp 31
#define O_jsr 32
#define O_lda 33
#define O_ldx 34
#define O_ldy 35
#define O_lsr 36
#define O_mvn 37
#define O_mvp 38
#define O_nop 39
#define O_ora 40
#define O_pea 41
#define O_pei 42
#define O_per 43
#define O_pha 44
#define O_phb 45
#define O_phd 46
#define O_phk 47
#define O_php 48
#define O_phx 49
#define O_phy 50
#define O_pla 51
#define O_plb 52
#define O_pld 53
#define O_plp 54
#define O_plx 55
#define O_ply 56
#define O_rep 57
#define O_rol 58
#define O_ror 59
#define O_rti 60
#define O_rtl 61
#define O_rts 62
#define O_sbc 63
#define O_sec 64
#define O_sed 65
#define O_sei 66
#define O_sep 67
#define O_sta 68
#define O_stp 69
#define O_stx 70
#define O_sty 71
#define O_stz 72
#define O_tax 73
#define O_tay 74
#define O_tcd 75
#define O_tcs 76
#define O_tdc 77
#define O_trb 78
#define O_tsb 79
#define O_tsc 80
#define O_tsx 81
#define O_txa 82
#define O_txs 83
#define O_txy 84
#define O_tya 85
#define O_tyx 86
#define O_wai 87
#define O_wdm 88
#define O_xba 89
#define O_xce 90
#ifdef DEFINE_TABLE
	{0x69,	O_adc,	"adc",	ADDR_IMMTOA},
	{0x72,	O_adc,	"adc",	ADDR_DIR_IND},
	{0x71,	O_adc,	"adc",	ADDR_DIR_IND_IDX_Y},
	{0x73,	O_adc,	"adc",	ADDR_STACK_REL_INDX_IDX},
	{0x61,	O_adc,	"adc",	ADDR_DIR_IDX_IND_X},
	{0x67,	O_adc,	"adc",	ADDR_DIR_IND_LONG},
	{0x77,	O_adc,	"adc",	ADDR_DIR_IND_IDX_Y_LONG},
	{0x6D,	O_adc,	"adc",	ADDR_ABS},
	{0x7D,	O_adc,	"adc",	ADDR_ABS_IDX_X},
	{0x79,	O_adc,	"adc",	ADDR_ABS_IDX_Y},
	{0x6F,	O_adc,	"adc",	ADDR_ABS_LONG},
	{0x7F,	O_adc,	"adc",	ADDR_ABS_LONG_IDX_X},
	{0x65,	O_adc,	"adc",	ADDR_DIR},
	{0x63,	O_adc,	"adc",	ADDR_STACK_REL},
	{0x75,	O_adc,	"adc",	ADDR_DIR_IDX_X},
	{0x29,	O_and,	"and",	ADDR_IMMTOA},
	{0x32,	O_and,	"and",	ADDR_DIR_IND},
	{0x31,	O_and,	"and",	ADDR_DIR_IND_IDX_Y},
	{0x33,	O_and,	"and",	ADDR_STACK_REL_INDX_IDX},
	{0x21,	O_and,	"and",	ADDR_DIR_IDX_IND_X},
	{0x27,	O_and,	"and",	ADDR_DIR_IND_LONG},
	{0x37,	O_and,	"and",	ADDR_DIR_IND_IDX_Y_LONG},
	{0x2D,	O_and,	"and",	ADDR_ABS},
	{0x3D,	O_and,	"and",	ADDR_ABS_IDX_X},
	{0x39,	O_and,	"and",	ADDR_ABS_IDX_Y},
	{0x2F,	O_and,	"and",	ADDR_ABS_LONG},
	{0x3F,	O_and,	"and",	ADDR_ABS_LONG_IDX_X},
	{0x25,	O_and,	"and",	ADDR_DIR},
	{0x23,	O_and,	"and",	ADDR_STACK_REL},
	{0x35,	O_and,	"and",	ADDR_DIR_IDX_X},
	{0x0A,	O_asl,	"asl",	ADDR_ACC},
	{0x0E,	O_asl,	"asl",	ADDR_ABS},
	{0x1E,	O_asl,	"asl",	ADDR_ABS_IDX_X},
	{0x06,	O_asl,	"asl",	ADDR_DIR},
	{0x16,	O_asl,	"asl",	ADDR_DIR_IDX_X},
	{0x90,	O_bcc,	"bcc",	ADDR_PC_REL},
	{0xB0,	O_bcs,	"bcs",	ADDR_PC_REL},
	{0xF0,	O_beq,	"beq",	ADDR_PC_REL},
	{0x89,	O_bit,	"bit",	ADDR_IMMTOA},
	{0x24,	O_bit,	"bit",	ADDR_DIR_IND},
	{0x34,	O_bit,	"bit",	ADDR_DIR_IDX_IND_X},
	{0x2C,	O_bit,	"bit",	ADDR_ABS},
	{0x3C,	O_bit,	"bit",	ADDR_ABS_IDX_X},
	{0x30,	O_bmi,	"bmi",	ADDR_PC_REL},
	{0xD0,	O_bne,	"bne",	ADDR_PC_REL},
	{0x10,	O_bpl,	"bpl",	ADDR_PC_REL},
	{0x80,	O_bra,	"bra",	ADDR_PC_REL},
	{0x00,	O_brk,	"brk",	ADDR_STACK},
	{0x82,	O_brl,	"brl",	ADDR_PC_REL_LONG},
	{0x50,	O_bvc,	"bvc",	ADDR_PC_REL},
	{0x70,	O_bvs,	"bvs",	ADDR_PC_REL},
	{0x18,	O_clc,	"clc",	ADDR_IMPLIED},
	{0xD8,	O_cld,	"cld",	ADDR_IMPLIED},
	{0x58,	O_cli,	"cli",	ADDR_IMPLIED},
	{0xB8,	O_clv,	"clv",	ADDR_IMPLIED},
	{0xC9,	O_cmp,	"cmp",	ADDR_IMMTOA},
	{0xD2,	O_cmp,	"cmp",	ADDR_DIR_IND},
	{0xD1,	O_cmp,	"cmp",	ADDR_DIR_IND_IDX_Y},
	{0xD3,	O_cmp,	"cmp",	ADDR_STACK_REL_INDX_IDX},
	{0xC1,	O_cmp,	"cmp",	ADDR_DIR_IDX_IND_X},
	{0xC7,	O_cmp,	"cmp",	ADDR_DIR_IND_LONG},
	{0xD7,	O_cmp,	"cmp",	ADDR_DIR_IND_IDX_Y_LONG},
	{0xCD,	O_cmp,	"cmp",	ADDR_ABS},
	{0xDD,	O_cmp,	"cmp",	ADDR_ABS_IDX_X},
	{0xD9,	O_cmp,	"cmp",	ADDR_ABS_IDX_Y},
	{0xCF,	O_cmp,	"cmp",	ADDR_ABS_LONG},
	{0xDF,	O_cmp,	"cmp",	ADDR_ABS_LONG_IDX_X},
	{0xC5,	O_cmp,	"cmp",	ADDR_DIR},
	{0xC3,	O_cmp,	"cmp",	ADDR_STACK_REL},
	{0xD5,	O_cmp,	"cmp",	ADDR_DIR_IDX_X},
	{0x02,	O_cop,	"cop",	ADDR_IMMCOP},
	{0xE0,	O_cpx,	"cpx",	ADDR_IMMTOI},
	{0xEC,	O_cpx,	"cpx",	ADDR_ABS},
	{0xE4,	O_cpx,	"cpx",	ADDR_DIR},
	{0xC0,	O_cpy,	"cpy",	ADDR_IMMTOI},
	{0xCC,	O_cpy,	"cpy",	ADDR_ABS},
	{0xC4,	O_cpy,	"cpy",	ADDR_DIR},
	{0x3A,	O_dec,	"dec",	ADDR_ACC},
	{0xCE,	O_dec,	"dec",	ADDR_ABS},
	{0xDE,	O_dec,	"dec",	ADDR_ABS_IDX_X},
	{0xC6,	O_dec,	"dec",	ADDR_DIR},
	{0xD6,	O_dec,	"dec",	ADDR_DIR_IDX_X},
	{0xCA,	O_dex,	"dex",	ADDR_IMPLIED},
	{0x88,	O_dey,	"dey",	ADDR_IMPLIED},
	{0x49,	O_eor,	"eor",	ADDR_IMMTOA},
	{0x52,	O_eor,	"eor",	ADDR_DIR_IND},
	{0x51,	O_eor,	"eor",	ADDR_DIR_IND_IDX_Y},
	{0x53,	O_eor,	"eor",	ADDR_STACK_REL_INDX_IDX},
	{0x41,	O_eor,	"eor",	ADDR_DIR_IDX_IND_X},
	{0x47,	O_eor,	"eor",	ADDR_DIR_IND_LONG},
	{0x57,	O_eor,	"eor",	ADDR_DIR_IND_IDX_Y_LONG},
	{0x4D,	O_eor,	"eor",	ADDR_ABS},
	{0x5D,	O_eor,	"eor",	ADDR_ABS_IDX_X},
	{0x59,	O_eor,	"eor",	ADDR_ABS_IDX_Y},
	{0x4F,	O_eor,	"eor",	ADDR_ABS_LONG},
	{0x5F,	O_eor,	"eor",	ADDR_ABS_LONG_IDX_X},
	{0x45,	O_eor,	"eor",	ADDR_DIR},
	{0x43,	O_eor,	"eor",	ADDR_STACK_REL},
	{0x55,	O_eor,	"eor",	ADDR_DIR_IDX_X},
	{0x1A,	O_inc,	"inc",	ADDR_ACC},
	{0xEE,	O_inc,	"inc",	ADDR_ABS},
	{0xFE,	O_inc,	"inc",	ADDR_ABS_IDX_X},
	{0xE6,	O_inc,	"inc",	ADDR_DIR},
	{0xF6,	O_inc,	"inc",	ADDR_DIR_IDX_X},
	{0xE8,	O_inx,	"inx",	ADDR_IMPLIED},
	{0xC8,	O_iny,	"iny",	ADDR_IMPLIED},
	{0x6C,	O_jmp,	"jmp",	ADDR_ABS_IND},
	{0x7C,	O_jmp,	"jmp",	ADDR_ABS_IND_IDX},
	{0xDC,	O_jmp,	"jmp",	ADDR_ABS_IND_LONG},
	{0x4C,	O_jmp,	"jmp",	ADDR_ABS},
	{0x5C,	O_jmp,	"jmp",	ADDR_ABS_LONG},
	{0xFC,	O_jsr,	"jsr",	ADDR_ABS_IND_IDX},
	{0x20,	O_jsr,	"jsr",	ADDR_ABS},
	{0x22,	O_jsr,	"jsr",	ADDR_ABS_LONG},
	{0xA9,	O_lda,	"lda",	ADDR_IMMTOA},
	{0xB2,	O_lda,	"lda",	ADDR_DIR_IND},
	{0xB1,	O_lda,	"lda",	ADDR_DIR_IND_IDX_Y},
	{0xB3,	O_lda,	"lda",	ADDR_STACK_REL_INDX_IDX},
	{0xA1,	O_lda,	"lda",	ADDR_DIR_IDX_IND_X},
	{0xA7,	O_lda,	"lda",	ADDR_DIR_IND_LONG},
	{0xB7,	O_lda,	"lda",	ADDR_DIR_IND_IDX_Y_LONG},
	{0xAD,	O_lda,	"lda",	ADDR_ABS},
	{0xBD,	O_lda,	"lda",	ADDR_ABS_IDX_X},
	{0xB9,	O_lda,	"lda",	ADDR_ABS_IDX_Y},
	{0xAF,	O_lda,	"lda",	ADDR_ABS_LONG},
	{0xBF,	O_lda,	"lda",	ADDR_ABS_LONG_IDX_X},
	{0xA5,	O_lda,	"lda",	ADDR_DIR},
	{0xA3,	O_lda,	"lda",	ADDR_STACK_REL},
	{0xB5,	O_lda,	"lda",	ADDR_DIR_IDX_X},
	{0xA2,	O_ldx,	"ldx",	ADDR_IMMTOI},
	{0xAE,	O_ldx,	"ldx",	ADDR_ABS},
	{0xBE,	O_ldx,	"ldx",	ADDR_ABS_IDX_Y},
	{0xA6,	O_ldx,	"ldx",	ADDR_DIR},
	{0xB6,	O_ldx,	"ldx",	ADDR_DIR_IDX_Y},
	{0xA0,	O_ldy,	"ldy",	ADDR_IMMTOI},
	{0xAC,	O_ldy,	"ldy",	ADDR_ABS},
	{0xBC,	O_ldy,	"ldy",	ADDR_ABS_IDX_X},
	{0xA4,	O_ldy,	"ldy",	ADDR_DIR},
	{0xB4,	O_ldy,	"ldy",	ADDR_DIR_IDX_X},
	{0x4A,	O_lsr,	"lsr",	ADDR_ACC},
	{0x4E,	O_lsr,	"lsr",	ADDR_ABS},
	{0x5E,	O_lsr,	"lsr",	ADDR_ABS_IDX_X},
	{0x46,	O_lsr,	"lsr",	ADDR_DIR},
	{0x56,	O_lsr,	"lsr",	ADDR_DIR_IDX_X},
	{0x54,	O_mvn,	"mvn",	ADDR_BLOCK_MOVE},
	{0x44,	O_mvp,	"mvp",	ADDR_BLOCK_MOVE},
	{0xEA,	O_nop,	"nop",	ADDR_IMPLIED},
	{0x09,	O_ora,	"ora",	ADDR_IMMTOA},
	{0x12,	O_ora,	"ora",	ADDR_DIR_IND},
	{0x11,	O_ora,	"ora",	ADDR_DIR_IND_IDX_Y},
	{0x13,	O_ora,	"ora",	ADDR_STACK_REL_INDX_IDX},
	{0x01,	O_ora,	"ora",	ADDR_DIR_IDX_IND_X},
	{0x07,	O_ora,	"ora",	ADDR_DIR_IND_LONG},
	{0x17,	O_ora,	"ora",	ADDR_DIR_IND_IDX_Y_LONG},
	{0x0D,	O_ora,	"ora",	ADDR_ABS},
	{0x1D,	O_ora,	"ora",	ADDR_ABS_IDX_X},
	{0x19,	O_ora,	"ora",	ADDR_ABS_IDX_Y},
	{0x0F,	O_ora,	"ora",	ADDR_ABS_LONG},
	{0x1F,	O_ora,	"ora",	ADDR_ABS_LONG_IDX_X},
	{0x05,	O_ora,	"ora",	ADDR_DIR},
	{0x03,	O_ora,	"ora",	ADDR_STACK_REL},
	{0x15,	O_ora,	"ora",	ADDR_DIR_IDX_X},
	{0xF4,	O_pea,	"pea",	ADDR_ABS},
	{0xD4,	O_pei,	"pei",	ADDR_DIR},
	{0x62,	O_per,	"per",	ADDR_PC_REL_LONG},
	{0x48,	O_pha,	"pha",	ADDR_STACK},
	{0x8B,	O_phb,	"phb",	ADDR_STACK},
	{0x0B,	O_phd,	"phd",	ADDR_STACK},
	{0x4B,	O_phk,	"phk",	ADDR_STACK},
	{0x08,	O_php,	"php",	ADDR_STACK},
	{0xDA,	O_phx,	"phx",	ADDR_STACK},
	{0x5A,	O_phy,	"phy",	ADDR_STACK},
	{0x68,	O_pla,	"pla",	ADDR_STACK},
	{0xAB,	O_plb,	"plb",	ADDR_STACK},
	{0x2B,	O_pld,	"pld",	ADDR_STACK},
	{0x28,	O_plp,	"plp",	ADDR_STACK},
	{0xFA,	O_plx,	"plx",	ADDR_STACK},
	{0x7A,	O_ply,	"ply",	ADDR_STACK},
	{0xC2,	O_rep,	"rep",	ADDR_IMMCOP},
	{0x2A,	O_rol,	"rol",	ADDR_ACC},
	{0x2E,	O_rol,	"rol",	ADDR_ABS},
	{0x3E,	O_rol,	"rol",	ADDR_ABS_IDX_X},
	{0x26,	O_rol,	"rol",	ADDR_DIR},
	{0x36,	O_rol,	"rol",	ADDR_DIR_IDX_X},
	{0x6A,	O_ror,	"ror",	ADDR_ACC},
	{0x6E,	O_ror,	"ror",	ADDR_ABS},
	{0x7E,	O_ror,	"ror",	ADDR_ABS_IDX_X},
	{0x66,	O_ror,	"ror",	ADDR_DIR},
	{0x76,	O_ror,	"ror",	ADDR_DIR_IDX_X},
	{0x40,	O_rti,	"rti",	ADDR_STACK},
	{0x6B,	O_rtl,	"rtl",	ADDR_STACK},
	{0x60,	O_rts,	"rts",	ADDR_STACK},
	{0xE9,	O_sbc,	"sbc",	ADDR_IMMTOA},
	{0xF2,	O_sbc,	"sbc",	ADDR_DIR_IND},
	{0xF1,	O_sbc,	"sbc",	ADDR_DIR_IND_IDX_Y},
	{0xF3,	O_sbc,	"sbc",	ADDR_STACK_REL_INDX_IDX},
	{0xE1,	O_sbc,	"sbc",	ADDR_DIR_IDX_IND_X},
	{0xE7,	O_sbc,	"sbc",	ADDR_DIR_IND_LONG},
	{0xF7,	O_sbc,	"sbc",	ADDR_DIR_IND_IDX_Y_LONG},
	{0xED,	O_sbc,	"sbc",	ADDR_ABS},
	{0xFD,	O_sbc,	"sbc",	ADDR_ABS_IDX_X},
	{0xF9,	O_sbc,	"sbc",	ADDR_ABS_IDX_Y},
	{0xEF,	O_sbc,	"sbc",	ADDR_ABS_LONG},
	{0xFF,	O_sbc,	"sbc",	ADDR_ABS_LONG_IDX_X},
	{0xE5,	O_sbc,	"sbc",	ADDR_DIR},
	{0xE3,	O_sbc,	"sbc",	ADDR_STACK_REL},
	{0xF5,	O_sbc,	"sbc",	ADDR_DIR_IDX_X},
	{0x38,	O_sec,	"sec",	ADDR_IMPLIED},
	{0xF8,	O_sed,	"sed",	ADDR_IMPLIED},
	{0x78,	O_sei,	"sei",	ADDR_IMPLIED},
	{0xE2,	O_sep,	"sep",	ADDR_IMMCOP},
	{0x92,	O_sta,	"sta",	ADDR_DIR_IND},
	{0x91,	O_sta,	"sta",	ADDR_DIR_IND_IDX_Y},
	{0x93,	O_sta,	"sta",	ADDR_STACK_REL_INDX_IDX},
	{0x81,	O_sta,	"sta",	ADDR_DIR_IDX_IND_X},
	{0x87,	O_sta,	"sta",	ADDR_DIR_IND_LONG},
	{0x97,	O_sta,	"sta",	ADDR_DIR_IND_IDX_Y_LONG},
	{0x8D,	O_sta,	"sta",	ADDR_ABS},
	{0x9D,	O_sta,	"sta",	ADDR_ABS_IDX_X},
	{0x99,	O_sta,	"sta",	ADDR_ABS_IDX_Y},
	{0x8F,	O_sta,	"sta",	ADDR_ABS_LONG},
	{0x9F,	O_sta,	"sta",	ADDR_ABS_LONG_IDX_X},
	{0x85,	O_sta,	"sta",	ADDR_DIR},
	{0x83,	O_sta,	"sta",	ADDR_STACK_REL},
	{0x95,	O_sta,	"sta",	ADDR_DIR_IDX_X},
	{0xDB,	O_stp,	"stp",	ADDR_IMPLIED},
	{0x8E,	O_stx,	"stx",	ADDR_ABS},
	{0x86,	O_stx,	"stx",	ADDR_DIR},
	{0x96,	O_stx,	"stx",	ADDR_DIR_IDX_X},
	{0x8C,	O_sty,	"sty",	ADDR_ABS},
	{0x84,	O_sty,	"sty",	ADDR_DIR},
	{0x94,	O_sty,	"sty",	ADDR_DIR_IDX_X},
	{0x9C,	O_stz,	"stz",	ADDR_ABS},
	{0x9E,	O_stz,	"stz",	ADDR_ABS_IDX_X},
	{0x64,	O_stz,	"stz",	ADDR_DIR},
	{0x74,	O_stz,	"stz",	ADDR_DIR_IDX_X},
	{0xAA,	O_tax,	"tax",	ADDR_IMPLIED},
	{0xA8,	O_tay,	"tay",	ADDR_IMPLIED},
	{0x5B,	O_tcd,	"tcd",	ADDR_IMPLIED},
	{0x1B,	O_tcs,	"tcs",	ADDR_IMPLIED},
	{0x7B,	O_tdc,	"tdc",	ADDR_IMPLIED},
	{0x1C,	O_trb,	"trb",	ADDR_ABS},
	{0x14,	O_trb,	"trb",	ADDR_DIR},
	{0x0C,	O_tsb,	"tsb",	ADDR_ABS},
	{0x04,	O_tsb,	"tsb",	ADDR_DIR},
	{0x3B,	O_tsc,	"tsc",	ADDR_IMPLIED},
	{0xBA,	O_tsx,	"tsx",	ADDR_IMPLIED},
	{0x8A,	O_txa,	"txa",	ADDR_IMPLIED},
	{0x9A,	O_txs,	"txs",	ADDR_IMPLIED},
	{0x9B,	O_txy,	"txy",	ADDR_IMPLIED},
	{0x98,	O_tya,	"tya",	ADDR_IMPLIED},
	{0xBB,	O_tyx,	"tyx",	ADDR_IMPLIED},
	{0xCB,	O_wai,	"wai",	ADDR_IMPLIED},
	{0x42,	O_wdm,	"wdm",	ADDR_IMPLIED},
	{0xEB,	O_xba,	"xba",	ADDR_IMPLIED},
	{0xFB,	O_xce,	"xce",	ADDR_IMPLIED},
	{ 0 }
};
#endif
#define DISASM()\
  case ADDR_IMMTOA:\
	args[0] = M==0 ? asR_W65_ABS16 : asR_W65_ABS8;\
	print_operand (0, "	#$0", args);\
	size += M==0 ? 2:1;\
	break;\
  case ADDR_IMMCOP:\
	args[0] = asR_W65_ABS8;\
	print_operand (0, "	#$0", args);\
	size += 1;\
	break;\
  case ADDR_IMMTOI:\
	args[0] = X==0 ? asR_W65_ABS16 : asR_W65_ABS8;\
	print_operand (0, "	#$0", args);\
	size += X==0 ? 2:1;\
	break;\
  case ADDR_ACC:\
	print_operand (0, "	a", 0);\
	size += 0;\
	break;\
  case ADDR_PC_REL:\
	args[0] = asR_W65_PCR8;\
	print_operand (0, "	$0", args);\
	size += 1;\
	break;\
  case ADDR_PC_REL_LONG:\
	args[0] = asR_W65_PCR16;\
	print_operand (0, "	$0", args);\
	size += 2;\
	break;\
  case ADDR_IMPLIED:\
		size += 0;\
	break;\
  case ADDR_STACK:\
		size += 0;\
	break;\
  case ADDR_DIR:\
	args[0] = asR_W65_ABS8;\
	print_operand (1, "	<$0", args);\
	size += 1;\
	break;\
  case ADDR_DIR_IDX_X:\
	args[0] = asR_W65_ABS8;\
	print_operand (1, "	<$0,x", args);\
	size += 1;\
	break;\
  case ADDR_DIR_IDX_Y:\
	args[0] = asR_W65_ABS8;\
	print_operand (1, "	<$0,y", args);\
	size += 1;\
	break;\
  case ADDR_DIR_IND:\
	args[0] = asR_W65_ABS8;\
	print_operand (1, "	(<$0)", args);\
	size += 1;\
	break;\
  case ADDR_DIR_IDX_IND_X:\
	args[0] = asR_W65_ABS8;\
	print_operand (1, "	(<$0,x)", args);\
	size += 1;\
	break;\
  case ADDR_DIR_IND_IDX_Y:\
	args[0] = asR_W65_ABS8;\
	print_operand (1, "	(<$0),y", args);\
	size += 1;\
	break;\
  case ADDR_DIR_IND_LONG:\
	args[0] = asR_W65_ABS8;\
	print_operand (1, "	[$0]", args);\
	size += 1;\
	break;\
  case ADDR_DIR_IND_IDX_Y_LONG:\
	args[0] = asR_W65_ABS8;\
	print_operand (1, "	[$0],y", args);\
	size += 1;\
	break;\
  case ADDR_ABS:\
	args[0] = asR_W65_ABS16;\
	print_operand (1, "	!$0", args);\
	size += 2;\
	break;\
  case ADDR_ABS_IDX_X:\
	args[0] = asR_W65_ABS16;\
	print_operand (1, "	!$0,x", args);\
	size += 2;\
	break;\
  case ADDR_ABS_IDX_Y:\
	args[0] = asR_W65_ABS16;\
	print_operand (1, "	!$0,y", args);\
	size += 2;\
	break;\
  case ADDR_ABS_LONG:\
	args[0] = asR_W65_ABS24;\
	print_operand (1, "	>$0", args);\
	size += 3;\
	break;\
  case ADDR_ABS_IND_LONG:\
	args[0] = asR_W65_ABS16;\
	print_operand (1, "	[>$0]", args);\
	size += 2;\
	break;\
  case ADDR_ABS_LONG_IDX_X:\
	args[0] = asR_W65_ABS24;\
	print_operand (1, "	>$0,x", args);\
	size += 3;\
	break;\
  case ADDR_STACK_REL:\
	args[0] = asR_W65_ABS8;\
	print_operand (0, "	$0,s", args);\
	size += 1;\
	break;\
  case ADDR_STACK_REL_INDX_IDX:\
	args[0] = asR_W65_ABS8;\
	print_operand (0, "	($0,s),y", args);\
	size += 1;\
	break;\
  case ADDR_ABS_IND:\
	args[0] = asR_W65_ABS16;\
	print_operand (1, "	($0)", args);\
	size += 2;\
	break;\
  case ADDR_ABS_IND_IDX:\
	args[0] = asR_W65_ABS16;\
	print_operand (1, "	($0,x)", args);\
	size += 2;\
	break;\
  case ADDR_BLOCK_MOVE:\
	args[0] = (asR_W65_ABS16 >>8) &0xff;\
	args[1] = ( asR_W65_ABS16 & 0xff);\
	print_operand (0,"	$0,$1",args);\
	size += 2;\
	break;\

#define GETINFO(size,type,pcrel)\
	case ADDR_IMMTOA: size = M==0 ? 2:1;type=M==0 ? R_W65_ABS16 : R_W65_ABS8;pcrel=0;break;\
	case ADDR_IMMCOP: size = 1;type=R_W65_ABS8;pcrel=0;break;\
	case ADDR_IMMTOI: size = X==0 ? 2:1;type=X==0 ? R_W65_ABS16 : R_W65_ABS8;pcrel=0;break;\
	case ADDR_ACC: size = 0;type=-1;pcrel=0;break;\
	case ADDR_PC_REL: size = 1;type=R_W65_PCR8;pcrel=0;break;\
	case ADDR_PC_REL_LONG: size = 2;type=R_W65_PCR16;pcrel=0;break;\
	case ADDR_IMPLIED: size = 0;type=-1;pcrel=0;break;\
	case ADDR_STACK: size = 0;type=-1;pcrel=0;break;\
	case ADDR_DIR: size = 1;type=R_W65_ABS8;pcrel=0;break;\
	case ADDR_DIR_IDX_X: size = 1;type=R_W65_ABS8;pcrel=0;break;\
	case ADDR_DIR_IDX_Y: size = 1;type=R_W65_ABS8;pcrel=0;break;\
	case ADDR_DIR_IND: size = 1;type=R_W65_ABS8;pcrel=0;break;\
	case ADDR_DIR_IDX_IND_X: size = 1;type=R_W65_ABS8;pcrel=0;break;\
	case ADDR_DIR_IND_IDX_Y: size = 1;type=R_W65_ABS8;pcrel=0;break;\
	case ADDR_DIR_IND_LONG: size = 1;type=R_W65_ABS8;pcrel=0;break;\
	case ADDR_DIR_IND_IDX_Y_LONG: size = 1;type=R_W65_ABS8;pcrel=0;break;\
	case ADDR_ABS: size = 2;type=R_W65_ABS16;pcrel=0;break;\
	case ADDR_ABS_IDX_X: size = 2;type=R_W65_ABS16;pcrel=0;break;\
	case ADDR_ABS_IDX_Y: size = 2;type=R_W65_ABS16;pcrel=0;break;\
	case ADDR_ABS_LONG: size = 3;type=R_W65_ABS24;pcrel=0;break;\
	case ADDR_ABS_IND_LONG: size = 2;type=R_W65_ABS16;pcrel=0;break;\
	case ADDR_ABS_LONG_IDX_X: size = 3;type=R_W65_ABS24;pcrel=0;break;\
	case ADDR_STACK_REL: size = 1;type=R_W65_ABS8;pcrel=0;break;\
	case ADDR_STACK_REL_INDX_IDX: size = 1;type=R_W65_ABS8;pcrel=0;break;\
	case ADDR_ABS_IND: size = 2;type=R_W65_ABS16;pcrel=0;break;\
	case ADDR_ABS_IND_IDX: size = 2;type=R_W65_ABS16;pcrel=0;break;\
	case ADDR_BLOCK_MOVE: size = 2;type=-1;pcrel=0;break;\

