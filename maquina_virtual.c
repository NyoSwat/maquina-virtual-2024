#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include"maquina_virtual.h"

//retorna posicion del segmento contenido en los bytes del registro
 int Pos_Seg(MaquinaVirtual *mv, int segment) {
    int pos = 0, i = 0;
    int vec[5] = {KS,CS,DS,ES,SS};

    while ( i < 5 &&  )
    {
        if ( mv->registros[i] > 0 &&  ) {
            pos++;
        }
        i++;
    }

    return pos;
}

int corrigeSize(int size)
{
    int aux1 = 0, aux2 = 0;
    aux1 = (size >> 8) & 0x00FF;
    aux2 = (size & 0x00FF) << 8;
    return aux2 | aux1;
}


void cargaVF(Toperaciones *v){
    //2 operandos
    v[0x0] = MOV; v[0x1] = ADD;
    v[0x2] = SUB; v[0x3] = SWAP;
    v[0x4] = MUL; v[0x5] = DIV;
    v[0x6] = CMP; v[0x7] = SHL;
    v[0x8] = SHR; v[0x9] = AND;
    v[0xA] = OR;  v[0xB] = XOR;
    v[0xC] = RND;
    //1 operando
    v[0x10] = SYS; v[0x11] = JMP;
    v[0x12] = JZ;  v[0x12] = JP;
    v[0x13] = JN;  v[0x14] = JNZ;
    v[0x15] = JNP; v[0x16] = JNP;
    v[0x17] = JNN; v[0x18] = LDL;
    v[0x19] = LDH; v[0x1A] = NOT;
    v[0x1B] = PUSH; v[0x1C] = POP;
    v[0x1D] = CALL;
    //0 operandos
    v[0x1E] = RET; v[0x1F] = STOP;
}

//primer byte de la sentencia de 4
void LeerByte(char instruccion, char *operando1, char *operando2, unsigned int *operacion)
{
    *operacion = instruccion & 0x1F;
    *operando1 = (instruccion >> 4) & 0x03;
    *operando2 = (instruccion >> 6) & 0x03;
}

//aumenta la posicion del registro ip en funciona de la cantidad de bytes usadas por los operandos
void sumaIP(int *ip,char operando1,char operando2){
    int op1,op2;
    if(operando1 == 0x0){
        op1 = 3;
    }else{
        if(operando1 == 0x1){
            op1 = 2;
        }else{
            if(operando1 == 0x2)
                op1 = 1;
            else
                op1 = 0;
        }
    }
    if(operando2 == 0x0){ // Memoria
        op2 = 3;
    }else{
        if(operando2 == 0x1){ // inmediato
            op2 = 2;
        }else{
            if(operando2 == 0x2) // registro
                op2= 1;
            else
                op2 = 0;
        }
    }
    *ip += 1 + op1 + op2;
}

void InformaError(MaquinaVirtual *mv, Error error){
    char * typeError[4]= {"Instrucción invalida","Division por cero","Fallo de segmento","Memoria insuficiente"};
    printf("ERROR: %s ",typeError[error.code]);
    if(error.code==0)
        printf("Instruccion: %02X",error.invalidInstruction);
    mv->registros[IP] = mv->segmentos[CS].size;
}

void ejecutaCiclo(MaquinaVirtual *mv, char version, int ipAux)
{
    unsigned int operacion, ok;
    operando op[2];
    Toperaciones arrFunciones[32];
    Error error;

    cargaVF(arrFunciones);

    LeerByte(mv->Memoria[mv->registros[IP]],&(op[0].tipo),&(op[1].tipo),&operacion);

    sumaIP(&(mv->registros[IP]), op[0].tipo, op[1].tipo);

    recuperaOperandos(mv,op,ipAux);

    switch (version) {
        case 1: ok = (operacion >= 0 && operacion <= 0x0C) || (operacion >= 0x10 && operacion <= 0x1A) || (operacion == 0x1F);
            break;
        case 2: ok = (operacion >= 0 && operacion <= 0x0C) || (operacion >= 0x10 && operacion <= 0x1F);
    }
    if (ok) {
        *arrFunciones[operacion](mv,op);
    }
    else {
        error.code = 0;
        error.invalidInstruction = operacion;
        InformaError(mv,error);
    }
}


