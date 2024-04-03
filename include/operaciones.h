#ifndef OPERACIONES_H
#define OPERACIONES_H
#include "maquina_virtual.h"
#include "operandos.h"

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
void setCC(MaquinaVirtual *mv,int numero);
void STOP(MaquinaVirtual *mv, operando *op);

#endif OPERACIONES_H