#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <stdbool.h>

int request;
int response;
int shmId;
char* attachMem = NULL;
char* data = NULL;
int fdPath;
off_t fileSize;

typedef struct types{
    int type, offset, size;
    char name[9];
}types;

/// Function which creates the connection with the tester
void start(){
    /// Create the pipe for writing
    int create = mkfifo("RESP_PIPE_68078", 0600);

    if(create < 0){
        printf("ERROR\ncannot create the response pipe\n");
        exit(1);
    }

    /// Open the pipe for reading
    request = open("REQ_PIPE_68078", O_RDWR);
    if(request < 0){
        printf("ERROR\ncannot open the request pipe\n");
        exit(1);
    }

    /// Open the pipe for writing
    response = open("RESP_PIPE_68078", O_WRONLY);
    unsigned int size = 7;
    write(response, &size, 1);

    int w = write(response, "CONNECT", 7);

    if(w > 0){
        printf("SUCCESS\n");
    }
}

/// Function which send to the response pipe the desired message
void requestPing(){
    unsigned int size = 4;

    write(response, &size, 1);
    write(response, "PING", size);
    write(response, &size, 1);
    write(response, "PONG", size);

    unsigned int number = 0x109EE; /// Number 68078 in hexa
    write(response, &number, sizeof(unsigned int));
}

/// Function which creates a shared memory
void createSharedMemory(){
    unsigned int* number = malloc(sizeof(unsigned int));
    read(request, number, sizeof(unsigned int));
    unsigned int size;

    /// Creates the shared memory
    shmId = shmget(19036, 3583577, IPC_CREAT | 0664);

    if(shmId > 0){
        /// Attach the shared memory to an addres in the real memory
        attachMem = (char*)shmat(shmId, NULL, 0);
        size = 10;
        write(response, &size, 1);
        write(response, "CREATE_SHM", size);
        if(attachMem == (void*)-1){
            size = 5;
            write(response, &size, 1);
            write(response, "ERROR", size);
        }
        else{
            size = 7;
            write(response, &size, 1);
            write(response, "SUCCESS", size);
        }
    }
}

/// Function which writes in the shared memory created
void writeSharedMemory(){
    /// Read the offset where I have to write and the value which I have to write
    unsigned int* offset = malloc(sizeof(unsigned int));
    read(request, offset, sizeof(unsigned int));
    unsigned int* value = malloc(sizeof(unsigned int));
    read(request, value, sizeof(unsigned int));

    /// Check if the offset is not in the shared memory space
    if((*offset) >= 0 && (*offset) <= 3583577){
        /// Chech if all the bytes which I have to write corresponds to an offset of the shared memory
        if((*offset) + 4 <= 3583577 && (*offset) + 4 >= 0){
            /// Write in the memory byte by byte
            char number[4];
            memcpy(number, value, 4);
            attachMem[*offset] = number[0];
            attachMem[*offset+1] = number[1];
            attachMem[*offset+2] = number[2];
            attachMem[*offset+3] = number[3];
            int size1 = 12;
            int size2 = 7;
            write(response, &size1, 1);
            write(response, "WRITE_TO_SHM", size1);
            write(response, &size2, 1);
            write(response, "SUCCESS", size2);
        }
        else{
            int size2 = 12;
            write(response, &size2, 1);
            write(response, "WRITE_TO_SHM", size2);
            int size1 = 5;
            write(response, &size1, 1);
            write(response, "ERROR", size1);
        }
    }
    else{
        int size2 = 12;
        write(response, &size2, 1);
        write(response, "WRITE_TO_SHM", size2);
        int size1 = 5;
        write(response, &size1, 1);
        write(response, "ERROR", size1);
    }
}

/// Function which maps the file into memory
void mapFile(){
    /// Read the dimension of the path and the path to the file
    unsigned int* dim = malloc(sizeof(unsigned int));
    read(request, dim, 1);
    char* path = malloc(sizeof(char)*(*dim));
    read(request, path, *dim);
    path[*dim] = '\0';

    /// Open the file
    fdPath = open(path, O_RDONLY);
    /// Move the cursor to the end
    fileSize = lseek(fdPath, 0, SEEK_END);
    data = (char*)mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fdPath, 0);

    if(data == (void*)-1){
        int s = 8;
        write(response, &s, 1);
        write(response, "MAP_FILE", s);
        s = 5;
        write(response, &s, 1);
        write(response, "ERROR", s);
        ///close(fdPath);
    }
    else{
        int s = 8;
        write(response, &s, 1);
        write(response, "MAP_FILE", s);
        s = 7;
        write(response, &s, 1);
        write(response, "SUCCESS", s);
    }
}

