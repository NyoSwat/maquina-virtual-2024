#define NUM_REGISTROS 16
#define NUM_TABLA_SEGMENTOS 8
#define NUM_MEMORIA 16384

#define CS 0
#define DS 1
#define ES 2
#define SS 3
#define KS 4
#define IP 5
#define SP 6
#define BP 7
#define CC 8
#define AC 9

/*
typedef struct {
    char header[8];
    char imagenArchivo[16];
}imagen;*/

//estructura del procesador de la maquina virtual
typedef struct {
    char *Memoria;
    int  registros[NUM_REGISTROS];
    struct segmentos{
       unsigned short int base,size;
    }segmentos[NUM_TABLA_SEGMENTOS];
    char header[8];
    char imagenArchivo[16];
} MaquinaVirtual;

//estructura de los operandos del lenguaje assembler
typedef struct  {
    char tipo;
    char registro;
    char segmentoReg;
    int desplazamiento;
} operando;

//estructura para trabajar con las funciones de llamada al sistema operativo
typedef struct {
    unsigned int posicion;
    char cantidad;
    char tamanio;
    char formato;
}Sistema;

//estructura para reporte de errores
typedef struct {
    int code,invalidInstruction;
}Error;

//funciones modularizadas para las funciones de la maquina
int Pos_Seg(MaquinaVirtual *mv, int segment);
int corrigeSize(int rango);
void LeerByte(char instruccion, char *op1, char *op2, unsigned int *operacion);
void sumaIP(int *ip,char operando1,char operando2);
void InformaError(MaquinaVirtual *mv, Error error);
void ejecutaCiclo(MaquinaVirtual *mv, char version, int ipAux);

//creacion de archivo de imagen vmi
void creaArchivoDeImagen(MaquinaVirtual mv);

//funciones para emplear los operandos de las operaciones del lenguaje assembler
int getReg(MaquinaVirtual *mv, operando op);
int getMem(MaquinaVirtual *mv, operando op);
int getOp(MaquinaVirtual *mv, operando op);
void setOp(MaquinaVirtual *mv, operando op, int);
void recuperaOperandos(MaquinaVirtual *mv,operando *o,int ip);
void imprimeOperando(operando op);


//tipo de funcion para operaciones de la maquina virtual
typedef void (*Toperaciones)(MaquinaVirtual *, operando *);
typedef void (*funcionSys)(MaquinaVirtual *,Sistema);

void cargaVF(Toperaciones *);
void loadSYSOperationArray(funcionSys *vecLlamadas);

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
void PUSH(MaquinaVirtual *mv, operando *op);
void POP(MaquinaVirtual *mv, operando *op);
void CALL(MaquinaVirtual *mv, operando *op);
void RET(MaquinaVirtual *mv, operando *op);

//funciones de la llamada sys
void readSys(MaquinaVirtual *mv,Sistema aux);
void writeSys(MaquinaVirtual *mv,Sistema aux);
void readStringSys(MaquinaVirtual *mv,Sistema aux);
void writeStringSys(MaquinaVirtual *mv,Sistema aux);
void clearScreen(MaquinaVirtual *mv,Sistema aux);
void breakPoint(MaquinaVirtual *mv,Sistema aux);


//DISASSEMBLER
typedef struct{
    unsigned int ipInicio;
    unsigned int ipFinal;
    unsigned int codigoOperacion;
    operando operandos[2];
}InstruccionDisassembler;

typedef void (*funcionDisassembler)(InstruccionDisassembler );

void cargaVectorDisassembler(funcionDisassembler *v);

void imprimeMOV(InstruccionDisassembler disInstruccion);
void imprimeADD(InstruccionDisassembler disInstruccion);
void imprimeSUB(InstruccionDisassembler disInstruccion);
void imprimeSWAP(InstruccionDisassembler disInstruccion);
void imprimeMUL(InstruccionDisassembler disInstruccion);
void imprimeDIV(InstruccionDisassembler disInstruccion);
void imprimeCMP(InstruccionDisassembler disInstruccion);
void imprimeSHL(InstruccionDisassembler disInstruccion);
void imprimeSHR(InstruccionDisassembler disInstruccion);
void imprimeAND(InstruccionDisassembler disInstruccion);
void imprimeOR(InstruccionDisassembler disInstruccion);
void imprimeXOR(InstruccionDisassembler disInstruccion);
void imprimeSYS(InstruccionDisassembler disInstruccion);
void imprimeJMP(InstruccionDisassembler disInstruccion);
void imprimeJZ(InstruccionDisassembler disInstruccion);
void imprimeJP(InstruccionDisassembler disInstruccion);
void imprimeJN(InstruccionDisassembler disInstruccion);
void imprimeJNZ(InstruccionDisassembler disInstruccion);
void imprimeJNP(InstruccionDisassembler disInstruccion);
void imprimeJNN(InstruccionDisassembler disInstruccion);
void imprimeLDL(InstruccionDisassembler disInstruccion);
void imprimeLDH(InstruccionDisassembler disInstruccion);
void imprimeRND(InstruccionDisassembler disInstruccion);
void imprimeNOT(InstruccionDisassembler disInstruccion);
void imprimeSTOP(InstruccionDisassembler disInstruccion);
void imprimePUSH(InstruccionDisassembler disInstruccion);
void imprimePOP(InstruccionDisassembler disInstruccion);
void imprimeCALL(InstruccionDisassembler disInstruccion);
void imprimeRET(InstruccionDisassembler disInstruccion);


void obtieneTAG(char reg,char segmento,char nombre[]);
