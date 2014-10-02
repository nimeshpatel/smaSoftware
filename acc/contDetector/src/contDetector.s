	.file	"contDetector.c"
.toc
.csect .text[PR]
 # GNU C version 2.9-gnupro-98r2 (ppc-xcoff-lynxos) compiled by GNU C version 2.9-gnupro-98r1.
 # options passed:  -ansi -O -Wall -Wcast-align -ansi -finline-functions
 # options enabled:  -fdefer-pop -fomit-frame-pointer -fthread-jumps
 # -fpeephole -ffunction-cse -finline-functions -finline
 # -fkeep-static-consts -fpcc-struct-return -fsched-interblock -fsched-spec
 # -fsjlj-exceptions -finhibit-size-directive -fverbose-asm
 # -flive-range-gdb -fargument-alias -mpowerpc -mnew-mnemonics

gcc2_compiled.:
__gnu_compiled_c:
.csect _contDetector.rw_c[RO]
	.balign 4
LC..0:
	.byte "contDetectorOpen: opened"
	.byte 13, 10, 0
	.balign 4
LC..2:
	.byte "contDetectorClosed"
	.byte 13, 10, 0
	.balign 4
LC..5:
	.byte "contDetector: tswait() reports no timeouts available"
	.byte 13, 10, 0
	.balign 4
LC..7:
	.byte "contDetector: tswait() released on signal"
	.byte 13, 10, 0
	.balign 4
LC..9:
	.byte "contDetector: tswait() timed out before semaphore was posted"
	.byte 13, 10, 0
.toc
LC..4:
	.tc slevel[TC],slevel[RW]
LC..6:
	.tc LC..5[TC],LC..5
LC..8:
	.tc LC..7[TC],LC..7
LC..10:
	.tc LC..9[TC],LC..9
.csect .text[PR]
	.balign 4
	.globl contDetectorRead
	.globl .contDetectorRead
.csect contDetectorRead[DS]
contDetectorRead:
	.long .contDetectorRead, TOC[tc0], 0
.csect .text[PR]
.contDetectorRead:
	mflr 0
	stw 27,-20(1)
	stw 28,-16(1)
	stw 29,-12(1)
	stw 30,-8(1)
	stw 31,-4(1)
	stw 0,8(1)
	stwu 1,-80(1)
	mr 31,3
	mr 27,5
	lwz 30,24(31)
	cmpwi 0,30,1
	bc 4,2,L..9
	lwz 3,0(31)
	bl .iSetCntl
	cror 31,31,31
	lwz 9,0(31)
	lha 0,20(31)
	sth 0,32(9)
	lwz 9,0(31)
	lhz 0,22(31)
	sth 0,34(9)
	lwz 29,LC..4(2)
	bl .disable_intr
	cror 31,31,31
	li 0,3
	stw 0,0(29)
	lwz 9,0(31)
	li 0,42
	sth 0,36(9)
	stw 30,28(31)
	addi 3,31,8
	li 4,1
	bl .swait
	cror 31,31,31
	mr 29,3
	li 0,0
	stw 0,28(31)
	bl .enable_intr
	cror 31,31,31
	b L..10
L..9:
	lwz 0,24(31)
	cmpwi 0,0,2
	bc 4,2,L..11
	lwz 9,0(31)
	li 0,42
	sth 0,36(9)
	li 29,0
	lwz 9,0(31)
	lhz 0,22(9)
	cmpwi 0,0,0
	bc 4,2,L..13
	li 28,2
	li 30,0
L..15:
	stw 28,28(31)
	addi 3,31,8
	li 4,1
	li 5,1
	bl .tswait
	cror 31,31,31
	stw 30,28(31)
	addi 29,29,1
	cmpwi 0,29,199
	bc 12,1,L..13
	lwz 9,0(31)
	lhz 0,22(9)
	cmpwi 0,0,0
	bc 12,2,L..15
L..13:
	li 0,3
	stw 0,28(31)
	addi 3,31,8
	li 4,1
	li 5,200
	bl .tswait
	cror 31,31,31
	mr 29,3
	li 0,0
	stw 0,28(31)
	b L..10
L..11:
	li 3,0
	b L..28
L..10:
	cmpwi 0,29,2
	bc 4,2,L..19
	lwz 0,12(31)
	cmpwi 0,0,0
	bc 12,2,L..20
	lwz 3,LC..6(2)
	bl .cprintf
	cror 31,31,31
L..20:
	li 3,11
	b L..29
