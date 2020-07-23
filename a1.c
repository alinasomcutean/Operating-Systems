#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>

#define MAX_PATH_LEN 1024

#define __DEBUG

#ifdef __DEBUG
void debug_info (const char *file, const char *function, const int line)
{
        fprintf(stderr, "DEBUG. ERROR PLACE: File=\"%s\", Function=\"%s\", Line=\"%d\"\n", file, function, line);
}

#define ERR_MSG(DBG_MSG) { \
        perror(DBG_MSG); \
        debug_info(__FILE__, __FUNCTION__, __LINE__); \
}

#else                   // with no __DEBUG just displays the error message

#define ERR_MSG(DBG_MSG) { \
        perror(DBG_MSG); \
}

#endif

void variant(){
    printf("68078\n");
}

char *file_permissions(char *dirName){ ///function that creates a string with the permissions of the file

    char *permissions = malloc(sizeof(char));
    struct stat inode;
    lstat(dirName, &inode);

    if(inode.st_mode & S_IRUSR){
        permissions[0] = 'r';
    }
    else{
        permissions[0] = '-';
    }

    if(inode.st_mode & S_IWUSR){
        permissions[1] = 'w';
    }
    else{
        permissions[1] = '-';
    }

    if(inode.st_mode & S_IXUSR){
        permissions[2] = 'x';
    }
    else{
        permissions[2] = '-';
    }

    if(inode.st_mode & S_IRGRP){
        permissions[3] = 'r';
    }
    else{
        permissions[3] = '-';
    }

    if(inode.st_mode & S_IWGRP){
        permissions[4] = 'w';
    }
    else{
        permissions[4] = '-';
    }

    if(inode.st_mode & S_IXGRP){
        permissions[5] = 'x';
    }
    else{
        permissions[5] = '-';
    }

    if(inode.st_mode & S_IROTH){
        permissions[6] = 'r';
    }
    else{
        permissions[6] = '-';
    }

    if(inode.st_mode & S_IWOTH){
        permissions[7] = 'w';
    }
    else{
        permissions[7] = '-';
    }

    if(inode.st_mode & S_IXOTH){
        permissions[8] = 'x';
    }
    else{
        permissions[8] = '-';
    }

    return permissions;
}

void show_directory_content(char *dirName, int i, int rec, int value, char *permissions){
    DIR *dir;
    struct dirent *dirEntry;
    struct stat inode;
    char name[MAX_PATH_LEN];
    char *p;

    dir = opendir(dirName);
    if(dir == 0){
        ERR_MSG("Error openint directory");
        exit(4);
    }
    else{
        if(i == 0){
            printf("SUCCESS\n");
            i++;
        }
    }

    ///iterate the directory contents
    while((dirEntry = readdir(dir)) != 0){
        if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0){
            /// build the complete path to the element in the directory
            snprintf(name, MAX_PATH_LEN, "%s/%s", dirName, dirEntry->d_name);

            /// get info about the directory's element
            lstat(name, &inode);
            if(S_ISDIR(inode.st_mode)){ /// if it is a directory
                if(rec == 1){ /// if the list should be done recursively
                    show_directory_content(name, i, rec, value, permissions);
                }
            }
            else{
                if(value != -1){  ///if size_greater filter must be applied
                    if(S_ISREG(inode.st_mode)){ /// if it is a regular file
                        if(inode.st_size > value){ ///if the specified condition is applieds
                            printf("%s\n", name);
                        }
                    }
                }
            }

            p = file_permissions(name);
            if(strcmp(permissions, "") != 0){ /// if permission filster is applied
                if(strcmp(permissions, p) == 0){ /// if the file has the same permission as the permission readed
                    printf("%s\n", name);
                }
            }
            else{
                if(value == -1){ /// if no filter is applied, then a simple listing is done
                    printf("%s\n", name);
                }
            }
            free(p);
        }
    }
    closedir(dir);
}

typedef struct types{
    int type, offset, size;
    char name[9];
}types;