void creaArchivoDeImagen(MaquinaVirtual mv)
{
    FILE *archImagen;
    unsigned short int i, tamanio = ( mv.header[6] << 8 | mv.header[7] );
    char DD; // DD=descriptor de segmento

    archImagen=fopen(mv.imagenArchivo,"wb");
    if(archImagen != NULL)
    {
        for(i=0; i<8; i++)
            fprintf(archImagen,"%c",mv.header[i]);
        for(i=0; i<15; i++)
        {
            fprintf(archImagen, "%c", (mv.registros[i] & 0xFF000000) >> 24);
            fprintf(archImagen, "%c", (mv.registros[i] & 0x00FF0000) >> 16);
            fprintf(archImagen, "%c", (mv.registros[i] & 0x0000FF00) >> 8);
            fprintf(archImagen, "%c", mv.registros[i] & 0x000000FF);
        }
        for(i=0; i<8; i++){
            DD=mv.segmentos[i].base << 4 | mv.segmentos[i].size;
            fprintf(archImagen,"%c",DD);
        }
        for(i=0; i<tamanio; i++){
            fprintf(archImagen,"%c",mv.Memoria[i]);
        }
    }
    fclose(archImagen);
}

//recupera los operandos a partir de los datos en Memoria
void recuperaOperandos(MaquinaVirtual *mv,operando *op,int ip)
{
    char aux;
    int auxInt;
    for (int i = 0; i++; i<2)
    {
        auxInt = 0;
        switch(op[i].tipo) {

            case 0x00: //tipo Memoria
                aux = mv->Memoria[++ip];  //leo en un auxiliar el byte que dice el registro en el que se va a almacenar
                op[i].segmentoReg = (aux >> 6) & 0x03;
                aux = aux & 0x0F;
                op[i].registro = (int)aux;
                //printf("registro t mem op 1 %d\n",aux);

                auxInt |= mv->Memoria[++ip] << 8; //leo en un int auxiliar los 2 bytes que representan el desplazamiento de bytes
                auxInt |= (mv->Memoria[++ip] & 0x000000FF);

                op[i].desplazamiento = auxInt;
                //printf("desplazamiento t mem op 1 %d\n",aux);
                break;

            case 0x01: //tipo inmediato
                auxInt |= mv->Memoria[++ip] << 8; //leo en un int auxiliar los 2 bytes que representan el numero inmediato
                //printf("set inm %04X\n",auxInt);
                auxInt |= (mv->Memoria[++ip] & 0x00FF);
                //printf("set inm %04X\n",auxInt);
                op[i].desplazamiento = auxInt;
                break;

            case 0x02: //tipo registro
                aux = mv->Memoria[++ip];  //leo en un auxiliar el byte que dice el registro que voy a usar
                aux = aux & 0x0F;
                op[i].registro = aux;
                //printf("registro op %d\n",aux);
                aux = mv->Memoria[ip]; //leo en un auxiliar el byte y saco el segmento de registro
                aux = aux >> 4;
                op[i].segmentoReg = aux & 0x03;
                //printf("segmento t reg op 1 %d\n",aux);
                break;
        }
    }
}

int getReg(MaquinaVirtual *mv, operando op)
{
    int n;

    if ( op.segmentoReg == 0x00 )  // extended
        n = mv->registros[(int)op.registro];
    if ( op.segmentoReg == 0x01 ) { // AL
        n = mv->registros[(int)op.registro];
        n = n << 24;
        n = n >> 24;
    }
    if ( op.segmentoReg == 0x02 ) { // AH
        n = mv->registros[(int)op.registro];
        n = n << 16;
        n = n >> 24;
    }
    if ( op.segmentoReg == 0x03) { // AX
        n = mv->registros[(int)op.registro];
        n = n << 16;
        n = n >> 16;
    }

    return n;
}

int getMem(MaquinaVirtual *mv, operando op) {

    int num = 0;
    int posSeg = (mv->registros[op.registro] >> 16) & 0x0000000F;
    int puntero = (mv->registros[op.registro]) & 0x0000FFFF;

    if ( mv->segmentos[posSeg].base + op.desplazamiento + puntero >= mv->segmentos[posSeg].base  ) {
        if ( op.segmentoReg == 0x00 ) { //segmento de 4 bytes
             num |= mv->Memoria[mv->segmentos[posSeg].base + puntero + op.desplazamiento] << 24;
             num |= (0x00FF0000 & (mv->Memoria[mv->segmentos[posSeg].base + puntero + op.desplazamiento + 1] <<16));
             num |= (0x0000FF00 & (mv->Memoria[mv->segmentos[posSeg].base + puntero + op.desplazamiento + 2] <<8));
             num |= (0x000000FF & (mv->Memoria[mv->segmentos[posSeg].base + puntero + op.desplazamiento + 3]));
        } else {
            if ( op.segmentoReg == 0x02 ) { // segmento de 2 bytes
                num |= mv->Memoria[mv->segmentos[posSeg].base + puntero + op.desplazamiento] << 8;
                num |= mv->Memoria[mv->segmentos[posSeg].base + puntero + op.desplazamiento + 1];
            } else { //segmento de 1 byte
                 num = mv->Memoria[mv->segmentos[posSeg].base + puntero + op.desplazamiento];
            }
        }
    } else {
        printf("error de segmento\n");
    }
    return num;
}