L..19:
	cmpwi 0,29,-1
	bc 4,2,L..22
	lwz 0,12(31)
	cmpwi 0,0,0
	bc 12,2,L..23
	lwz 3,LC..8(2)
	bl .cprintf
	cror 31,31,31
L..23:
	li 3,4
	b L..29
L..22:
	cmpwi 0,29,1
	bc 12,2,L..26
	lwz 0,32(31)
	cmpwi 0,0,0
	bc 12,2,L..21
L..26:
	lwz 0,12(31)
	cmpwi 0,0,0
	bc 12,2,L..27
	lwz 3,LC..10(2)
	bl .cprintf
	cror 31,31,31
L..27:
	li 0,0
	stw 0,32(31)
	li 3,60
L..29:
	bl .pseterr
	cror 31,31,31
	li 3,-1
	b L..28
L..21:
	lwz 9,0(31)
	lhz 0,12(9)
	slwi 0,0,16
	lhz 9,14(9)
	or 0,0,9
	stw 0,0(27)
	lwz 9,0(31)
	lhz 0,16(9)
	slwi 0,0,16
	lhz 9,18(9)
	or 0,0,9
	stw 0,4(27)
	lwz 9,0(31)
	lhz 0,20(9)
	slwi 0,0,16
	lhz 9,22(9)
	or 0,0,9
	stw 0,8(27)
	li 3,12
L..28:
	la 1,80(1)
	lwz 0,8(1)
	mtlr 0
	lwz 27,-20(1)
	lwz 28,-16(1)
	lwz 29,-12(1)
	lwz 30,-8(1)
	lwz 31,-4(1)
	blr
.toc
LC..11:
	.tc slevel[TC],slevel[RW]
LC..12:
	.tc ast2[TC],ast2[RW]
LC..13:
	.tc L..47[TC],L..47
.csect .text[PR]
	.balign 4
	.globl contDetectorioctl
	.globl .contDetectorioctl
.csect contDetectorioctl[DS]
contDetectorioctl:
	.long .contDetectorioctl, TOC[tc0], 0
.csect .text[PR]
.contDetectorioctl:
	mflr 0
	stw 28,-16(1)
	stw 29,-12(1)
	stw 30,-8(1)
	stw 31,-4(1)
	stw 0,8(1)
	stwu 1,-72(1)
	mr 31,3
	mr 29,6
	addi 0,5,-256
	cmplwi 0,0,5
	bc 12,1,L..46
	lwz 9,LC..13(2)
	slwi 0,0,2
	lwzx 0,9,0
	add 0,0,9
	mtctr 0
	bctr
	.balign 4
	.balign 4
L..47:
	.long L..33-L..47
	.long L..34-L..47
	.long L..32-L..47
	.long L..36-L..47
	.long L..40-L..47
	.long L..41-L..47
L..33:
	li 0,0
	stw 0,12(31)
	b L..32
L..34:
	li 0,1
	stw 0,12(31)
	b L..32
L..36:
	lwz 9,0(31)
	li 0,1
	sth 0,38(9)
	lwz 9,0(29)
	stw 9,24(31)
	lwz 0,4(29)
	stw 0,20(31)
	li 0,0
	stw 0,8(31)
	cmpwi 0,9,1
	bc 4,2,L..37
	lwz 3,0(31)
	bl .iSetCntl
	cror 31,31,31
	b L..32
L..37:
	lwz 0,24(31)
	cmpwi 0,0,2
	bc 4,2,L..32
	lwz 3,0(31)
	bl .cSetCntl
	cror 31,31,31
	b L..32
L..40:
	lwz 0,28(31)
	stw 0,0(29)
	addi 3,31,8
	bl .scount
	cror 31,31,31
	stw 3,4(29)
	b L..32
L..41:
	addi 30,31,8
	mr 3,30
	bl .scount
	cror 31,31,31
	cmpwi 0,3,0
	bc 4,0,L..42
	lwz 29,LC..11(2)
	lwz 28,0(29)
	bl .disable_intr
	cror 31,31,31
	li 0,3
	stw 0,0(29)
	li 0,1
	stw 0,32(31)
	mr 3,30
	bl .sreset
	cror 31,31,31
	stw 28,0(29)
	cmpwi 0,28,2
	bc 12,1,L..43
	bl .enable_intr
	cror 31,31,31
L..43:
	lwz 9,LC..11(2)
	lwz 0,0(9)
	cmpwi 0,0,1
	bc 12,1,L..32
	lwz 9,LC..12(2)
	lwz 0,0(9)
	cmpwi 0,0,0
	bc 12,2,L..32
	bl ._mail
	cror 31,31,31
	b L..32
