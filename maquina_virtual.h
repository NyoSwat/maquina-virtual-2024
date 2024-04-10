// maquina_virtual.h
#ifndef MAQUINA_VIRTUAL_H
#define MAQUINA_VIRTUAL_H

#include "operandos.h"

#define NUM_REGISTROS 16
#define NUM_SEGMENTOS 8
#define NUM_MEMORIA 16384

#define CS 0
#define DS 1
#define IP 5
#define CC 8
#define AC 9


//estructura del procesador de la maquina virtual
typedef struct {
    char Memoria[NUM_MEMORIA];  
    unsigned long int  registros[NUM_REGISTROS];    
    struct segmentos{
       short int base,size;
    }segmentos[NUM_SEGMENTOS];
} MaquinaVirtual;


//funciones de utilidad
int corrigeRango(int rango);

void inicializaSegmento(MaquinaVirtual *mv);

// Funciones para operar sobre la máquina virtual

void cargaMV(MaquinaVirtual *mv, char*, int*);

void disassembler(MaquinaVirtual mv);

void ejecutarMV(MaquinaVirtual *mv);


//tipode funcion para operaciones de la maquina virtual
typedef void (*Toperaciones)(MaquinaVirtual *, operando *);

void cargaVF(Toperaciones *);

//funciones de assembler
void setCC(MaquinaVirtual *mv, int );
void MOV(MaquinaVirtual *mv, operando *op);
void ADD(MaquinaVirtual *mv, operando *op);
void SUB(MaquinaVirtual *mv, operando *op);
void SWAP(MaquinaVirtual *mv, operando *op);
void MUL(MaquinaVirtual *mv, operando *op);
void DIV(MaquinaVirtual *mv, operando *op);
void CMP(MaquinaVirtual *mv, operando *op);
void SHL(MaquinaVirtual *mv, operando *op);
void SHR(MaquinaVirtual *mv, operando *op);
void AND(MaquinaVirtual *mv, operando *op);
void OR(MaquinaVirtual *mv, operando *op);
void XOR(MaquinaVirtual *mv, operando *op);
void RND(MaquinaVirtual *mv, operando *op);
void SYS(MaquinaVirtual *mv, operando *op);
void JMP(MaquinaVirtual *mv, operando *op);
void JZ(MaquinaVirtual *mv, operando *op);
void JP(MaquinaVirtual *mv, operando *op);
void JN(MaquinaVirtual *mv, operando *op);
void JNZ(MaquinaVirtual *mv, operando *op);
void JNP(MaquinaVirtual *mv, operando *op);
void JNN(MaquinaVirtual *mv, operando *op);
void LDL(MaquinaVirtual *mv, operando *op);
void LDH(MaquinaVirtual *mv, operando *op);
void NOT(MaquinaVirtual *mv, operando *op);
void STOP(MaquinaVirtual *mv, operando *op);


#endif // MAQUINA_VIRTUAL_H