int getOp(MaquinaVirtual *mv, operando op)
{
    char reg = 0x02, mem = 0x00, inm = 0x01;

    if (op.tipo == reg)
        return getReg(mv,op);
    if (op.tipo == mem )
        return getMem(mv,op);
    else
        return op.desplazamiento;
}

void setOp(MaquinaVirtual *mv, operando op, int num)
{
    int posSeg = (mv->registros[op.registro] >> 16) & 0x0000000F;
    int puntero = mv->registros[op.registro] & 0x0000FFFF;

    switch (op.tipo)
    {
        case 0x00: //memoria
            if ( mv->segmentos[posSeg].base + op.desplazamiento + puntero >= mv->segmentos[posSeg].base  ) {
                if ( op.segmentoReg == 0x00 ) { //segmento de 4 bytes
                    mv->Memoria[mv->segmentos[posSeg].base + puntero + op.desplazamiento] = (char)((num >> 24) & 0x000000FF);
                    mv->Memoria[mv->segmentos[posSeg].base + puntero + op.desplazamiento + 1] = (char)(num >> 16) & 0x000000FF;
                    mv->Memoria[mv->segmentos[posSeg].base + puntero + op.desplazamiento + 2] = (char)(num >> 8) & 0x000000FF;
                    mv->Memoria[mv->segmentos[posSeg].base + puntero + op.desplazamiento + 3] = (char)(num & 0x000000FF);
                    if ( mv->segmentos[posSeg].size <= puntero + op.desplazamiento + 3 )
                            mv->segmentos[posSeg].size += puntero + op.desplazamiento + 4;
                } else {
                    if ( op.segmentoReg == 0x02 ) { // segmento de 2 bytes
                        mv->Memoria[mv->segmentos[posSeg].base + puntero + op.desplazamiento] = (char)(num >> 8) & 0x000000FF;
                        mv->Memoria[mv->segmentos[posSeg].base + puntero + op.desplazamiento + 1] = (char)(num & 0x000000FF);
                        if ( mv->segmentos[posSeg].size <= puntero + op.desplazamiento + 1 )
                            mv->segmentos[posSeg].size += puntero + op.desplazamiento + 2;
                    } else { //segmento de 1 byte
                        mv->Memoria[mv->segmentos[posSeg].size + puntero + op.desplazamiento] = (char) num;
                        if ( mv->segmentos[posSeg].size <= puntero + op.desplazamiento )
                            mv->segmentos[posSeg].base += 1;
                    }
                }
            } else {
                printf("error de segmento \n");
                mv->registros[IP] = mv->segmentos[0].size;
            }
            break;

        case 0x02: //registro
            if (op.segmentoReg == 0x03) { //uso de los 2 ultimos bytes del registro
                mv->registros[(int)op.registro] &= 0xFFFF0000; //mascara para inicializar los 2 bytes
                mv->registros[(int)op.registro] |= (num & 0x0000FFFF); //ultimos 2 bytes del entero asignados

            } else if (op.segmentoReg == 0x02) { //segmento 3er byte
                mv->registros[(int)op.registro] &= 0xFFFF00FF; //limito el registro
                mv->registros[(int)op.registro] |= ((num & 0x000000FF) << 8); //asigno al 3er byte

            } else if ( op.segmentoReg == 0x01) { // segmneto 4to byte
                mv->registros[(int)op.registro] &= 0xFFFFFF00; //inicializa ultimo byte
                mv->registros[(int)op.registro] |= (num & 0x000000FF); //asigna el ultimo byte del entero al registro

            } else if (op.segmentoReg == 0x00) {
                mv->registros[(int)op.registro] = num; //registro extendido
            }
            break;
        default: //inmediato
            printf("no es posible asignar valor a un inmediato");
            //stop(mv)
            break;
    }
}

void imprimeOperando(operando op){
    char nombre[5];
    switch (op.tipo){

    case 0x00://memoria
            obtieneTAG(op.registro,0,nombre);
            switch (op.segmentoReg) {
            case 0:printf("l[%s + %d]",nombre,op.desplazamiento);
                break;
            case 2:printf("w[%s + %d]",nombre,op.desplazamiento);
                break;
            case 3:printf("b[%s + %d]",nombre,op.desplazamiento);
                break;
            default:printf("[%s + %d]",nombre,op.desplazamiento);
                break;
              }
        break;
    case 0x01://inmediato
            printf(" %04X",op.desplazamiento);
        break;
    case 0x02://registro
            obtieneTAG(op.registro,op.segmentoReg,nombre);
            printf("%s ",nombre);
        break;
    }
}

