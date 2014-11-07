//#ifdef _DEBUG

#include <stdio.h>
#include <stdlib.h>
#include "easytype.h"

EERR divs(FLOAT64* r, FLOAT64 a, FLOAT64 b)
{
    if ( b == 0.0 )
    {
        return_err(1,"div","Division by 0.0",NULL);
    }

    *r = a / b;

    return NULL;
}

EERR calc(FLOAT64 a, FLOAT64 b, BYTE op)
{
    if ( op == 0 )
    {
        FLOAT64 r;
        EERR e = divs( &r, a, b);
        if (e)
        {
            return_err(2,"calc","problema in operazione",e);
        }
        printf("%lf/%lf=%lf\n",a,b,r);
    }
    else
    {
        return_err(3,"calc","operazione non consetita",NULL);
    }

    return NULL;
}

int main()
{
    printf("EasyType v.Alpha\n");

    UINT64 test =S64(0);
    test = S64(300000000 * 12);
    printf("%" lld "\n\n",test);


    TUPLE si;
    tuple_new(si,CHAR*,UINT32);

    tuple(si,CHAR*,0) = malloc(10);
    strcpy(tuple(si,CHAR*,0),"mio");
    tuple(si,UINT32,1) = 1;

    printf("%s %d \n\n",tuple(si,CHAR*,0),tuple(si,UINT32,1));



    EERR e;
    puts("Calcola 10/2:");
    if ( (e = calc( 10, 2, 0)) )
    {
        err_printup(e);
        err_freeup(e);
    }
    puts("\nCalcola 10?2:");
    if ( (e = calc( 10, 2, 1)) )
    {
        err_printup(e);
        err_freeup(e);
    }
    puts("\nCalcola 10/0.0:");
    if ( (e = calc( 10, 0.0, 0)) )
    {
        err_printup(e);
        err_freeup(e);
    }


    return 0;
}

//#endif
