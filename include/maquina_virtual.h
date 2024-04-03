// maquina_virtual.h
#ifndef MAQUINA_VIRTUAL_H
#define MAQUINA_VIRTUAL_H

#include "operandos.h"

#define NUM_REGISTROS 16
#define NUM_SEGMENTOS 8
#define NUM_MEMORIA 16384


//estructura del procesador de la maquina virtual
typedef struct {
    char Memoria[NUM_MEMORIA];  
    unsigned long int  registros[NUM_REGISTROS];    
    struct segmentos{
       short int base,size;
    }segmentos[NUM_SEGMENTOS];
} MaquinaVirtual;

// Funciones para operar sobre la máquina virtual
void cargaMV(MaquinaVirtual *mv, char*, int);

void disassembler(MaquinaVirtual mv);

void ejecutarMV(MaquinaVirtual *mv);


#endif // MAQUINA_VIRTUAL_H