/// Function which reads from a specific offset from the memory
void readOffset(){
    /// Read the offset where I want to read and the no of bytes which I want to read
    unsigned int* offset = malloc(sizeof(unsigned int));
    read(request, offset, sizeof(unsigned int));
    unsigned int* noOfBytes = malloc(sizeof(unsigned int));
    read(request, noOfBytes, sizeof(unsigned int));

    char* r = malloc(sizeof(char));
    int size1 = 21;
    int err = 5;
    int success = 7;

    write(response, &size1, 1);
    write(response, "READ_FROM_FILE_OFFSET", size1);

    /// Check if the shared memory exists
    if(shmId > 0){
        /// Check if the file is mapped in the memory
        if(data != (void*) -1){
            /// Check if the offset read and the no of bytes which I have to read is smaller than the total size of the file
            if((*offset) + (*noOfBytes) < fileSize){
                /// Read every byte in an auxiliary variable and write it back at the begining of the memory
                for(int i = 0; i < *noOfBytes; i++){
                    r[i] = data[*offset + i];
                    attachMem[i] = r[i];
                }
                write(response, &success, 1);
                write(response, "SUCCESS", success);
            }
            else{
                write(response, &err, 1);
                write(response, "ERROR", err);
            }
        }
        else{
            write(response, &err, 1);
            write(response, "ERROR", err);
        }
    }
    else{
        write(response, &err, 1);
        write(response, "ERROR", err);
    }
}

void readFromSection(){
    unsigned int* sectionNo = malloc(sizeof(unsigned int));
    unsigned int* offset = malloc(sizeof(unsigned int));
    unsigned int* noOfBytes = malloc(sizeof(unsigned int));

    read(request, sectionNo, sizeof(unsigned int));
    read(request, offset, sizeof(unsigned int));
    read(request, noOfBytes, sizeof(unsigned int));

    int size = 22;
    int err = 5;
    int success = 7;
    write(response, &size, 1);
    write(response, "READ_FROM_FILE_SECTION", size);

    int noOfSections = data[7];
    types* t = malloc(noOfSections * sizeof(types));

    int a = 8;
    int sect = *sectionNo;
    for(int i = 1; i <= noOfSections; i++){
        memcpy(&t[i-1].name, data + a, 9);
        a += 9;
        memcpy(&t[i-1].type, data + a, 2);
        a += 2;
        memcpy(&t[i-1].offset, data + a, 4);
        a += 4;
        memcpy(&t[i-1].size, data + a, 4);
        a += 4;
    }
    if(sect <= noOfSections){
        ///lseek(*data, t[*sectionNo].offset+(*off
        char* r = malloc(sizeof(char));
        /**for(int i = 0; i < *noOfBytes; i++){
            r[i] = data[t[sect].offset + *offset + i];
            attachMem[i] = r[i];
        }*/

        if((*offset) + (*noOfBytes) <= t[sect-1].size){
            /**for(int i = 0; i < (*noOfBytes); i++){
                r[i] = data[(*offset) + i];
                attachMem[i] = r[i];
            }*/
            memcpy(r, data + *offset, *noOfBytes);
            memcpy(attachMem, r, *noOfBytes);
            write(response, &success, 1);
            write(response, "SUCCESS", success);
        }
        else{
            write(response, &err, 1);
            write(response, "ERROR", err);
        }

    }
    else{
        write(response, &err, 1);
        write(response, "ERROR", err);
    }
}

void readFromLogicalSpace(){
    unsigned int* logicalOffset = malloc(sizeof(unsigned int));
    unsigned int* noOfBytes = malloc(sizeof(unsigned int));

    read(request, logicalOffset, sizeof(unsigned int));
    read(request, noOfBytes, sizeof(unsigned int));

    int size = 30;
    write(response, &size, 1);
    write(response, "READ_FROM_LOGICAL_SPACE_OFFSET", size);

    int position = 7;
    int noOfSections = data[position];
    int section = 1;
    int alignment = 0;

    while(section <= noOfSections){
        position += 15;
        int sectionSize = 0;
        lseek(*data, position, SEEK_SET);
        memcpy(&sectionSize, data, 4);
        ///char* r = malloc(sizeof(char));
        for(int i = 0; i < sectionSize; i++){
            attachMem[alignment] = data[position];
            alignment++;
            position++;
        }
        section++;
    }
}

int main(int argc, char **argv){
    start();
    unsigned char* dim = malloc(sizeof(unsigned char));
    read(request, dim, 1);
    char* buf = malloc(sizeof(char)*((*dim)+1));
    read(request, buf, *dim);
    buf[*dim] = '\0';
    while(strcmp(buf, "EXIT") != 0){
        if(strcmp(buf, "PING") == 0){
            requestPing();
        }
        if(strcmp(buf, "CREATE_SHM") == 0){
            createSharedMemory();
        }
        if(strcmp(buf, "WRITE_TO_SHM") == 0){
            writeSharedMemory();
        }
        if(strcmp(buf, "MAP_FILE") == 0){
            mapFile();
        }
        if(strcmp(buf, "READ_FROM_FILE_OFFSET") == 0){
            readOffset();
        }
        if(strcmp(buf, "READ_FROM_FILE_SECTION") == 0){
            readFromSection();
            ///exit(0);
        }
        if(strcmp(buf, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0){
            readFromLogicalSpace();
            exit(0);
        }
        free(buf);
        dim = realloc(dim, sizeof(unsigned char));
        read(request, dim, 1);
        buf = malloc(sizeof(char)*((*dim)+1));
        read(request, buf, *dim);
        buf[*dim] = '\0';
    }
    if(strcmp(buf, "EXIT") == 0){
        unlink("/home/alina/Desktop/Assign_3/RESP_PIPE_68078");
        free(buf);
        close(fdPath);
    }
    return 0;
}
