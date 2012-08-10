#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#define READER 0
#define WRITER 1

typedef struct _array_t
{
    int id;
    int data;
}array_t;

int main (int argc, char *argv[])
{
    int fd[2];

    int r = pipe( fd );
    if ( r != 0 )
    {
        perror( "pipe()" );
        exit( 1 );
    }

    if ( fork() )
    {
        /* Parent, writer */
        int n = 0;
        unsigned int value = 0;
        array_t array;
        close( fd[READER] );
        while ( 1 )
        {
            array.id   = n;
            value = value * 33 + n;
            array.data = value;
            //if  (array.data >= 2147483648)
            //{
            //    value = 0;
            //}
            write( fd[WRITER], &array, sizeof(array_t) );
            ++n;
            sleep( 1 );
        }
    }
    else
    {
        /* Child, reader */
        array_t array;
        close( fd[WRITER] );
        while ( 1 )
        {
            read( fd[READER], &array, sizeof(array_t) );
            printf("Got value: array.id = %d\n", array.id);
            printf("           array.data = %d\n", array.data);
        }
    }

    return 0;
}