//OPERACIONES DEL LENGUAGE

//funcion de asignacion al Condition Code
void setCC(MaquinaVirtual *mv, int n){
    if ( n > 0 )
        mv->registros[CC] &= 0xCFFFFFFF; //0000
    else if ( n < 0)
        mv->registros[CC] &= 0x8FFFFFFF; //1000
    else
        mv->registros[CC] &= 0x4FFFFFFF; //0100
} // N (1 si resultado negativo) Z (1 si resultado cero)

//asigna al primer operando valor del segundo operando
void MOV(MaquinaVirtual *mv, operando *op)
{
    setOp(mv,op[0],getOp(mv,op[1]));
}
//operacion suma al primer operando el valor del segundo operando
void ADD(MaquinaVirtual *mv, operando *op){
    int aux = getOp(mv,op[0]) + getOp(mv,op[1]);
    setOp(mv,op[0],aux);
    setCC(mv,aux);
}
//Resta al primer operando el segundo y lo guarda en el priemro
void SUB(MaquinaVirtual *mv, operando *op){
    int aux = getOp(mv,op[0]) - getOp(mv,op[1]);
    setOp(mv,op[0],aux);
    setCC(mv,aux);
}
//intercambia los valores de los operandos
void SWAP(MaquinaVirtual *mv, operando *op){
    int aux = getOp(mv,op[0]);
    setOp(mv,op[0],getOp(mv,op[1]));
    setOp(mv,op[1],aux);
}
//multiplica el primer operando por el valor del segundo
void MUL(MaquinaVirtual *mv, operando *op){
    int aux = getOp(mv,op[0])*getOp(mv,op[1]);
    setOp(mv,op[0],aux);
    setCC(mv,aux);
}
//divide al primer operando por el segundo, guarda el resto en el registro AC
void DIV(MaquinaVirtual *mv, operando *op){
    if( getOp(mv,op[1]) ) {
        int aux = getOp(mv,op[0]) / getOp(mv,op[1]);
        setOp(mv,op[0],aux);
        mv->registros[AC] = getOp(mv,op[0]) % getOp(mv,op[1]);
        setCC(mv,aux);
    }
    else {
        printf("Error al intentar dividir por 0\n");
        STOP(mv,op);
    }
}
//resta del valor primer operando el valor del segundo, no la referencia, afecta al CC
void CMP(MaquinaVirtual *mv, operando *op){
    int aux = getOp(mv,op[0]) - getOp(mv,op[1]);
    setCC(mv,aux);
}
//corrimiento de bytes a la izquierda
void SHL(MaquinaVirtual *mv, operando *op){
    setOp(mv,op[0],getOp(mv,op[0]) << getOp(mv,op[1]));
}
//corrimiento de bytes a la derecha
void SHR(MaquinaVirtual *mv, operando *op){
    int aux = getOp(mv,op[0]) >> getOp(mv,op[1]);
    setOp(mv,op[0],aux);
}
void AND(MaquinaVirtual *mv, operando *op){
    int aux = getOp(mv,op[0]) & getOp(mv,op[1]);
    setOp(mv,op[0],aux);
    setCC(mv,aux);
}
void OR(MaquinaVirtual *mv, operando *op){
    int aux = getOp(mv,op[0]) | getOp(mv,op[1]);
    setOp(mv,op[0],aux);
    setCC(mv,aux);
}
void XOR(MaquinaVirtual *mv, operando *op){
    int aux = getOp(mv,op[0]) ^ getOp(mv,op[1]);
    setOp(mv,op[0],aux);
    setCC(mv,aux);
}
void RND(MaquinaVirtual *mv, operando *op){
    mv->registros[AC] = rand() % (getOp(mv,op[0]) - 1);
}

void SYS(MaquinaVirtual *mv, operando *op){
    funcionSys vecLlamadas[0x10];
    Sistema aux;
    int llamada = getOp(mv,op[0]);

    loadSYSOperationArray(vecLlamadas);
    if(llamada == 1 || llamada == 2){
        //printf("sys registro %02X\n",mv->registros[13]);
        aux.posicion = mv->registros[13];
        aux.cantidad = mv->registros[12] & 0x000000FF;
        aux.tamanio = (mv->registros[12] >> 8) & 0x000000FF;
        aux.formato = mv->registros[10] & 0x000000FF;
    }

    vecLlamadas[llamada](mv,aux);
}
void JMP(MaquinaVirtual *mv, operando *op){
    int aux = getOp(mv,op[0]);
    mv->registros[IP] = (unsigned int) aux;
}

