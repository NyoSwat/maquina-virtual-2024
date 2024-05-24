#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"maquina_virtual.h"


void asignaSegmento(MaquinaVirtual *mv, unsigned short int tamSegmentos[5] );
//inicializa la maquina virtual
void cargaMV(MaquinaVirtual *mv, char *[], int *numInstrucciones, unsigned int memoria , char *version);
//muestra el codigo assembler ingresado por archivo
void disassembler(MaquinaVirtual *mv);
//ejecucion del segmento de codigo
void ejecutarMV(MaquinaVirtual *mv, char version, int numInstrucciones);

// vmx.exe [filename.vmx] [filename.vmi] [m=M] [-d]
int main(int argc, char *argv[])
{
    MaquinaVirtual mv;
    int numInstrucciones = 0;
    int entraDisassembler = 0;
    char version;
    int entraCarga = 0;
    unsigned int memoriaParam;
    unsigned int aux;

    //validacion de argumentos opcionales
    for ( int i = 1; i < argc; i++)
    {
        int len = strlen(argv[i]);

        //verificar  [filename.vmx]
        if (len >= 4 && strcmp(argv[i] + len - 4, ".vmx") == 0) {
            entraCarga++;
        }

        //verificar [filename.vmi]
        if (len >= 4 && strcmp(argv[i] + len - 4, ".vmi") == 0) {
            strcpy(mv.imagenArchivo,argv[i]);
            printf("%s",mv.imagenArchivo);
        }

        // Verificar [m=M]
        if (sscanf(argv[i],"m=%d",&aux)) {
            printf("m=M: %s\n", argv[i]);

            memoriaParam = (int) argv[i];
        }

        // Verificar [-d]
        if (strcmp(argv[i], "-d") == 0) {
            entraDisassembler = 1;
        }
    }
    //printf("dissasembler: %d",entraDisassembler);
    if ( entraCarga )
    {
        printf("\nInicio carga: \n\n");
        cargaMV(&mv,argv,&numInstrucciones,memoriaParam,&version);
        printf("\nInicio ejecucion: \n\n");
        ejecutarMV(&mv,version,numInstrucciones);

        if ( entraDisassembler ) disassembler(&mv);
    }
    gets("Fin");
    return EXIT_SUCCESS;
}


void asignaSegmento(MaquinaVirtual *mv, unsigned short int tamSegmentos[5] )
{
    int contSegmentos = 0;
    int vecOrden[5] = { KS, CS, DS, ES, SS};
    unsigned short int sizeAnt = 0;

    for ( int i = 0; i < 5; i++) {
        if ( tamSegmentos[i] > 0) {
            if ( contSegmentos == 0 ) {
                mv->registros[vecOrden[i]] = 0;
            } else {
                mv->segmentos[vecOrden[i]].base = sizeAnt;
                mv->segmentos[vecOrden[i]].size = tamSegmentos[i];
                mv->registros[vecOrden[i]] = mv->segmentos[vecOrden[i]].base;
            }
            sizeAnt = tamSegmentos[i] + 1;
            contSegmentos++;
        } else {
            mv->registros[vecOrden[i]] = -1;
        }
    }
}