L..42:
	li 3,3
	b L..49
L..46:
	li 3,22
L..49:
	bl .pseterr
	cror 31,31,31
	li 3,-1
	b L..48
L..32:
	li 3,0
L..48:
	la 1,72(1)
	lwz 0,8(1)
	mtlr 0
	lwz 28,-16(1)
	lwz 29,-12(1)
	lwz 30,-8(1)
	lwz 31,-4(1)
	blr
.csect _contDetector.rw_c[RW]
	.balign 4
idTest.14:
	.byte 73
	.byte 80
	.byte 65
	.byte 67
	.byte 163
	.byte 22
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 12
	.byte 30
.csect _contDetector.rw_c[RO]
	.balign 4
LC..14:
	.byte "Entering install of contDetector device"
	.byte 13, 10, 0
	.balign 4
LC..18:
	.byte "contDetector driver installed"
	.byte 13, 10, 0
.toc
LC..15:
	.tc LC..14[TC],LC..14
LC..16:
	.tc idTest.14[TC],idTest.14
LC..17:
	.tc intHndlr[TC],intHndlr[DS]
LC..19:
	.tc LC..18[TC],LC..18
.csect .text[PR]
	.balign 4
	.globl contDetectorInstall
	.globl .contDetectorInstall
.csect contDetectorInstall[DS]
contDetectorInstall:
	.long .contDetectorInstall, TOC[tc0], 0
.csect .text[PR]
.contDetectorInstall:
	mflr 0
	stw 30,-8(1)
	stw 31,-4(1)
	stw 0,8(1)
	stwu 1,-64(1)
	mr 30,3
	lwz 3,LC..15(2)
	bl .cprintf
	cror 31,31,31
	li 11,0
	lwz 9,0(30)
	addi 10,9,128
	lwz 8,LC..16(2)
L..54:
	add 0,11,11
	lhzx 9,10,0
	rlwinm 9,9,0,0xff
	lbzx 0,8,11
	cmpw 0,9,0
	bc 4,2,L..60
	addi 11,11,1
	cmplwi 0,11,11
	bc 4,1,L..54
	li 3,36
	bl .sysbrk
	cror 31,31,31
	mr. 31,3
	bc 4,2,L..57
	li 3,12
	b L..61
L..60:
	li 3,6
L..61:
	bl .pseterr
	cror 31,31,31
	li 3,-1
	b L..59
L..57:
	lwz 11,0(30)
	stw 11,0(31)
	li 9,0
	stw 9,12(31)
	stw 9,16(31)
	stw 9,24(31)
	li 0,1
	sth 0,38(11)
	stw 9,8(31)
	stw 9,32(31)
	lwz 0,4(30)
	stw 0,4(31)
	lwz 9,0(31)
	lhz 0,6(30)
	sth 0,44(9)
	lwz 3,4(31)
	lwz 4,LC..17(2)
	mr 5,31
	bl .iointset
	cror 31,31,31
	lwz 0,12(31)
	cmpwi 0,0,0
	bc 12,2,L..58
	lwz 3,LC..19(2)
	bl .cprintf
	cror 31,31,31
L..58:
	mr 3,31
L..59:
	la 1,64(1)
	lwz 0,8(1)
	mtlr 0
	lwz 30,-8(1)
	lwz 31,-4(1)
	blr
.csect _contDetector.rw_c[RO]
	.balign 4
LC..20:
	.byte "contDetectorUninstall: freeing memory for statics struct"
	.byte 13, 10, 0
	.globl entry_points
.csect .data[RW]
	.balign 4
entry_points:
	.long contDetectorOpen
	.long contDetectorClose
	.long contDetectorRead
	.long contDetectorInvalid
	.long contDetectorInvalid
	.long contDetectorioctl
	.long contDetectorInstall
	.long contDetectorUninstall
	.long 0
.csect .text[PR]
	.balign 4
	.globl cSetCntl
	.globl .cSetCntl
.csect cSetCntl[DS]
cSetCntl:
	.long .cSetCntl, TOC[tc0], 0
.csect .text[PR]
.cSetCntl:
	li 0,389
	sth 0,10(3)
	li 0,5909
	sth 0,2(3)
	li 0,1813
	sth 0,6(3)
	li 9,0
	addi 3,3,24
	li 11,0
L..74:
	add 0,9,9
	sthx 11,3,0
	addi 9,9,1
	cmpwi 0,9,5
	bc 4,1,L..74
	blr
	.balign 4
	.globl iSetCntl
	.globl .iSetCntl
.csect iSetCntl[DS]
iSetCntl:
	.long .iSetCntl, TOC[tc0], 0
