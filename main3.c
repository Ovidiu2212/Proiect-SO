#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void hub_mon()
{
    int pfd[2];
    if ( pipe(pfd) == -1 )
    {
        printf("Error Pipe\n");
        exit(-1);
    }

    pid_t pid;
    pid = fork();
    if ( pid == 0 ) 
    {
        close(pfd[0]);
        dup2(pfd[1],1);
        close(pfd[1]);
        execlp("./monitor_reports","",NULL);
    }
    else if ( pid > 0 )
    {
        close(pfd[1]);

        char buffer[100];
        buffer[0] = 0;
        while ( read(pfd[0],&buffer,100*sizeof(char)) >= 0 )
        {
            if ( strlen(buffer) > 0 )
            {
                printf("%s",buffer);
                fflush(stdout);
                buffer[0] = 0;
            }
            sleep(1);
        }
        close(pfd[0]);    
        return;
    }
}
int main(void)
{
    hub_mon();
    return 0;
}