void parse(char *dirName){
    int dir;
    dir = open(dirName, O_RDONLY);

     if(dir == -1){
        ERR_MSG("Error opening file");
        exit(4);
    }

    char magic[4] = "\0";
    int headerSize = 0;
    int version = 0;
    int nrOfSections = 0;

    lseek(dir, 0, SEEK_SET);

    read(dir, &magic, 4);
    read(dir, &headerSize, 2);
    read(dir, &version, 1);
    read(dir, &nrOfSections, 1);

    types* t = malloc(nrOfSections * sizeof(types));
    for(int i = 1; i <= nrOfSections; i++){
        read(dir, &t[i-1].name, 9);
        read(dir, &t[i-1].type, 2);
        read(dir, &t[i-1].offset, 4);
        read(dir, &t[i-1].size, 4);
    }

    if(strcmp(magic, "ZYj6") != 0){
        printf("ERROR\nwrong magic\n");
        free(t);
        close(dir);
        exit(4);
    }

    if(version < 39 || version > 128){
        printf("ERROR\nwrong version\n");
        free(t);
        close(dir);
        exit(4);
    }

    if(nrOfSections < 3 || nrOfSections > 19){
        printf("ERROR\nwrong sect_nr\n");
        free(t);
        close(dir);
        exit(4);
    }

    for(int i = 1; i <= nrOfSections; i++){
        if(t[i-1].type != 56 && t[i-1].type != 21){
            printf("ERROR\nwrong sect_types\n");
            free(t);
            close(dir);
            exit(4);
        }
    }

    printf("SUCCESS\n");
    printf("version=%d\n", version);
    printf("nr_sections=%d\n", nrOfSections);
    for(int i = 1; i <= nrOfSections; i++){
        printf("section%d: %s %d %d\n", i, t[i-1].name, t[i-1].type, t[i-1].size);
    }
    free(t);
    close(dir);
}

void extract(char *dirName, int section, int line){
    int dir;
    dir = open(dirName, O_RDONLY);

    if(dir == -1){
        printf("ERROR\n");
        printf("invalid file\n");
        exit(4);
    }
    else{
        printf("SUCCESS\n");
    }

    char magic[4] = "\0";
    int headerSize = 0;
    int version = 0;
    int nrOfSections = 0;
    int nrLines = 1;
    char str;
    types* t;

    lseek(dir, 0, SEEK_SET);

    read(dir, &magic, 4);
    read(dir, &headerSize, 2);
    read(dir, &version, 1);
    read(dir, &nrOfSections, 1);

    t = malloc(nrOfSections * sizeof(types));
    for(int i = 1; i <= nrOfSections; i++){
        read(dir, &t[i-1].name, 9);
        read(dir, &t[i-1].type, 2);
        read(dir, &t[i-1].offset, 4);
        read(dir, &t[i-1].size, 4);
    }

    if(section > nrOfSections || section < 1){ /// if the section number is not correct
        printf("ERROR\n");
        printf("invalid section\n");
        free(t);
        close(dir);
        exit(4);
    }
    else{ /// if the section number is correct, move the cursor before the last character from that section
        lseek(dir, t[section-1].offset + t[section-1].size - 1, SEEK_SET);
    }

    read(dir, &str, 1);

    while(str != '\0' && nrLines != line){ /// count the number of the lines until the section is done or it was count the same number of lines as the specified value
        if(str == 0xA){ /// if the character is 0A in hexa
            lseek(dir, -2, SEEK_CUR);
            read(dir, &str, 1);
            if(str == 0xD){ /// and the character before is 0D in hexa it means that the cursor is at the end of a new line
                nrLines++;
            }
        }
        else{
            lseek(dir, -2, SEEK_CUR);
            read(dir, &str, 1);
        }
    }

    if(nrLines != line){ /// if it was specified a number of line too large
        printf("ERROR\n");
        printf("invalid line\n");
        free(t);
        close(dir);
        exit(4);
    }
    else /// if the line number specified is correct
    {
        while(str != '\0'){ /// until it is on the same line, print each character in the reverse order
            printf("%c", str);
            if(str == 0xA){
                lseek(dir, -2, SEEK_CUR);
                read(dir, &str, 1);
                if(str == 0xD){ /// if the needed line is done return from the function
                    free(t);
                    close(dir);
                    return;
                }
            }
            lseek(dir, -2, SEEK_CUR);
            read(dir, &str, 1);
        }
    }

    printf("\n");
    free(t);
    close(dir);
}

