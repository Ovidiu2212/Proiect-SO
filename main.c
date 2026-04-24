#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

int repid;

typedef struct report{
    int id;
    char inspector[20];
    float lat;
    float longit;
    char issue[20];
    int severity;
    time_t timestamp;
    char text[50];
}report;

int isDir(const char* fileName)
{
    struct stat path;

    stat(fileName, &path);

    return S_ISDIR(path.st_mode);
}

void increment_repid()
{
    repid++;
    FILE *f;
    f = fopen("id.txt","w");
    fprintf(f,"%d",repid);
    fclose(f);
}

void create_dir( char name[20] )
{
    int fd;
    char dirpath[20];
    char filepath[20];

    mkdir(name,0750);
    
    strcpy(dirpath,name);
    strcat(dirpath,"/");

    mkdir(name,0750);

    strcpy(filepath,dirpath);
    strcat(filepath,"reports.dat");
    fd = open(filepath,O_CREAT|O_WRONLY,0664);
    chmod(filepath,0664);
    close(fd);

    strcpy(filepath,dirpath);
    strcat(filepath,"district.cfg");
    fd = open(filepath,O_CREAT|O_WRONLY,0640);
    chmod(filepath,0640);
    close(fd);

    strcpy(filepath,dirpath);
    strcat(filepath,"logged_district");
    fd = open(filepath,O_CREAT|O_WRONLY,0644);
    chmod(filepath,0644);
    close(fd);
}

void print_rep ( report *rep )
{
    printf("id : %d\n", rep->id);
    printf("inspector : %s\n",rep->inspector);
    printf("x : %f\n",rep->lat);
    printf("y: %f\n",rep->longit);
    printf("issue : %s\n",rep->issue);
    printf("severity: %d\n",rep->severity);
    printf("text : %s\n",rep->text);
}

void add_rep( char filepath[20], char role[9], char user[20] )
{
    report rep;
    int fd;

    fd = open(filepath,O_WRONLY|O_APPEND,0664);
    rep.id = repid;
    strcpy(rep.inspector,user);
    printf("Latitude : ");
    scanf("%f",&rep.lat);
    printf("Longitude : ");
    scanf("%f",&rep.longit);
    printf("Issue : ");
    scanf("%20s",rep.issue);
    printf("Issue severity : ");
    scanf("%d\n",&rep.severity);
    time(&rep.timestamp);
    printf("Issue description : ");
    fflush(stdout);
    fgets(rep.text,50,stdin);
    rep.text[strlen(rep.text)-1] = '\0';
    write(fd,&rep,sizeof(rep));
    close(fd);
}

int main ( int argc, char **argv )
{   
    char role[9],user[20];
    char dirpath[20];
    char filepath[20];
    FILE *f;

    f = fopen("id.txt","r");
    fscanf(f,"%d",&repid);
    fclose(f);

    if ( argc < 6 )
    {
        printf("Numar prea mic de argumente");
        exit(-1);
    }

    if( strcmp(argv[1],"--role") != 0 )
    {
        printf("Argument incorect ( usage : city_manager --role ROLE --user USER --command)");
        exit(-1);
    }
    strcpy(role,argv[2]);

    if( strcmp(argv[3],"--user") != 0 )
    {
        printf("Argument incorect ( usage : city_manager --role ROLE --user USER --command)");
        exit(-1);
    }
    strcpy(user,argv[4]);

    struct stat st;
    if ( stat(argv[6],&st) == -1 )
    {
        create_dir(argv[6]);
    }       
    if ( isDir(argv[6]) == 0 )
    {
        printf("File already exists and is not a directory\n");
        exit(-1);
    }

    strcpy(dirpath,argv[6]);
    strcat(dirpath,"/");
    strcpy(filepath,dirpath);
    strcat(filepath,"reports.dat");
    add_rep(filepath,role,user);

    return 0;
}