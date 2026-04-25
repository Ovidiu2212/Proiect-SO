#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

int repid;

typedef struct report{
    int id;
    char inspector[20];
    float lat;
    float longit;
    char category[20];
    int severity;
    time_t timestamp;
    char text[50];
}report;

int isDir (const char* fileName)
{
    struct stat st;

    stat(fileName, &st);

    return S_ISDIR(st.st_mode);
}

void increment_repid()
{
    repid++;
    FILE *f;
    f = fopen("id.txt","w");
    fprintf(f,"%d",repid);
    fclose(f);
}

void create_files ( char dirpath[20] )
{
    int fd;
    char filepath[20];

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

void create_dir ( char name[20] )
{
    char dirpath[20];

    mkdir(name,0750);
    
    strcpy(dirpath,name);
    strcat(dirpath,"/");
    create_files(dirpath);
}

void setup_directory ( char name[20] )
{
    struct stat st;
    char dirpath[20];
    strcpy(dirpath,name);
    strcat(dirpath,"/");

    if ( stat(name,&st) == -1 )
    {
        create_dir(name);
    }       
    if ( isDir(name) == 0 )
    {
        printf("File already exists and is not a directory\n");
        exit(-1);
    }
    else
    {
        create_files(dirpath);
    }
}

void print_rep ( report *rep )
{
    printf("Id : %d\n", rep->id);
    printf("Inspector : %s\n",rep->inspector);
    printf("X : %f\n",rep->lat);
    printf("Y: %f\n",rep->longit);
    printf("Issue category : %s\n",rep->category);
    printf("Severity: %d\n",rep->severity);
    printf("Time : %s",ctime(&rep->timestamp));
    printf("Issue description : %s\n",rep->text);
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
    printf("Issue category : ");
    scanf("%20s",rep.category);
    printf("Issue severity : ");
    scanf("%d",&rep.severity);
    time(&rep.timestamp);
    int c;
    while ( ( c = getchar()) != '\n' && c != EOF );
    printf("Issue description :");
    fgets(rep.text,50,stdin);
    rep.text[strlen(rep.text)-1] = '\0';
    write(fd,&rep,sizeof(rep));
    increment_repid();
    close(fd);
}

void list_repfile( char filepath[20], char role[9], char user[20] )
{
    report rep;
    int fd;

    fd = open(filepath,O_RDONLY,0664);
    while ( read(fd,&rep,sizeof(report)) == sizeof(report) )
    {
        print_rep(&rep);
        printf("\n");
    }
    close(fd);
}

void view_rep( char filepath[20], char role[9], char user[20], int id )
{
    report rep;
    int fd;
    int found = 0;

    fd = open(filepath,O_RDONLY,0664);
    while ( read(fd,&rep,sizeof(report)) == sizeof(report) )
    {
        if ( rep.id == id )
        {
            found = 1;
            print_rep(&rep);
        }
    }
    if ( found == 0 )
    {
        printf("Report not found\n");
    }
    close(fd);
}

void remove_rep( char filepath[20], char role[9], char user[20], int id )
{
    report rep;
    int fd;
    int found = 0;
    int size = 0;

    fd = open(filepath,O_RDWR,0664);
    while ( read(fd,&rep,sizeof(report)) == sizeof(report) )
    {
        size++;
        if ( rep.id == id )
        {
            found = 1;
            while ( read(fd,&rep,sizeof(report)) == sizeof(report) )
            {
                size++;
                lseek(fd,(-2)*sizeof(report),SEEK_CUR);
                write(fd,&rep,sizeof(rep));
                lseek(fd,sizeof(report),SEEK_CUR);
            }
        }
    }
    if ( found == 0 )
    {
        printf("Report not found\n");
    }
    else
        ftruncate(fd,(size-1)*sizeof(report));
    close(fd);
}

void update_threshold( char filepath[20], char role[9], char user[20], int value )
{
    int fd;

    fd = open(filepath,O_WRONLY,0640);
    write(fd,&value,sizeof(int));
    close(fd);
}

int parse_condition(const char *input, char *field, char *op, char *value)
{
    if (input == NULL || field == NULL || op == NULL || value == NULL) {
        return -1; // Invalid parameters
    }
    
    // Find the first ':' which separates field from operator
    const char *first_colon = strchr(input, ':');
    if (first_colon == NULL) {
        return -1; // No field separator found
    }
    
    // Extract field
    size_t field_len = first_colon - input;
    if (field_len == 0 || field_len >= 20) { // Assuming 256 is max field length
        return -1; // Invalid field length
    }
    
    strncpy(field, input, field_len);
    field[field_len] = '\0';
    
    // Validate field
    if (strcmp(field, "severity") != 0 && 
        strcmp(field, "category") != 0 && 
        strcmp(field, "inspector") != 0 && 
        strcmp(field, "timestamp") != 0) {
        return -1; // Unsupported field
    }
    
    // Find the second ':' which separates operator from value
    const char *second_colon = strchr(first_colon + 1, ':');
    if (second_colon == NULL) {
        return -1; // No operator-value separator found
    }
    
    // Extract operator
    size_t op_len = second_colon - (first_colon + 1);
    if (op_len == 0 || op_len >= 4) { // Assuming 16 is max operator length
        return -1; // Invalid operator length
    }
    
    strncpy(op, first_colon + 1, op_len);
    op[op_len] = '\0';
    
    // Validate operator
    if (strcmp(op, "==") != 0 && 
        strcmp(op, "!=") != 0 && 
        strcmp(op, "<") != 0 && 
        strcmp(op, "<=") != 0 && 
        strcmp(op, ">") != 0 && 
        strcmp(op, ">=") != 0) {
        return -1; // Unsupported operator
    }
    
    // Extract value (everything after the second colon)
    const char *value_start = second_colon + 1;
    size_t value_len = strlen(value_start);
    
    if (value_len == 0 || value_len >= 20) {
        return -1; // Empty value not allowed
    }
    
    // For timestamp field, validate that value is numeric
    if (strcmp(field, "timestamp") == 0) {
        // Check if all characters are digits (optional negative sign for timestamps)
        for (size_t i = 0; i < value_len; i++) {
            if (!isdigit((unsigned char)value_start[i])) {
                return -1; // Timestamp must be numeric
            }
        }
    }
    
    strcpy(value, value_start);
    
    return 0; // Success
}

int match_condition(report *r, const char *field, const char *op, const char *value) {
    if (r == NULL || field == NULL || op == NULL || value == NULL) {
        return 0; // Invalid parameters, condition fails
    }
    
    // Compare based on field type
    if (strcmp(field, "severity") == 0) {
        int int_value = atoi(value);
        
        if (strcmp(op, "==") == 0) return r->severity == int_value;
        if (strcmp(op, "!=") == 0) return r->severity != int_value;
        if (strcmp(op, "<") == 0)  return r->severity < int_value;
        if (strcmp(op, "<=") == 0) return r->severity <= int_value;
        if (strcmp(op, ">") == 0)  return r->severity > int_value;
        if (strcmp(op, ">=") == 0) return r->severity >= int_value;
    }
    else if (strcmp(field, "timestamp") == 0) {
        time_t time_value = (time_t)atol(value);
        
        if (strcmp(op, "==") == 0) return r->timestamp == time_value;
        if (strcmp(op, "!=") == 0) return r->timestamp != time_value;
        if (strcmp(op, "<") == 0)  return r->timestamp < time_value;
        if (strcmp(op, "<=") == 0) return r->timestamp <= time_value;
        if (strcmp(op, ">") == 0)  return r->timestamp > time_value;
        if (strcmp(op, ">=") == 0) return r->timestamp >= time_value;
    }
    else if (strcmp(field, "category") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->category, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->category, value) != 0;
        
        // For string fields, <, <=, >, >= compare lexicographically
        if (strcmp(op, "<") == 0)  return strcmp(r->category, value) < 0;
        if (strcmp(op, "<=") == 0) return strcmp(r->category, value) <= 0;
        if (strcmp(op, ">") == 0)  return strcmp(r->category, value) > 0;
        if (strcmp(op, ">=") == 0) return strcmp(r->category, value) >= 0;
    }
    else if (strcmp(field, "inspector") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->inspector, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->inspector, value) != 0;
        
        // For string fields, <, <=, >, >= compare lexicographically
        if (strcmp(op, "<") == 0)  return strcmp(r->inspector, value) < 0;
        if (strcmp(op, "<=") == 0) return strcmp(r->inspector, value) <= 0;
        if (strcmp(op, ">") == 0)  return strcmp(r->inspector, value) > 0;
        if (strcmp(op, ">=") == 0) return strcmp(r->inspector, value) >= 0;
    }
    
    return 0; // Unsupported field or operator
}

