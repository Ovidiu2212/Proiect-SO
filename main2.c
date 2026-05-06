#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

void create_pid_file()
{
    int fd;
    fd = open(".monitor_pid",O_CREAT|O_WRONLY,0666);

    pid_t pid;
    pid = getpid();
    write(fd,&pid,sizeof(pid_t));
    close(fd);
}

void delete_pid_file()
{
    pid_t pid;
    pid = fork();
    if ( pid == 0 )
    {
        execlp("rm","rm",".monitor_pid",NULL);
    }
    else if ( pid > 0 )
    {
        if ( wait(NULL) != -1 )
        {
            return;
        }
        else
        {
            printf("Error wait");
        }
    }
}

static void sig_handler ( int signo )
{
    if ( signo == SIGINT )
    {
        printf("Caught signal SIGINT, stopping program\n");
        fflush(stdout);
        delete_pid_file();
        exit(-1);
    }
    if ( signo == SIGUSR1 )
    {
        printf("A new report was added to a district\n");
        fflush(stdout);
    }
}

int main (void)
{
    create_pid_file();

    struct sigaction psa;
    memset (&psa, 0, sizeof (psa));
    psa.sa_handler = sig_handler;
    sigaction (SIGINT, &psa, NULL);
    sigaction (SIGUSR1, &psa, NULL);
    while(1)
    {
        sleep(1);
    }
    return 0;
}