void cargaMV(MaquinaVirtual *mv, char* argv[], int *numInstrucciones, unsigned int memoria , char *version)
{
    char *header = (char*)malloc(6*sizeof(char));
    unsigned short int arrTamSegmentos[5];
    unsigned short int size = 0;
    int i, cont, totalSize = 0;
    unsigned short int offset = 0;
    FILE *arch = fopen("sample.vmx","rb");

    if( arch )
    {
        //comienza el proceso del archivo .vmx
        fgets(header, 6 * sizeof(char),arch); //obtentiene nombre del header
        if(strcmp(header,"VMX24") ) {
            printf("error de extensión");
        } else {

            //Tras verificar el encabezado, verifico la version de trabajo
            fread(version,sizeof(char),1,arch);
            printf("header: %s, version: %02X",header,*version);
            //inicializacion de segmentos
            if ( *version == 1)
            {
                fread(&size,sizeof(unsigned short int),1,arch);
                size = corrigeSize(size);
                mv->segmentos[CS].base = 0x0000;
                mv->segmentos[CS].size = size;
                mv->registros[CS] = 0;

                mv->segmentos[DS].base = ( mv->segmentos[CS].size + 1);
                mv->segmentos[DS].size = ( NUM_MEMORIA - mv->segmentos[CS].size );
                mv->registros[DS] = 0x00000000 | mv->segmentos[DS].base;

            } else if ( *version == 2 ) {

                //Carga el header y el nombre del archivo .vmi;
                for( i = 0; i < 5; i++ ) {
                    mv->header[i] = header[i];
                }
                mv->header[6] = (char) size >> 8;
                mv->header[7] = (char) size;

                //trabajo los segmentos de memoria
                for(int j = 0; j < 5; j++) {
                    fread(&size, sizeof(unsigned short int), 1, arch);
                    arrTamSegmentos[j] = size;
                    arrTamSegmentos[j] = corrigeSize(arrTamSegmentos[j]);
                }
                asignaSegmento(mv, arrTamSegmentos);

                //validacion del tamaño de memoria
                if ( memoria > 0 && memoria < totalSize ) {
                    printf( "\nERROR: MEMORIA INSUFICIENTE\n");
                    exit(EXIT_FAILURE);
                } else {
                    if ( memoria > 0 ) {
                        mv->Memoria = (char *) malloc( sizeof(char) * memoria );
                    } else {
                        mv->Memoria = (char *) malloc( sizeof(char) * NUM_MEMORIA);
                    }
                }

                fread(&offset, sizeof(unsigned short int), 1, arch);


            } else {
                printf(" error de version ");
                exit(EXIT_FAILURE);
            }
            //bloque de lectura del binario para cargarlo en Memoria
            cont = 0;
            while (fread(&(mv->Memoria[cont]),sizeof(char),1,arch))
            {
                cont++;
            }
            if (*version >  1) *numInstrucciones = cont;
        }
    }
    mv->registros[IP] = Pos_Seg(mv,CS) + offset;
    fclose(arch);
}


void ejecutarMV(MaquinaVirtual *mv, char version, int numInstrucciones)
{

    switch (version) {
    case 1:
        while(mv->registros[IP] < mv->segmentos[mv->registros[CS]].size)
            ejecutaCiclo(mv,version);
        break;
    case 2:
        while(mv->registros[IP] < mv->segmentos[Pos_Seg(mv,CS)].size || mv->registros[IP] < numInstrucciones)
            ejecutaCiclo(mv,version);
        break;
    }
}


void disassembler(MaquinaVirtual *mv)
{
    int i = 0;
    int ipAssembler = Pos_Seg(mv,CS);
    operando operandos[2];
    funcionDisassembler imprimeFuncion[0xF2];
    InstruccionDisassembler vecDisassembler[NUM_MEMORIA];
    unsigned int operacion;
    printf("\n-------------\n\nDisassembler: \n\n------------\n");
    while(ipAssembler < mv->segmentos[Pos_Seg(mv,CS)].size)
    {
        LeerByte(mv->Memoria[ipAssembler],&(operandos[0].tipo),&(operandos[1].tipo),&operacion);

        vecDisassembler[i].operandos->tipo = operandos[0].tipo;
        vecDisassembler[i].operandos->tipo = operandos[1].tipo;

        vecDisassembler->ipInicio = ipAssembler;
        vecDisassembler->codigoOperacion = operacion;

        recuperaOperandos(mv,operandos,ipAssembler);

        vecDisassembler[i].operandos[0] = operandos[0];
        vecDisassembler[i].operandos[1] = operandos[1];

        sumaIP(&(ipAssembler),operandos[0].tipo,operandos[1].tipo);

        vecDisassembler[i].ipFinal = ipAssembler;

        i++;
    }

    cargaVectorDisassembler(imprimeFuncion);
    vecDisassembler[0].ipInicio = 0;

    for(int z = 0; z < i; z++)
    {
        printf("[%04X] ",vecDisassembler[z].ipInicio);
        for(int ip = vecDisassembler[z].ipInicio; ip < vecDisassembler[z].ipFinal ;ip++){
           printf("%02X ",mv->Memoria[ip] & 0xFF);
        }

        printf("\t\t|\t");
        operacion = vecDisassembler[z].codigoOperacion;
        if((operacion >= 0 && operacion <= 12) || (operacion >= 0x10 && operacion <= 0x1F)){
            imprimeFuncion[vecDisassembler[z].codigoOperacion](vecDisassembler[z]);
        }

        printf("\n");
    }

}
