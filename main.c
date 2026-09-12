#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_LEN 128

typedef struct arguments{
    unsigned int file_count;
    char** filenames;
} arguments;

typedef struct mode{
    int global_mode;
    int newlines;
    int tabs; 
};
		
struct mode mode;

int read_stdin(){
    int exit_code = 0;
    int buf_cap = MAX_LEN;
    char* buf = malloc(buf_cap *sizeof(char));
    if (buf == NULL){
        perror("ERROR - malloc failed\n");
        exit(1);
    }

    while(fgets(buf, buf_cap, stdin)){
        exit_code = fputs(buf, stdout);
    }

    return exit_code;
}


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
    mode.newlines = 0;
    mode.tabs = 0;

    for(int i=1; i < argc; i++){
        if ((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0)){
            printf("Concatenate FILE(s), or standard input, to standard output\nWith no FILE, or when FILE is -, read standard input.\n\nUsage: cat [OPTION]... [FILE]...\n\nOptions:\n  -A, --show-all          equivalent to -vET\n  -b, --number-nonblank   number nonempty output lines, overrides -n\n  -e                      equivalent to -vE\n  -E, --show-ends         display $ at end of each line\n  -n, --number            number all output lines\n  -s, --squeeze-blank     suppress repeated empty output lines\n  -t                      equivalent to -vT\n  -T, --show-tabs         display TAB characters at ^I\n  -v, --show-nonprinting  use ^ and M- notation, except for LF (\\n) and TAB (\\t)\n  -u                      (ignored)\n  -h, --help              Print help  -V, --version           Print version\n");
            exit(0);
        } else if ((strcmp(argv[i], "--version") == 0) || (strcmp(argv[i], "-V") == 0)){
            printf("Version 0.1 by Olaf :D\n");
            exit(0);
        } else if ((strcmp(argv[i], "-E") == 0)){
            mode.newlines = 1;
        } else if ((strcmp(argv[i], "-T") == 0)){
            mode.tabs = 1;
        } else {
            args->filenames[index] = argv[i];
            index++;
        }
    }
    if (index > 0){
        args->file_count = index;
        mode.global_mode = 0;
    } else {
        mode.global_mode = 1;
    }
}

int write_buffer(char* buffer, int size){
    // if any one of the modes are active, use the character-by-character writing.
    if (mode.newlines + mode.tabs >= 1){
        for (int i = 0; i < size; i++){
            char act = *(buffer + i);
            switch (act){
                case '\0':
                    exit(0);
                    break;
                case '\t':
                    if (mode.tabs == 1){ fputs("^I", stdout);}
                    break;
                case '\n':
                    if (mode.newlines == 1){ fputs("$\n", stdout);}
                    break;
                default:
                    fputc(act, stdout);
            }
    }
    } else {
        fwrite(buffer, sizeof(char), size, stdout);
    }
}
int main(int argc, char** argv){

    int debug = 0;
    arguments args = {0};
    parse_args(argc, argv, &args);
    
    char* buffer = NULL;
    int buf_size = 0;

    // file mode
    if (mode.global_mode == 0){
    
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
    } else if (mode.global_mode == 1){
        read_stdin();
    }
    

    return 0;
}
