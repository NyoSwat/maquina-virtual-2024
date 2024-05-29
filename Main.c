#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"maquina_virtual.h"


//inicializa la maquina virtual
void cargaMV(MaquinaVirtual *mv, char *, int *numInstrucciones, unsigned int memoria , char *version);
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
    char *filename;
    int entraCarga = 0;
    unsigned int memoriaParam = 0;
    unsigned int aux;

    //validacion de argumentos opcionales
    for ( int i = 1; i < argc; i++)
    {
        int len = strlen(argv[i]);

        //verificar  [filename.vmx]
        if (len >= 4 && strcmp(argv[i] + len - 4, ".vmx") == 0) {
            entraCarga++;
            filename = argv[i];
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
        printf("\nInicio carga, archivo: %s \n\n\n", filename);
        cargaMV(&mv,filename,&numInstrucciones,memoriaParam,&version);
        //printf("\nInicio ejecucion: \n\n");
        ejecutarMV(&mv,version,numInstrucciones);

        if ( entraDisassembler ) disassembler(&mv);
    }
    gets("Fin");
    return EXIT_SUCCESS;
}


void cargaMV(MaquinaVirtual *mv, char* filename, int *numInstrucciones, unsigned int memoria , char *version)
{
    char *header = (char*)malloc(6*sizeof(char));
    unsigned short int arrTamSegmentos[5];
    unsigned short int size = 0;
    unsigned int antSize = 0;
    int i, cont, cantSeg = 0, totalSize = 0;
    unsigned short int offset = 0;

    FILE *arch = fopen(filename,"rb");

    if( arch )
    {
        //comienza el proceso del archivo .vmx
        fgets(header, 6 * sizeof(char),arch); //obtentiene nombre del header
        if(strcmp(header,"VMX24") ) {
            printf("error de extensión");
        } else {

            //Tras verificar el encabezado, verifico la version de trabajo
            fread(version,sizeof(char),1,arch);
            printf("header: %s, version: %02X\n",header,*version);

            //Carga el header y el nombre del archivo .vmi;
            for( i = 0; i < 5; i++ ) {
                mv->header[i] = header[i];
            }
            mv->header[6] = (char) size >> 8;
            mv->header[7] = (char) size;

            //inicializacion de segmentos
            mv->segmentos[0].base = 0;

            if ( *version == 1)
            {
                fread(&size,sizeof(unsigned short int),1,arch);
                size = corrigeSize(size);
                mv->segmentos[CS].size = size;
                mv->registros[0] = 0;
                mv->segmentos[DS].base = ( mv->segmentos[CS].size + 1);
                mv->segmentos[DS].size = ( NUM_MEMORIA - mv->segmentos[DS].base );
                mv->registros[DS] = (0x00000000 | mv->segmentos[DS].base) << 16;
                mv->registros[ES] = -1;
                mv->registros[SS] = -1;
                mv->registros[KS] = -1;

                mv->Memoria = (char *) malloc( sizeof(char) * NUM_MEMORIA );

            } else if ( *version == 2 ) {
                //trabajo los segmentos de memoria
                for(int j = 0; j < 5; j++)
                {
                    fread(&size, sizeof(unsigned short int), 1, arch);
                    arrTamSegmentos[j] = size;
                    arrTamSegmentos[j] = corrigeSize(arrTamSegmentos[j]);
                }
                cantSeg = 0;
                if ( arrTamSegmentos[KS] > 0 ) {

                    mv->segmentos[0].base = 0;
                    mv->segmentos[0].size = arrTamSegmentos[KS];
                    mv->registros[KS] = 0;
                    totalSize += arrTamSegmentos[KS];
                    antSize = arrTamSegmentos[KS];
                    cantSeg++;
                } else {
                    mv->registros[KS] = -1;
                }
                for (int  k = 0; k < 4; k++)
                    if ( arrTamSegmentos[k] > 0 ) {
                        if ( k == 0 && mv->registros[KS] == -1 ) {
                            mv->segmentos[k].base = 0;
                        } else {
                            mv->segmentos[k].base = antSize + 1;
                        }
                        mv->segmentos[k].size = arrTamSegmentos[k];
                        mv->registros[k] = mv->segmentos[k].base << 16;
                        mv->registros[k] &= 0xFFFF0000;
                        if ( k == 3 ) {
                            mv->registros[SP] = (cantSeg + 1) << 16;
                        }
                        totalSize += arrTamSegmentos[k];
                        antSize = arrTamSegmentos[k];
                        cantSeg++;
                    } else {
                        mv->registros[k];
                    }


                //validacion del tamaño de memoria
                if ( ((memoria > 0) && (memoria < totalSize)) || ((memoria == 0) && (NUM_MEMORIA < totalSize))  ) {
                    printf( "\nERROR: MEMORIA INSUFICIENTE\n");
                    exit(EXIT_FAILURE);
                } else {
                    if ( memoria > 0 ) {
                        mv->Memoria = (char *) malloc( sizeof(char) * memoria );
                    } else {
                        mv->Memoria = (char *) malloc( sizeof(char) * NUM_MEMORIA);
                    }
                }
                //lectura del offset
                fread(&offset, sizeof(unsigned short int), 1, arch);
            }

            //bloque de lectura del binario para cargarlo en Memoria
            cont = 0;
            while (fread(&(mv->Memoria[cont]),sizeof(char),1,arch))
            {
                //printf("%08X ",mv->Memoria[cont]);
                cont++;
            }
            if (*version >  1) *numInstrucciones = cont;
        }
    mv->registros[IP] = mv->segmentos[Pos_Seg(mv,CS)].base + offset;
    } else {
        printf("el archivo esta vacio");
    }
    fclose(arch);
}


void ejecutarMV(MaquinaVirtual *mv, char version, int numInstrucciones)
{
    int ipAux;
    int cont = 0;
    printf("\n-------------\nEjecucion: \n\n------------\n");
    while( cont < ( mv->segmentos[Pos_Seg(mv,CS)].base + mv->segmentos[Pos_Seg(mv,CS)].size ) ){
        ipAux = mv->registros[IP];
        ejecutaCiclo(mv,version,ipAux);
        cont++;
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
    while(ipAssembler < (mv->segmentos[].base + mv->segmentos[].size) )
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
        for(int ip = vecDisassembler[z].ipInicio; ip < vecDisassembler[z].ipFinal ; ip++){
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
