#ifndef OPERANDOS_H
#define OPERANDOS_H

#include "maquina_virtual.h"

typedef struct  {
    char tipo;
    char registro;
    char segmentoReg;
    int desplazamiento;
} operando;

int getReg(MaquinaVirtual *mv, operando op);

int getMem(MaquinaVirtual *mv, operando op);

int getOp(MaquinaVirtual *mv, operando op);

void setOp(MaquinaVirtual *mv, operando op, int);

#endif OPERANDOS_H