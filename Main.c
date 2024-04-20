#include<stdio.h>
#include<stdlib.h>
#include"maquina_virtual.h"

int main(int argc, char const *argv[])
{
    int falloArch = 0;
    MaquinaVirtual mv;

    //verificar argumentos
    if( argc < 2 || argc > 3){ 
        return EXIT_FAILURE;
    }
    // Obtener la extensión del archivo proporcionado
    char *extension = strrchr(argv[1], '.');
    // Verificar que la extensión sea .vmx
    if (extension == NULL || strcmp(extension, ".vmx") != 0) {
        fprintf(stderr, "Error: El archivo debe tener la extensión .vmx\n");
        return EXIT_FAILURE;
    } else {
        //carga el contenido del archivo en el segmento de datos
        cargaMV(&mv,argv[1], &falloArch);
        if ( falloArch ) 
            return EXIT_FAILURE;
    }
    //verificar el argumento "-d" para el modo debug
    if ( argc == 3 && strcmp(argv[2],"-d") == 0 ) {
       dissasembler(&mv);
    }
    ejecutarMV(&mv);

    return EXIT_SUCCESS;
}
