#
# Somewhat complete instruction set and operand type check.  No
# relocations or deferred register definitions here.
#
#
#
Main TETRA 3
 TRAP 3,4,5
 FCMP $12,$23,$241
 FLOT $112,ROUND_OFF,$41
 FLOT $112,ROUND_NEAR,141
 FLOT $191,$242
 FLOT $195,42
 FUN  $122,$203,$4
 FEQL $102,$30,$40
 FLOTU $102,$14
 FLOTU $132,ROUND_UP,$14
 FLOTU $102,ROUND_DOWN,$104
 FLOTU $172,ROUND_NEAR,$140
 FLOTU $1,ROUND_OFF,$134
 FADD $112,$223,$41
 FIX $112,ROUND_OFF,$41
 FIX $11,$141
 SFLOT $112,ROUND_OFF,$41
 SFLOT $112,ROUND_NEAR,141
 FSUB $112,$223,$41
 FIXU $102,$14
 FIXU $132,ROUND_UP,$14
 SFLOTU $11,$141
 SFLOTU $112,141
 SFLOTU $112,ROUND_NEAR,141
 SFLOTU $112,ROUND_OFF,$41
 FMUL $102,$30,$40
 FCMPE $12,$223,$1
 MUL  $122,$203,44
 MUL $102,$30,$40
 FEQLE $12,$223,$1
 FUNE $12,$223,$11
 MULU  $122,$213,44
 MULU $132,$30,$40
 FDIV $12,$223,$11
 FSQRT $132,ROUND_UP,$14
 FSQRT $11,$141
 DIV  $122,$213,44
 DIV $132,$30,$40
 FREM $12,$223,$11
 FINT $132,ROUND_UP,$14
 FINT $11,$141
 DIVU $12,$223,$1
 DIVU  $122,$203,255
 ADD $12,$223,$1
 ADD $122,$203,255
 2ADDU $12,$223,$11
 2ADDU $122,$203,0
 ADDU $122,$203,255
 ADDU $12,$223,$11
 LDA $122,$203,255
 LDA $12,$223,$11
 4ADDU $122,$203,205
 4ADDU $12,$223,$111
 SUB $12,$223,$11
 SUB $122,$203,205
 8ADDU $12,$223,$11
 8ADDU $122,$203,205
 SUBU $2,$223,$11
 SUBU $12,$20,205
 16ADDU $2,$223,$11
 16ADDU $12,$20,205
 CMP $2,$223,$11
 CMP $12,$20,205
 SL $2,$223,$11
 SL $12,$20,205
 CMPU $2,$223,$11
 CMPU $12,$20,205
 SLU $2,$223,$11
 SLU $12,$20,205
 NEG $2,23,$11
 NEG $12,0,205
 NEG $192,10,205
 SR $12,$20,205
 SR $2,$223,$11
 NEGU $2,23,$11
 NEGU $12,0,205
 SRU $12,$20,205
 SRU $2,$223,$11
