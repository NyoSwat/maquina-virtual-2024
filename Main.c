#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"maquina_virtual.h"

// vmx.exe [filename.vmx] [filename.vmi] [m=M] [-d]
int main(int argc, char *argv[])
{
    MaquinaVirtual mv;
    int entraDisassembler = 0;
    int entraCarga = 0;
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

        }

        // Verificar [-d]
        if (strcmp(argv[i], "-d") == 0) {
            entraDisassembler = 1;
        }
    }
    //printf("%d",entraDisassembler);
    //carga 
    //ejecucion
    //disassembler
    gets("Fin");
    return EXIT_SUCCESS;
}
