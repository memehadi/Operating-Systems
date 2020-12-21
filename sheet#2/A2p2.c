/*
Musab Mehadi
mmehadi@jacobs-university.de
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define _POSIX_C_SOURCE 200809L

static void
work(const char *msg)
{
    (void) printf("%s ", msg);
    exit(EXIT_SUCCESS);
}

void get_arg(char **argv)
{
    printf("Type in your arguments please:\n");
    char *li = NULL;
    size_t len = 0;
    ssize_t lineSize = 0;
    lineSize = getline(&li, &len, stdin);
    
    //making  lists of arguments
    char * token = strtok(li, " ");
    int i=1;
    while(token != NULL)
    {
        if(li ==NULL) 
        {
            argv[1] = 0;
            break;
        }
        argv[i]= token;
        i++;
        token = strtok(NULL, " ");
    }


    free(li);
}

//getting input(this time with max)
void getargsm(int max, char **argv)
{
    printf("Type in your arguments please:\n");
    char *line = NULL;
    size_t len = 0;
    ssize_t lineSize = 0;
    lineSize = getline(&li, &len, stdin);
    
    //making the arguments list
    char * token = strtok(li, " ");
    int i=1; int n=0;
    while( (token != NULL) && (n<max) )
    {
        if(line ==NULL) 
        {
            argv[1] = 0;
            break;
        }
        argv[i]= token;
        i++; n++;
        token = strtok(NULL, " ");
    }

    free(li);
}

void exec(char **argv)// executing the child
{
    pid_t pid= fork();
    int status;

    if(pid == -1)
    {
        perror("unable to implement fork");
        status = EXIT_FAILURE;
    }
    else if (pid == 0)//if child
    {
        if( execvp(*argv, argv) < 0 )//executing command
        {
            printf(" unable to exec \n");
            status = EXIT_FAILURE;
            
        }
        
    }
    else
    {
        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid() failed");
            status = EXIT_FAILURE;
        }
    }
}

int main(int argc, char *argv[])
{
    int i, stat, status = EXIT_SUCCESS;
    pid_t pids;


    while(1)
    {
        int casenum = getopt(argc, argv, "n:s");
        int max =-1;

        if (casenum == -1)
            break;

        switch(casenum)
        {
            default:
            {
                printf("default case:\n");
                get_arg(argv);           
                exec(argv);   
                break;   

            }
            case 'n': max = atoi(optarg);
            {
                printf("case n:\n");
                getargsm(max, argv);
                exec(argv);

                break;
            }

            case 't':
            {
                printf("case t:\n");
                geta_rg(argv);           
                exec(argv);


                break;
            }
        }


        
    }

    (void) printf("\n");
    return status;
}
