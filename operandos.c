#include<operandos.h>

int getReg(MaquinaVirtual *mv, operando op){
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

