#include<stdio.h>
#include<stdlib.h>
#include"maquina_virtual.h"
#include"operandos.h"


int corrigeRango(int rango){
    int aux1 = 0, aux2 = 0;
    aux1 = (rango >> 8) & 0x00FF;
    aux2 = (rango & 0x00FF) << 8;
    return aux2 | aux1;
}

void cargaMV(MaquinaVirtual *mv, char argv[], int *falloArch)
{
    char *header = (char*)malloc(sizeof(char));
    char version;
    int rango;
    FILE *archVMX = fopen(argv,"rb");

    if( archVMX == NULL ) {
        perror("Error al abrir el archivo");
        *falloArch = 1;
    } else {
        //comienza el proceso del archivo .vmx
        fgets(header, 6 * sizeof(char), archVMX ); //obtentiene el header
        if(stcmp(header,"VMX24") ) {
            printf("error de extensión");
        } else {
            //Tras verificar el encabezado, comienzo a leer el programa
            fread(version,sizeof(char),1,archVMX);//leo la version
            fread(rango,sizeof(unsigned short int),1,archVMX);
            rango = corrigeRango(rango); 
            printf("[ %S, %c, %d ]\n",header,version,rango);

            //inicializa segmentos
            mv->segmentos[0].base = 0x0000;
            mv->segmentos[0].size = rango;

            mv->registros[0] = 0;
            
            //bloque de lectura del binario para cargarlo en memoria
            int cont = 0;
            while (fread(&(mv->Memoria[cont]),sizeof(char),1,archVMX))
            {
                if (cont < mv->segmentos[0].size) {
                    
                } else {
                    mv->segmentos[1].size += 1;
                }
                cont++;
            }
            
        }
    }
    fclose(archVMX);
}

void ejecutarMV(MaquinaVirtual *mv)
{
    operando op[2];
    Toperaciones arrFunciones[256];

    cargaVF(arrFunciones);

    

}


//funcion de asignacion al Condition Code
void setCC(MaquinaVirtual *mv, int n){
    if ( n > 0 )
        mv->registros[CC] = 0x00000000; //0000
    else if ( n < 0) 
        mv->registros[CC] = 0x80000000; //1000
    else 
        mv->registros[CC] = 0x40000000; //0100
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
    int aux = getOp(mv,op[0]) + getOp(mv,op[1]);
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

}
void SYS(MaquinaVirtual *mv, operando *op){

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

}
void LDH(MaquinaVirtual *mv, operando *op){

}
void NOT(MaquinaVirtual *mv, operando *op){
    int aux = getOp(mv,(op[0]));
    !aux;
    setOp(mv,op[0],aux);
    setCC(mv,aux);
}

void STOP(MaquinaVirtual *mv, operando *op){
    mv->registros[IP] = mv->segmentos[0].size; 
}