int find_valide_file(char *dirName){
    int dir;
    dir = open(dirName, O_RDONLY);

    if(dir == -1){
        printf("ERROR\n");
        printf("invalid file\n");
        exit(4);
    }

    char magic[4] = "\0";
    int headerSize = 0;
    int version = 0;
    int nrOfSections = 0;
    int nrLines = 1;
    char str;
    types* t;

    lseek(dir, 0, SEEK_SET);

    read(dir, &magic, 4);
    read(dir, &headerSize, 2);
    read(dir, &version, 1);
    read(dir, &nrOfSections, 1);

    t = malloc(nrOfSections * sizeof(types));
    for(int i = 1; i <= nrOfSections; i++){
        read(dir, &t[i-1].name, 9);
        read(dir, &t[i-1].type, 2);
        read(dir, &t[i-1].offset, 4);
        read(dir, &t[i-1].size, 4);
    }

    for(int i = 0; i < nrOfSections; i++){ /// for each section
        nrLines = 1;
        lseek(dir, t[i].offset + t[i].size - 1, SEEK_SET);
        read(dir, &str, 1);
        while(str != '\0'){ /// count the nubmer of lines
            if(str == 0xA){
                lseek(dir, -2, SEEK_CUR);
                    read(dir, &str, 1);
                if(str == 0xD){
                    nrLines++;
                }
            }
            else{
                lseek(dir, -2, SEEK_CUR);
                read(dir, &str, 1);
            }
            if(nrLines >= 14){ /// if a section has more than 14 lines, then the file is valid
                close(dir);
                free(t);
                return 1;
            }
        }
    }

    close(dir);
    free(t);
    return 0; /// the file is not valid
}

void findall(char *dirName, int i){
    DIR *dir;
    struct dirent *dirEntry;
    struct stat inode;
    char name[MAX_PATH_LEN];

    dir = opendir(dirName);
    if(dir == 0){
        printf("ERROR\n");
        printf("invalid directory path\n");
        exit(4);
    }
    else{
        if(i == 0){
            printf("SUCCESS\n");
            i = 1;
        }
    }

    ///iterate the directory contents
    while((dirEntry = readdir(dir)) != 0){
        if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0){
            /// build the complete path to the element in the directory
            snprintf(name, MAX_PATH_LEN, "%s/%s", dirName, dirEntry->d_name);

            /// get info about the directory's element
            lstat(name, &inode);

            if(S_ISDIR(inode.st_mode)){ /// if it is a directory
                findall(name, i);
            }
            if(find_valide_file(name) == 1){ /// if the file is valid, print it
                printf("%s\n", name);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char **argv){
    if(argc >= 2){
        if(strcmp(argv[1], "variant") == 0){
            variant();
        }
        if(strcmp(argv[1], "list") == 0){
            int ok = 0;
            char str[MAX_PATH_LEN];
            int value = -1;
            char *permissions;
            permissions = malloc(sizeof(char));
            for(int i = 2; i < argc; i++){
                if(strcmp(argv[i], "recursive") == 0){
                    ok = 1;
                }
                if(strstr(argv[i], "permissions=") != NULL){
                    sscanf(argv[i], "permissions=%s", permissions);
                }
                if(strstr(argv[i], "size_greater=") != NULL){
                    sscanf(argv[i], "size_greater=%d", &value);
                }
                if(strstr(argv[i], "path=") != NULL){
                    sscanf(argv[i], "path=%s", str);
                }
            }
            show_directory_content(str, 0, ok, value, permissions);
            free(permissions);
        }
        if(strcmp(argv[1], "parse") == 0){
            char str[MAX_PATH_LEN];
            if(strstr(argv[2], "path=") != NULL){
                sscanf(argv[2], "path=%s", str);
            }
            parse(str);
        }
        if(strcmp(argv[1], "extract") == 0){
            char str[MAX_PATH_LEN];
            int section;
            int line;
            for(int i = 2; i < argc; i++){
                if(strstr(argv[i], "path=") != NULL){
                    sscanf(argv[i], "path=%s", str);
                }
                if(strstr(argv[i], "section=") != NULL){
                    sscanf(argv[i], "section=%d", &section);
                }
                if(strstr(argv[i], "line=") != NULL){
                    sscanf(argv[i], "line=%d", &line);
                }
            }
            extract(str, section, line);
        }
        if(strcmp(argv[1], "findall") == 0){
            char str[MAX_PATH_LEN];
            if(strstr(argv[2], "path=") != NULL){
                sscanf(argv[2], "path=%s", str);
            }
            findall(str, 0);
        }
    }
    return 0;
}
