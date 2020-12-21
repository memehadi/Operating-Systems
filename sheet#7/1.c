#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <getopt.h>



int main(int argc, char* argv[] )
{
    int fd[2];
    if(pipe(fd)==-1)
    {
        printf("error while opening the pipe\n");
        return 1;
    }
    int id = fork();
    if (id ==-1)
    {
        printf("error while forking\n");
        return 4 ;
    }
    if(id==0)
    {
        close(fd[0]);
        int x;
        printf("enter a number:");
        scanf("%d",&x);
       if( write(fd[1],&x,sizeof(int))== -1)
       {         printf("error while writing to the pipe\n");
           return 2;
       };
       close(fd[1]);
    }
    else
    {
        close(fd[1]);
        int y;
        if(read(fd[0],&y,sizeof(int))==-1)
        {
            printf("error while reading the file\n");
            return 3;
        };
        y=2*y;
        close(fd[0]);
        printf("%d",y);

    }

    return 0;

}