1H BN $2,2F
2H BN $2,1B
1H BNN $2,2B
2H BNN $2,1B
1H BZ $255,2F
2H BZ $255,1B
1H BNZ $255,2F
2H BNZ $255,1B
1H BP $25,2F
2H BP $25,1B
1H BNP $25,2F
2H BNP $25,1B
1H BOD $25,2F
2H BOD $25,1B
1H BEV $25,2F
2H BEV $25,1B
1H PBN $2,2F
2H PBN $2,1B
1H PBNN $2,2F
2H PBNN $2,1B
1H PBZ $12,2F
2H PBZ $22,1B
1H PBNZ $32,2F
2H PBNZ $52,1B
1H PBOD $25,2F
2H PBOD $25,1B
1H PBEV $25,2F
2H PBEV $25,1B
 CSN $2,$223,$11
 CSN $12,$20,205
 CSNN $2,$223,$11
 CSNN $12,$20,205
 CSZ $2,$203,$11
 CSZ $12,$200,205
 CSNZ $2,$203,$11
 CSNZ $12,$200,205
 CSP $2,$203,$11
 CSP $12,$200,205
 CSNP $2,$203,$11
 CSNP $12,$200,205
 CSOD $2,$203,$11
 CSOD $12,$200,205
 CSEV $2,$203,$11
 CSEV $12,$200,205
 ZSN $2,$223,$11
 ZSN $12,$20,205
 ZSNN $2,$223,$11
 ZSNN $12,$20,205
 ZSZ $2,$203,$11
 ZSZ $12,$200,205
 ZSNZ $2,$203,$11
 ZSNZ $12,$200,205
 ZSP $2,$203,$11
 ZSP $12,$200,205
 ZSNP $2,$203,$11
 ZSNP $12,$200,205
 ZSOD $2,$203,$11
 ZSOD $12,$200,205
 ZSEV $2,$203,$11
 ZSEV $12,$200,205
 LDB $2,$0,$11
 LDB $12,$20,205
 LDT $2,$0,$11
 LDT $12,$20,205
 LDBU $2,$0,$11
 LDBU $12,$20,205
 LDTU $2,$0,$11
 LDTU $12,$20,205
 LDW $2,$0,$11
 LDW $12,$20,205
 LDO $2,$0,$11
 LDO $12,$20,205
 LDWU $2,$0,$11
 LDWU $12,$20,205
 LDOU $2,$0,$11
 LDOU $12,$20,205
 LDVTS $2,$0,$11
 LDVTS $12,$20,205
 LDHT $2,$0,$11
 LDHT $12,$20,205
 PRELD 112,$20,205
 PRELD 112,$20,$225
 CSWAP $2,$0,$11
 CSWAP $12,$20,205
 PREGO 112,$20,205
 PREGO 112,$20,$225
 LDUNC $2,$0,$11
 LDUNC $12,$20,205
 GO $2,$0,$11
 GO $12,$20,205
 STB $2,$10,$151
 STB $12,$20,205
 STT $32,$10,$151
 STT $12,$20,205
 STBU $2,$10,$151
 STBU $12,$20,205
 STTU $32,$10,$151
 STTU $12,$20,205
 STW $2,$10,$151
 STW $12,$220,205
 STO $32,$170,$151
 STO $182,$20,245
 STWU $2,$10,$151
 STWU $12,$220,205
 STOU $32,$170,$151
 STOU $182,$20,245
 STSF $32,$170,$151
 STSF $182,$20,245
 SYNCD 112,$20,205
 SYNCD 112,$20,$225
 STHT $32,$170,$151
 STHT $182,$20,245
 PREST 112,$20,205
 PREST 112,$20,$225
 STCO 32,$170,$151
 STCO 182,$20,245
 SYNCID 112,$20,205
 SYNCID 0,$20,$225
 STUNC $32,$170,$151
 STUNC $182,$20,245
 PUSHGO $32,$170,$151
 PUSHGO $182,$20,245
 SET $142,$200
 OR $32,$170,$151
 OR $182,$20,245
 AND $32,$170,$151
 AND $182,$20,245
 ORN $32,$170,$151
 ORN $182,$20,245
 ANDN $32,$170,$151
 ANDN $182,$20,245
 NOR $32,$170,$151
 NOR $182,$20,245
 NAND $32,$170,$151
 NAND $182,$20,245
 XOR $32,$170,$151
 XOR $182,$20,245
 NXOR $32,$170,$151
 NXOR $182,$20,245
 BDIF $32,$170,$151
 BDIF $182,$20,245
 MUX $32,$170,$151
 MUX $182,$20,245
 WDIF $32,$170,$151
 WDIF $182,$20,245
 SADD $32,$170,$151
 SADD $182,$0,245
 TDIF $32,$170,$151
 TDIF $182,$20,245
 MOR $32,$170,$151
 MOR $182,$20,245
 ODIF $32,$170,$151
 ODIF $182,$20,245
 MXOR $32,$17,$151
 MXOR $82,$180,24
 SETH $4,65535
 SETH $94,0
 SETH $4,255
 SETH $94,1234
 SETMH $94,1234
 ORH $94,1234
 ORMH $94,1234
 SETML $94,1234
 SETL $94,1234
 ORML $94,1234
 ORL $94,1234
 INCH $94,1234
 INCMH $94,1234
 ANDNH $94,1234
 ANDNMH $94,1234
 INCML $94,1234
 INCL $94,1234
 ANDNML $94,1234
0H ANDNL $94,1234
 JMP 0B
 JMP 0F
0H POP 42,65534
 RESUME 255
 RESUME 0
 RESUME 1
1H PUSHJ $25,2F
2H PUSHJ $25,1B
 SAVE $4,0
 UNSAVE 0,$234
1H GETA $25,2F
2H GETA $25,1B
 SYNC 8000001
 SWYM 1,2,3
 SWYM 0,0,0
 PUT rJ,34
 PUT rJ,$134
 GET $234,rJ
 TRIP 0,0,0
 TRIP 5,6,7