void JZ(MaquinaVirtual *mv, operando *op){
    unsigned int aux = getOp(mv,op[0]);
    if (mv->registros[CC] & 0xF0000000 == 0x40000000) //r & 11 == 10 ==> r = 1000 => r=0
        mv->registros[IP] = aux;
}

void JP(MaquinaVirtual *mv, operando *op){
    unsigned int aux = getOp(mv,op[0]);
    if (mv->registros[CC] & 0xF0000000 == 0x00000000) // r & 11 == 00 => r = 00 => r > 0
        mv->registros[IP] = aux;
}
void JN(MaquinaVirtual *mv, operando *op){
    unsigned int aux = getOp(mv,op[0]);
    if (mv->registros[CC] & 0xF0000000 == 0x80000000) // r & 11 == 10 => r = 10 => r < 0
        mv->registros[IP] = aux;
}
void JNZ(MaquinaVirtual *mv, operando *op){
    unsigned int aux = getOp(mv,op[0]);
    if (mv->registros[CC] & 0xF0000000 != 0x40000000)
        mv->registros[IP] = aux;
}
void JNP(MaquinaVirtual *mv, operando *op){
    unsigned int aux = getOp(mv,op[0]);
    if (mv->registros[CC] & 0xF0000000 != 0x00000000)
        mv->registros[IP] = aux;
}
void JNN(MaquinaVirtual *mv, operando *op){
    unsigned int aux = getOp(mv,op[0]);
    if (mv->registros[CC] & 0xF0000000 != 0x80000000) // r & 11 == 10 => r = 10 => r < 0
        mv->registros[IP] = aux;
}
void LDL(MaquinaVirtual *mv, operando *op){
    mv->registros[AC] &= 0xFFFF0000;
    mv->registros[AC] |= (getOp(mv,op[0]) & 0x0000FFFF);
}
void LDH(MaquinaVirtual *mv, operando *op){
    mv->registros[AC] &= 0x0000FFFF;
    mv->registros[AC] |= (getOp(mv,op[0]) << 16) & 0xFFFF0000;
}
void NOT(MaquinaVirtual *mv, operando *op){
    int aux = getOp(mv,(op[0]));
    !aux;
    setOp(mv,op[0],aux);
    setCC(mv,aux);
}

void STOP(MaquinaVirtual *mv, operando *op){
    mv->registros[IP] = mv->segmentos[Pos_Seg(mv,CS)].size;
}

void PUSH(MaquinaVirtual *mv, operando *op)
{
    int aux;
    //printf("PUSH\n");
    mv->registros[SP] -= 4;
    short int puntero = mv->registros[SP] & 0x0000FFFF;
    if(mv->registros[SP] < mv->registros[Pos_Seg(mv,SS)]) {
        printf("ERROR: STACK OVERFLOW\n");
        STOP(mv,op);
    } else {
        aux=getOp(mv,op[0]);

        mv->Memoria[mv->segmentos[SS].base + puntero + 3] = (aux & 0x000000FF);
        mv->Memoria[mv->segmentos[SS].base + puntero + 2] = (aux >> 8) & 0x000000FF;
        mv->Memoria[mv->segmentos[SS].base + puntero + 1] = (aux >> 16) & 0x000000FF;
        mv->Memoria[mv->segmentos[SS].base + puntero] = (aux >> 24) & 0x000000FF;

        sumaIP(&(mv->registros[IP]),op[0].tipo,op[1].tipo);
    }
}

void POP(MaquinaVirtual *mv, operando *op)
{
    int aux = 0;
    short int puntero = mv->registros[6] & 0x0000FFFF;
    //printf("POP\n");
    if(mv->segmentos[SS].size < puntero){
        printf("ERROR: STACK UNDERFLOW\n");
        STOP(mv,op);
    }else{
        aux |= mv->Memoria[mv->segmentos[SS].base + puntero] << 24;
        aux |= (0x00FF0000 & (mv->Memoria[mv->segmentos[SS].base + puntero + 1] << 16));
        aux |= (0x0000FF00 & (mv->Memoria[mv->segmentos[SS].base + puntero + 2] << 8));
        aux |= (0x000000FF & (mv->Memoria[mv->segmentos[SS].base + puntero + 3]));
        setOp(mv,op[0],aux);
        mv->registros[SP]+=4;
        sumaIP(&(mv->registros[IP]),op[0].tipo,op[1].tipo);
    }
}

