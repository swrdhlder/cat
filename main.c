#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_LEN 128

typedef struct arguments{
    unsigned int file_count;
    char** filenames;
} arguments;

int read_file(char* filename,  char** buffer){
    int buf_cap = MAX_LEN;
    char* buf = malloc(buf_cap * sizeof(char));
    if (buf == NULL){
        perror("ERROR - malloc failed\n");
        exit(1);
    }
    int buf_size = 0;
    int size = 0;

    FILE *f = fopen(filename, "r");
    if (f == NULL){
        perror("ERROR - fopen failed\n");
        exit(1);
    }
    do {
        if (buf_size + MAX_LEN >= buf_cap){
            buf_cap *= 2;
            buf = realloc(buf, buf_cap * sizeof(char));
            if (buf == NULL){
                perror("ERROR - realloc failed\n");
                exit(1);
            }
        }
        size = fread(buf + buf_size, sizeof(char), MAX_LEN, f);
        if (size == -1){
            perror("ERROR - fread failed");
        }
        buf_size += size;
    } while (size > 0);

    fclose(f);

    buf[buf_size] = '\0';

    *buffer = buf;

    return buf_size;
    
}

void parse_args(int argc, char** argv, arguments *args){
    
    args->filenames = malloc(argc * sizeof(char*));
    int index = 0;

    for(int i=1; i < argc; i++){
        if (strcmp(argv[i], "--help") == 0){
            printf("This is the help page. Just put any filename (or filenames!) and I'll happily concatenate them.\n");
            exit(0);
        } else {
            args->filenames[index] = argv[i];
            index++;
        }
    }

    args->file_count = index;
}

int write_buffer(char* buffer, int size){
    fwrite(buffer, sizeof(char), size, stdout);
}

int main(int argc, char** argv){

    int debug = 0;
    arguments args = {0};
    parse_args(argc, argv, &args);
    
    char* buffer = NULL;
    int buf_size = 0;
     
    for (int i=0; i < args.file_count; i++){
        
        char* content = NULL;
        int content_size = 0;
        content_size = read_file(args.filenames[i], &content);

        buffer = realloc(buffer, buf_size + content_size + 1);
        memcpy(buffer+buf_size, content, content_size);
        buf_size += content_size;

        free(content);
    }

    buffer[buf_size] = '\0';
    write_buffer(buffer, buf_size);
    free(buffer);
    free(args.filenames);


    return 0;
}