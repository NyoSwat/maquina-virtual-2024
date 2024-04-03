#ifndef OPERANDOS_H
#define OPERANDOS_H

#include "maquina_virtual.h"

typedef struct  {
    char tipo;
    char registro;
    char segmentoReg;
    int desplazamiento;
} operando;


int getOp(MaquinaVirtual *mv, operando op);

void setOp(MaquinaVirtual *mv, operando op, int);

#endif OPERANDOS_H