void CALL(MaquinaVirtual *mv, operando *op)
{
    int aux;
//    printf("CALL %02X\n",getOp(mv,op[0]));
    mv->registros[6] -= 4;
    short int puntero = mv->registros[SP] & 0x0000FFFF;
    if(mv->registros[SP] < mv->registros[SS]) {
        printf("ERROR: STACK OVERFLOW\n");
        STOP(mv,op);
    }else{
//        printf("entro a la asignacion de memoria del ip\n");

        aux = mv->registros[IP];

        mv->Memoria[mv->registros[SP] + puntero + 3] = (aux & 0x000000FF);
        mv->Memoria[mv->registros[SP] + puntero + 2] = (aux & 0x0000FF00)>>8;
        mv->Memoria[mv->registros[SP] + puntero + 1] = (aux & 0x00FF0000)>>16;
        mv->Memoria[mv->registros[SP] + puntero] = (aux & 0xFF000000)>>24;

        //printf("Va a entrar al JMP\n");

        mv->registros[IP] = getOp(mv,op[0]);
    }
}

void RET(MaquinaVirtual *mv, operando *op)
{
    int aux = 0;
    short int puntero = mv->registros[6] & 0x0000FFFF;

    //printf("RET\n ");
    if(mv->segmentos[Pos_Seg(mv,SS)].size < puntero){
        printf("ERROR: STACK UNDERFLOW\n");
        STOP(mv,op);
    }else{
//        printf("Entro al ret\n");
        aux |= mv->Memoria[mv->segmentos[SS].base + puntero] << 24;
        aux |= (0x00FF0000 & (mv->Memoria[mv->segmentos[SS].base + puntero + 1] << 16));
        aux |= (0x0000FF00 & (mv->Memoria[mv->segmentos[SS].base + puntero + 2] << 8));
        aux |= (0x000000FF & (mv->Memoria[mv->segmentos[SS].base + puntero + 3]));

        mv->registros[IP] = 0x0000FFFF & aux;

        mv->registros[SP] += 4;
        mv->registros[IP] += 3;
//        printf("IP RET: %04X\n",mv->registros[IP]);
    }
}

//FUNCIONES DEL SISTEMA

void loadSYSOperationArray(funcionSys *vecLlamadas)
{
    vecLlamadas[0x1] = readSys;
    vecLlamadas[0x2] = writeSys;
    vecLlamadas[0x3] = readStringSys;
    vecLlamadas[0x4] = writeStringSys;
    vecLlamadas[0x7] = clearScreen;
    vecLlamadas[0xF] = breakPoint;
}

void readSys(MaquinaVirtual *mv,Sistema aux) {
    int i = 0;
    int dato;
    operando auxOp;
    auxOp.tipo = 0;
    auxOp.registro = 13;
    auxOp.desplazamiento = 0;

    if(aux.tamanio == 4)
        auxOp.segmentoReg = 0;
    else
        auxOp.segmentoReg = aux.tamanio;

    int posSeg = (mv->registros[13] >> 16) & 0x0000000F;
    int puntero = (mv->registros[13]) & 0x0000FFFF;

    while(i<aux.cantidad)
    {
         printf("[%04X] ",puntero + auxOp.desplazamiento);
        switch (aux.formato) {
        case 1:
            scanf(" %d",&dato);
            setOp(mv,auxOp,dato);
            break;
        case 2:
            scanf(" %c",&dato);
            setOp(mv,auxOp,dato);
            break;
        case 4:
            scanf(" %o",&dato);
            setOp(mv,auxOp,dato);
            break;
        case 8:
            scanf(" %X",&dato);
            setOp(mv,auxOp,dato);
            break;
        }
        i++;
        auxOp.desplazamiento = i * aux.tamanio;
    }
}

void writeSys(MaquinaVirtual *mv, Sistema aux){
    int i = 0;
    operando auxOp; //creo una variable auxiliar para pedir a Memoria
    int auxint = 0;
    auxOp.registro = 13;
    auxOp.desplazamiento = 0;

    int posTDDS = (mv->registros[13] >>24) & 0x00000003;

    if(aux.tamanio == 4)
        auxOp.segmentoReg = 0; // si es 4 bytes asigno 0 por que asi va xd
    else
        auxOp.segmentoReg = aux.tamanio;
  //printf("formato de print %d\n",aux.formato);

    while(i < aux.cantidad) {
        printf("[%04X] ",(mv->registros[13] & 0x0000FFFF) - mv->segmentos[posTDDS].base + auxOp.desplazamiento);
        switch (aux.formato){
        case 1:
            printf("%d\n",getMem(mv,auxOp));
            break;
        case 2:
            auxint = getMem(mv,auxOp);
            if(32 <= auxint && auxint < 127)
                printf("%c %d\n",getMem(mv,auxOp));
            else
                printf(".... %d\n",auxint);
            break;
        case 4:
            printf("%o\n",getMem(mv,auxOp));
            break;
        case 8:
            printf("%08X\n",getMem(mv,auxOp));
            break;
        case 9:
            auxint = getMem(mv,auxOp);
            printf(" HEXA %08X # %d \n",auxint,auxint);
            break;
        case 15:
            auxint = getMem(mv,auxOp);
            if(32 <= auxint && auxint < 127)
                printf("' %c # %d @ %o  HEXA %08X\n",auxint,auxint,auxint,auxint);
            else
                printf("' .... # %d @ %o  HEXA %08X\n",auxint,auxint,auxint);

            break;
        default:
            i = aux.cantidad;
            break;
        }
        i++;
        auxOp.desplazamiento = i * aux.tamanio;
    }
}

