#include<C:\Users\Edda\Desktop\arquitectura de computadoras\Maquina Virtual\include\operandos.h>

int getOp(MaquinaVirtual *mv, operando op)
{
    char reg = 0x02, mem = 0x00, inm = 0x01;

    if (op.tipo == reg){
        return getReg();
    } else if (op.tipo == mem ) {
        return getMem();
    } else {
        return op.desplazamiento;
    }
}

void setOp(MaquinaVirtual *mv, operando op, int num)
{
    switch (operando.tipo)
    {
    case 0x00: //memoria
        
        break;
    case 0x02: //registro
    
        break;
    default: //inmediato
        printf("no es posible asignar valor a un inmediato");
        //stop(mv)
        break;
    }
}

