; check:	 not case sensitive for special operand modifier
; check:	 shigh, high, low
	.text
relocs:
	seth	r0, #shigh(0x87654321)
	add3	r0, r0, #low(0x87654321)
	seth	r0, #SHIGH(0x87654321)
	add3	r0, r0, #LOW(0x87654321)
	seth	r0, #shigh(0x1234ffff)
	add3	r0, r0, #low(0x1234ffff)
	seth	r0, #SHIGH(0x1234ffff)
	add3	r0, r0, #LOW(0x1234ffff)

	seth	r0, #high(0x87654321)
	or3	r0, r0, #low(0x87654321)
	seth	r0, #HIGH(0x87654321)
	or3	r0, r0, #LOW(0x87654321)
	seth	r0, #high(0x1234ffff)
	or3	r0, r0, #low(0x1234ffff)
	seth	r0, #HIGH(0x1234ffff)
	or3	r0, r0, #LOW(0x1234ffff)

	seth	r0, #shigh(0x87654320)
	ld	r0, @(#low(0x87654320),r0)
	seth	r0, #shigh(0x87654320)
	ldh	r0, @(#low(0x87654320),r0)
	seth	r0, #shigh(0x87654320)
	lduh	r0, @(#low(0x87654320),r0)
	seth	r0, #shigh(0x87654320)
	ldb	r0, @(#low(0x87654320),r0)
	seth	r0, #shigh(0x87654320)
	ldub	r0, @(#low(0x87654320),r0)

	seth	r0, #shigh(0x1234fff0)
	ld	r0, @(#low(0x1234fff0),r0)
	seth	r0, #shigh(0x1234fff0)
	ldh	r0, @(#low(0x1234fff0),r0)
	seth	r0, #shigh(0x1234fff0)
	lduh	r0, @(#low(0x1234fff0),r0)
	seth	r0, #shigh(0x1234fff0)
	ldb	r0, @(#low(0x1234fff0),r0)
	seth	r0, #shigh(0x1234fff0)
	ldub	r0, @(#low(0x1234fff0),r0)

	seth	r0, #SHIGH(0x87654320)
	ld	r0, @(#LOW(0x87654320),r0)
	seth	r0, #SHIGH(0x87654320)
	ldh	r0, @(#LOW(0x87654320),r0)
	seth	r0, #SHIGH(0x87654320)
	lduh	r0, @(#LOW(0x87654320),r0)
	seth	r0, #SHIGH(0x87654320)
	ldb	r0, @(#LOW(0x87654320),r0)
	seth	r0, #SHIGH(0x87654320)
	ldub	r0, @(#LOW(0x87654320),r0)
	seth	r0, #SHIGH(0x1234fff0)
	ld	r0, @(#LOW(0x1234fff0),r0)

	seth	r0, #shigh(0x87654320)
	st	r0, @(#low(0x87654320),r0)
	seth	r0, #shigh(0x87654320)
	sth	r0, @(#low(0x87654320),r0)
	seth	r0, #shigh(0x87654320)
	stb	r0, @(#low(0x87654320),r0)

	seth	r0, #shigh(0x1234fff0)
	st	r0, @(#low(0x1234fff0),r0)
	seth	r0, #shigh(0x1234fff0)
	sth	r0, @(#low(0x1234fff0),r0)
	seth	r0, #shigh(0x1234fff0)
	stb	r0, @(#low(0x1234fff0),r0)

	seth	r0, #SHIGH(0x87654320)
	st	r0, @(#LOW(0x87654320),r0)
	seth	r0, #SHIGH(0x87654320)
	sth	r0, @(#LOW(0x87654320),r0)
	seth	r0, #SHIGH(0x87654320)
	stb	r0, @(#LOW(0x87654320),r0)
	seth	r0, #SHIGH(0x1234fff0)
	st	r0, @(#LOW(0x1234fff0),r0)