.csect .text[PR]
.iSetCntl:
	li 0,395
	sth 0,10(3)
	li 0,1813
	sth 0,2(3)
	sth 0,6(3)
	li 9,0
	addi 3,3,24
	li 11,0
L..68:
	add 0,9,9
	sthx 11,3,0
	addi 9,9,1
	cmpwi 0,9,3
	bc 4,1,L..68
	blr
	.balign 4
	.globl intHndlr
	.globl .intHndlr
.csect intHndlr[DS]
intHndlr:
	.long .intHndlr, TOC[tc0], 0
.csect .text[PR]
.intHndlr:
	mflr 0
	stw 0,8(1)
	stwu 1,-56(1)
	lwz 9,0(3)
	li 0,0
	sth 0,42(9)
	lwz 9,28(3)
	addi 9,9,8
	stw 9,28(3)
	li 0,0
	stw 0,32(3)
	addi 3,3,8
	bl .ssignal
	cror 31,31,31
	la 1,56(1)
	lwz 0,8(1)
	mtlr 0
	blr
.toc
LC..22:
	.tc LC..0[TC],LC..0
.csect .text[PR]
	.balign 4
	.globl contDetectorOpen
	.globl .contDetectorOpen
.csect contDetectorOpen[DS]
contDetectorOpen:
	.long .contDetectorOpen, TOC[tc0], 0
.csect .text[PR]
.contDetectorOpen:
	mflr 0
	stw 0,8(1)
	stwu 1,-56(1)
	lwz 0,4(5)
	cmpwi 0,0,1
	bc 12,2,L..3
	li 3,22
	b L..78
L..3:
	lwz 0,16(3)
	cmpwi 0,0,0
	bc 12,2,L..4
	li 3,16
L..78:
	bl .pseterr
	cror 31,31,31
	li 3,-1
	b L..77
L..4:
	li 0,1
	stw 0,16(3)
	lwz 0,12(3)
	cmpwi 0,0,0
	bc 12,2,L..5
	lwz 3,LC..22(2)
	bl .cprintf
	cror 31,31,31
L..5:
	li 3,0
L..77:
	la 1,56(1)
	lwz 0,8(1)
	mtlr 0
	blr
.toc
LC..23:
	.tc LC..2[TC],LC..2
.csect .text[PR]
	.balign 4
	.globl contDetectorClose
	.globl .contDetectorClose
.csect contDetectorClose[DS]
contDetectorClose:
	.long .contDetectorClose, TOC[tc0], 0
.csect .text[PR]
.contDetectorClose:
	mflr 0
	stw 0,8(1)
	stwu 1,-56(1)
	li 0,0
	stw 0,16(3)
	lwz 0,12(3)
	cmpwi 0,0,0
	bc 12,2,L..7
	lwz 3,LC..23(2)
	bl .cprintf
	cror 31,31,31
L..7:
	li 3,0
	la 1,56(1)
	lwz 0,8(1)
	mtlr 0
	blr
	.balign 4
	.globl contDetectorInvalid
	.globl .contDetectorInvalid
.csect contDetectorInvalid[DS]
contDetectorInvalid:
	.long .contDetectorInvalid, TOC[tc0], 0
.csect .text[PR]
.contDetectorInvalid:
	mflr 0
	stw 0,8(1)
	stwu 1,-56(1)
	li 3,1
	bl .pseterr
	cror 31,31,31
	li 3,-1
	la 1,56(1)
	lwz 0,8(1)
	mtlr 0
	blr
.toc
LC..24:
	.tc LC..20[TC],LC..20
.csect .text[PR]
	.balign 4
	.globl contDetectorUninstall
	.globl .contDetectorUninstall
.csect contDetectorUninstall[DS]
contDetectorUninstall:
	.long .contDetectorUninstall, TOC[tc0], 0
.csect .text[PR]
.contDetectorUninstall:
	mflr 0
	stw 31,-4(1)
	stw 0,8(1)
	stwu 1,-64(1)
	mr 31,3
	lwz 0,12(31)
	cmpwi 0,0,0
	bc 12,2,L..63
	lwz 3,LC..24(2)
	bl .cprintf
	cror 31,31,31
L..63:
	lwz 3,4(31)
	bl .iointclr
	cror 31,31,31
	mr 3,31
	li 4,36
	bl .sysfree
	cror 31,31,31
	li 3,0
	la 1,64(1)
	lwz 0,8(1)
	mtlr 0
	lwz 31,-4(1)
	blr
_section_.text:
.csect .data[RW]
	.long _section_.text