void readStringSys(MaquinaVirtual *mv,Sistema aux)
{
    char *st;
    operando op;
    if(aux.tamanio > 0){
        st = (char *)malloc(aux.tamanio * sizeof(char));
    }else if(aux.tamanio == -1){
        st = (char *)malloc(16384 * sizeof(char));
    }
    op.tipo = 0;
    op.registro = 13;
    op.desplazamiento = 0;
    op.segmentoReg = 0x3;

    scanf(" %s",st);
    int i = 0;
    while(i < strlen(st)){
        setOp(mv,op,st[i]);
        i++;
        op.desplazamiento = i;
    }
}

void writeStringSys(MaquinaVirtual *mv,Sistema aux)
{
    char *st = (char * )malloc(16384 * sizeof(char));
    int cantCaracteres = 0;
    operando op;

    op.registro = 13;
    op.desplazamiento = 0;
    op.segmentoReg = 0x3;
    strcpy(st,"");
    if(aux.tamanio < 0){
        while(getMem(mv,op) != 0x00){
            //printf("%d ",getMem(mv,o));
            sprintf(st,"%s%c",st,getMem(mv,op));
            op.desplazamiento++;
        }
        sprintf(st,"%s%c",st,'\0');
    }else{
        while(getMem(mv,op) != 0x00 && cantCaracteres < aux.tamanio){
            printf("%c",getMem(mv,op));
            cantCaracteres++;
            op.desplazamiento++;
        }
    }
    printf("%s\n",st);
}

void clearScreen(MaquinaVirtual *mv,Sistema aux)
{
    system("cls");
}

void breakPoint(MaquinaVirtual *mv,Sistema aux)
{
    char key;

    if ( mv->imagenArchivo != NULL ) {
        do {
            creaArchivoDeImagen(*mv);
            scanf("%c\n",&key);
            if ( key == 13) {
                ejecutaCiclo(mv,mv->header[5],mv->registros[IP]);
            }
        } while ( key != 113 ); //para con el ingreso de la letra q
    }
}

//DISASSEMBLER

void cargaVectorDisassembler(funcionDisassembler *v){
    //operaciones de 2 operandos
    v[0x00] = imprimeMOV;
    v[0x01] = imprimeADD;
    v[0x02] = imprimeSUB;
    v[0x03] = imprimeSWAP;
    v[0x04] = imprimeMUL;
    v[0x05] = imprimeDIV;
    v[0x06] = imprimeCMP;
    v[0x07] = imprimeSHL;
    v[0x08] = imprimeSHR;
    v[0x09] = imprimeAND;
    v[0x0A] = imprimeOR;
    v[0x0B] = imprimeXOR;
    //operaciones de 1 operandos
    v[0x0C] = imprimeRND;
    v[0x10] = imprimeSYS;
    v[0x11] = imprimeJMP;
    v[0x12] = imprimeJZ;
    v[0x13] = imprimeJP;
    v[0x14] = imprimeJN;
    v[0x15] = imprimeJNZ;
    v[0x16] = imprimeJNP;
    v[0x17] = imprimeJNN;
    v[0x18] = imprimeLDL;
    v[0x19] = imprimeLDH;
    v[0x1A] = imprimeNOT;
    v[0x1B] = imprimePUSH;
    v[0x1C] = imprimePOP;
    v[0x1D] = imprimeCALL;
    //operaciones sin operandos
    v[0x1E] = imprimeRET;
    v[0x1F] = imprimeSTOP;
}

