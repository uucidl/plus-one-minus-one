/* a10 3
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/generators/8bit-cpu.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 3 */



#ifndef _8BIT_CPU_H
#define _8BIT_CPU_H

/*
  a stupid cpu processing an accumulator
*/

typedef enum {
    NOP,
    AND,
    OR,
    XOR,
    NEG,
    SHIFT,
    ADD,
    MUL
} opcode_t;

typedef struct
{
    opcode_t      opcode;
    unsigned char parameter;
} operator_t;

typedef struct
{
    operator_t    ops[8];
    unsigned char accum;
} _8pu_t;

static inline
void _8pu_init(_8pu_t* cpu, unsigned char accum)
{
    int i;
    for(i=0; i<8; i++) {
	cpu->ops[i].opcode    = NOP;
	cpu->ops[i].parameter = 0;
    }
    cpu->accum = accum;
}

static inline
void _8pu_next(_8pu_t* cpu)
{
    int i;
    for(i=0; i<8; i++) {
	operator_t* op = cpu->ops+i;
	switch(op->opcode) {
	case NOP:
	    break;
	case AND:
	    cpu->accum &= op->parameter;
	    break;
	case OR:
	    cpu->accum |= op->parameter;
	    break;
	case XOR:
	    cpu->accum ^= op->parameter;
	    break;
	case NEG:
	    cpu->accum = ~cpu->accum;
	    break;
	case SHIFT:
	    cpu->accum <<= op->parameter;
	    break;
	case ADD:
	    cpu->accum += op->parameter;
	    break;
	case MUL:
	    {
		const int temp = cpu->accum * op->parameter;
		cpu->accum = temp / 256;
	    }
	    break;
	}
    }
}

/* type of a specialised generator */
typedef void (*gen_t)(void); 
/* specialized block generator */
typedef void (*block_gen_t)(unsigned char* block, unsigned int n);

gen_t _8pu_specialize(_8pu_t* cpu, unsigned char* code);
gen_t _8pu_specialize_static(_8pu_t* cpu, unsigned char* code);

block_gen_t _8pu_specialize_block(_8pu_t* cpu, unsigned char* code);

#endif
