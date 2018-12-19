#include "dictionary.h"
#include "compile.h"

void include(dictionary *global_space, char *var_name, unsigned char *data, unsigned int data_size){
	variable *var_pointer;

	var_pointer = create_variable(GLOBAL, 0, var_name);
	var_pointer->is_function = 1;
	var_pointer->is_data = 1;
	var_pointer->data = data;
	var_pointer->data_size = data_size;
	write_dictionary(global_space, var_name, var_pointer, 0);
}

void include68k(dictionary *global_space){
	include(global_space, "putchar",
			"	move.w #0,D0\n"
			"	move.w #1,D1\n"
			"	move.l A7,A1\n"
			"	adda.l #3,A1\n"
			"	trap #15\n"
			"	adda.l #4,A7\n"
			"	rts\n",0);
	include(global_space, "__mul",
			"	move.l (A7),D7\n"
			"	move.l 4(A7),D0\n"
			"	mulu.w D0,D7\n"
			"	move.l D0,D1\n"
			"	mulu.w (A7)+,D0\n"
			"	swap D1\n"
			"	mulu.w (A7)+,D1\n"
			"	add.w D1,D0\n"
			"	swap D0\n"
			"	clr.w D0\n"
			"	add.l D0,D7\n"
			"	adda.l #4,A7\n"
			"	move.l D7,4(A7)\n"
			"	rts",0);
	include(global_space, "__div",
			"	move.l (A7)+,D0\n"
			"	move.l (A7)+,D1\n"
			"	tst.l	D0\n"
			"	bpl.s	__divNumPos\n"
			"	neg.l	D0\n"
			"	tst.l	D1\n"
			"	bpl.s	__divDenomPos\n"
			"	neg.l	D1\n"
			"	bsr.s	__udiv\n"
			"	neg.l	D1\n"
			"	rts\n"
			"\n"
			"__divDenomPos:\n"
			"	bsr.s	__udiv\n"
			"	neg.l	D0\n"
			"	neg.l	D1\n"
			"	rts\n"
			"\n"
			"__divNumPos:\n"
			"	tst.l	D1\n"
			"	bpl.s	__udiv\n"
			"	neg.l	D1\n"
			"	bsr.s	__udiv\n"
			"	neg.l	D0\n"
			"	rts\n"
			"\n"
			"__udiv:\n"
			"	move.l	D1,-(A7)\n"
			"	tst.w	(A7)+\n"
			"	bne.s	__udivBigDenom\n"
			"	swap.w	D0\n"
			"	move.w	D0,D1\n"
			"	beq.s	__udivSmallNum\n"
			"	divu.w	(A7),D1\n"
			"	move.w	D1,D0\n"
			"\n"
			"__udivSmallNum:\n"
			"	swap.w	D0\n"
			"	move.w	D0,D1\n"
			"	divu.w	(A7)+,D1\n"
			"	move.w	D1,D0\n"
			"	clr.w	D1\n"
			"	swap	D1\n"
			"	move.l D0,4(A7)\n"
			"	rts\n"
			"\n"
			"__udivBigDenom:\n"
			"	moveq	#15,D3\n"
			"	move.w	D3,D2\n"
			"	exg	D3,D1\n"
			"	swap	D0\n"
			"	move.w	D0,D1\n"
			"	clr.w	D0\n"
			"\n"
			"__udivDMls:\n"
			"	add.l	D0,D0\n"
			"	addx.l	D1,D1\n"
			"	cmp.l	D1,D3\n"
			"	bhi.s	__udivDMle\n"
			"	sub.l	D3,D1\n"
			"	addq.w	#1,D0\n"
			"\n"
			"__udivDMle:"
			"	dbf	D2,__udivDMls\n"
			"	adda.l #2,A7\n"
			"	move.l D0,4(A7)\n"
			"	rts",0);

}