void filter_repfile( char filepath[20], char role[9], char user[20], char condition[50] )
{
    report rep;
    int fd;
    char field[20],op[4],value[20];

    printf("OK");
    printf("%d",parse_condition(condition,field,op,value));
    fd = open(filepath,O_RDONLY,0664);
    printf("%s:%s:%s",field,op,value);
    while ( read(fd,&rep,sizeof(report)) == sizeof(report) )
    {
        printf("%d",rep.severity);
        if ( match_condition(&rep,field,op,value) == 1 )
        {
            print_rep(&rep);
            printf("\n");
        }
    }
    close(fd);
}

void add_log ( char *filepath, int argc, char **argv)
{
    int fd;
    fd = open(filepath,O_WRONLY|O_APPEND,0644);
    {
        char time_text[51];
        char space = ' ';
        char newline = '\n';
        time_t t;

        time(&t);
        strcpy(time_text,ctime(&t));
        time_text[strlen(time_text)-1] = '\0';
        write(fd,&time_text,strlen(time_text)*sizeof(char));
        write(fd,&space,sizeof(char));
        write(fd,argv[4],strlen(argv[4])*sizeof(char));
        write(fd,&space,sizeof(char));
        write(fd,argv[2],strlen(argv[2])*sizeof(char));
        write(fd,&space,sizeof(char));
        write(fd,argv[5]+2,(strlen(argv[5])-2)*sizeof(char));
        write(fd,&newline,sizeof(char));
    }
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
        printf("Incorect number of arguments");
        exit(-1);
    }

    if( strcmp(argv[1],"--role") != 0 )
    {
        printf("Arguments incorect ( usage : city_manager --role ROLE --user USER --command)");
        exit(-1);
    }
    strcpy(role,argv[2]);

    if( strcmp(argv[3],"--user") != 0 )
    {
        printf("Arguments incorect ( usage : city_manager --role ROLE --user USER --command)");
        exit(-1);
    }
    strcpy(user,argv[4]);

    strcpy(dirpath,argv[6]);
    strcat(dirpath,"/");
    strcat(filepath,dirpath);   

    if ( strcmp(argv[5],"--add") == 0 )
    {
        if ( argc != 7 )
        {
            printf("Incorect number of arguments");
            exit(-1);
        }
        setup_directory(argv[6]);
        strcat(filepath,"reports.dat");
        add_rep(filepath,role,user);
    }
    else if ( strcmp(argv[5],"--list") == 0)
    {
        if ( argc != 7 )
        {
            printf("Incorect number of arguments");
            exit(-1);
        }
        setup_directory(argv[6]);
        strcat(filepath,"reports.dat");
        list_repfile(filepath,role,user);
    }
    else if ( strcmp(argv[5],"--view") == 0)
    {
        if ( argc != 8 )
        {
            printf("Incorect number of arguments");
            exit(-1);
        }
        setup_directory(argv[6]);
        strcat(filepath,"reports.dat");
        view_rep(filepath,role,user,atoi(argv[7]));
    }
    else if ( strcmp(argv[5],"--remove_report") == 0)
    {
        if ( argc != 8 )
        {
            printf("Incorect number of arguments");
            exit(-1);
        }
        setup_directory(argv[6]);
        strcat(filepath,"reports.dat");
        remove_rep(filepath,role,user,atoi(argv[7]));
    }
    else if ( strcmp(argv[5],"--update_threshold") == 0)
    {
        if ( argc != 8 )
        {
            printf("Incorect number of arguments");
            exit(-1);
        }
        setup_directory(argv[6]);
        strcat(filepath,"district.cfg");
        update_threshold(filepath,role,user,atoi(argv[7]));
    }
    else if ( strcmp(argv[5],"--filter") == 0)
    {
        if ( argc != 8 )
        {
            printf("Incorect number of arguments");
            exit(-1);
        }
        setup_directory(argv[6]);
        strcat(filepath,"reports.dat");
        filter_repfile(filepath,role,user,argv[7]);
    }
    else
    {
        printf("Incorrect command");
        exit(-1);
    }

    //Log command
    strcpy(filepath,dirpath);
    strcat(filepath,"logged_district");
    add_log(filepath,argc,argv);
    return 0;
}