void obtieneTAG(char reg,char segmento,char nombre[]){
    switch (reg){
    case 0x00:strcpy(nombre,"CS");
        break;
    case 0x01:strcpy(nombre,"DS");
        break;
    case 0x02:strcpy(nombre,"ES");
        break;
    case 0x03:strcpy(nombre,"SS");
        break;
    case 0x04:strcpy(nombre,"KS");
        break;
    case 0x05:strcpy(nombre,"IP");
        break;
    case 0x06:strcpy(nombre,"SP");
        break;
    case 0x07:strcpy(nombre,"BP");
        break;
    case 0x08:strcpy(nombre,"CC");
        break;
    case 0x09:strcpy(nombre,"AC");
        break;
    case 0x0A:
        switch (segmento){
                case 0:strcpy(nombre,"EAX");
                    break;
                case 1:strcpy(nombre,"AL");
                    break;
                case 2:strcpy(nombre,"AH");
                    break;
                case 3:strcpy(nombre,"AX");
                    break;
        }
        break;
    case 0x0B:
        switch (segmento){
                case 0:strcpy(nombre,"EBX");
                    break;
                case 1:strcpy(nombre,"BL");
                    break;
                case 2:strcpy(nombre,"BH");
                    break;
                case 3:strcpy(nombre,"BX");
                    break;
        }
        break;
    case 0x0C:
        switch (segmento){
                case 0:strcpy(nombre,"ECX");
                    break;
                case 1:strcpy(nombre,"CL");
                    break;
                case 2:strcpy(nombre,"CH");
                    break;
                case 3:strcpy(nombre,"CX");
                    break;
        }
        break;
    case 0x0D:
        switch (segmento){
                case 0:strcpy(nombre,"EDX");
                    break;
                case 1:strcpy(nombre,"DL");
                    break;
                case 2:strcpy(nombre,"DH");
                    break;
                case 3:strcpy(nombre,"DX");
                    break;
        }
        break;
    case 0x0E:
        switch (segmento){
            case 0:strcpy(nombre,"EEX");
                break;
            case 1:strcpy(nombre,"EL");
                break;
            case 2:strcpy(nombre,"EH");
                break;
            case 3:strcpy(nombre,"EX");
                break;
        }
        break;
    case 0x0F:
        switch (segmento){
                case 0:strcpy(nombre,"EFX");
                    break;
                case 1:strcpy(nombre,"FL");
                    break;
                case 2:strcpy(nombre,"FH");
                    break;
                case 3:strcpy(nombre,"FX");
                    break;
        }
        break;
    }
}

void imprimeMOV(InstruccionDisassembler disInstruccion){
    printf("MOV ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeADD(InstruccionDisassembler disInstruccion){
    printf("ADD ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeSUB(InstruccionDisassembler disInstruccion){
    printf("SUB ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeSWAP(InstruccionDisassembler disInstruccion){
    printf("SWAP ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeMUL(InstruccionDisassembler disInstruccion){
    printf("MUL ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeDIV(InstruccionDisassembler disInstruccion){
    printf("DIV ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeCMP(InstruccionDisassembler disInstruccion){
    printf("CMP ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeSHL(InstruccionDisassembler disInstruccion){
    printf("SHL ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeSHR(InstruccionDisassembler disInstruccion){
    printf("SHR ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeAND(InstruccionDisassembler disInstruccion){
    printf("AND ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeOR(InstruccionDisassembler disInstruccion){
    printf("OR ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeXOR(InstruccionDisassembler disInstruccion){
    printf("XOR ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeSYS(InstruccionDisassembler disInstruccion){
    printf("SYS ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeJMP(InstruccionDisassembler disInstruccion){
    printf("JMP ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeJZ(InstruccionDisassembler disInstruccion){
    printf("JZ ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeJP(InstruccionDisassembler disInstruccion){
    printf("JP ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeJN(InstruccionDisassembler disInstruccion){
    printf("JN ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeJNZ(InstruccionDisassembler disInstruccion){
    printf("JNZ ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeJNP(InstruccionDisassembler disInstruccion){
    printf("JNP ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeJNN(InstruccionDisassembler disInstruccion){
    printf("JNN ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeLDL(InstruccionDisassembler disInstruccion){
    printf("LDL ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeLDH(InstruccionDisassembler disInstruccion){
    printf("LDH ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeRND(InstruccionDisassembler disInstruccion){
    printf("RND ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeNOT(InstruccionDisassembler disInstruccion){
    printf("NOT ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeSTOP(InstruccionDisassembler disInstruccion){
    printf("STOP ");
    printf("\n");
}
void imprimePUSH(InstruccionDisassembler disInstruccion){
    printf("PUSH ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimePOP(InstruccionDisassembler disInstruccion){
    printf("POP ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeCALL(InstruccionDisassembler disInstruccion){
    printf("CALL ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeRET(InstruccionDisassembler disInstruccion){
    printf("RET ");
    printf("\n");
}
