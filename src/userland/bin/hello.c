
#include "stdio.h"

int main(int argc, char **argv)
{
    (void)(argc);
    (void)(argv);
    FILE *stdout = fdopen(1, "w");

    for (unsigned int i = 0; i < 4; i++)
        fprintf(stdout, "Hello, World! (%u)\n", i);
    
    return 0;
}
