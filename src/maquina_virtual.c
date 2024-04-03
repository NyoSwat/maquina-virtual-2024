#include"maquina_virtual.h"

void cargaMV(MaquinaVirtual *mv, char argv[], int falloArch)
{
    char *header = (char*)malloc(sizeof(char));
    char version;
    int rango;
    FILE *archVMX = fopen(argv,"rb");

    if( *archVMX == NULL ) {
        perror("Error al abrir el archivo");
    } else {
        //comienza el proceso del archivo .vmx
        fgets(header, 6 * sizeof(char), archVMX ); //obtentiene el header
        if(stcmp(header,"VMX24") ) {
            printf("error de extensión");
        } else {
            //Tras verificar el encabezado, comienzo a leer el programa
            fread(version,sizeof(char),1,archVMX);//leo la version
            fread(rango,sizeof(unsigned short int),1,archVMX);

            printf("[ %S, %c, %d ]\n",header,version,rango);

        }
    }
    fclose(archVMX);
}