#ifdef _APP

#include <stdio.h>
#include <stdlib.h>
#include "easymath.h"

int main()
{
    printf("Debug Math!\n");

    mth_initrandom();

    int n = mth_random(10);
    printf("Random 1 to 10 : %d\n",n);

    n = mth_randomrange(10,100);
    printf("Random 10 to 100 : %d\n",n);

    MTHMAP map;
    for ( map = mth_randominitext(1,10); (n = mth_randomextractor(map,1,10)) != -1 ; printf("Random extractor: %d\n",n) );
    mth_randomfreeext(map);

    char rstr[10];
    mth_randomstr(rstr,10);
    printf("Random string:%s\n",rstr);

    mth_randomstrnum(rstr,10);
    printf("Random string num:%s\n",rstr);

    float f = mth_randomf01();
    printf("Random float 0/1:%f\n",f);

    int i;
    for (i=0; i < 20; i++)
    {
        for (n= (int)mth_randomgauss(40,100); n>0;printf("*"),n--);
        printf("\n");
    }

    printf("moon phase:%lf\n",mth_moonphase(19,11,1980));

    return 0;
}
#endif
