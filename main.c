#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_LEN 128

typedef struct arguments{
    unsigned int file_count;
    char** filenames;
} arguments;

typedef struct mode{
    int global_printmode;
    int newlines;
    int tabs; 
    int numbers;
    int lines;
    int nonempty_numbers;
}mode;
		
struct mode printmode;

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

void count_newlines(char *buffer, int buffer_size){
    int newlines = 0;
    for (int i=0; i<buffer_size; i++){
        if (buffer[i] == '\n'){
            newlines++;
        }
    }
    printmode.lines = newlines;
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
    count_newlines(buf, buf_size);
    return buf_size;
}

void parse_args(int argc, char** argv, arguments *args){
    args->filenames = malloc(argc * sizeof(char*));
    int index = 0;
    printmode.newlines = 0;
    printmode.tabs = 0;
    printmode.numbers = 0;
    printmode.nonempty_numbers = 0;


    for(int i=1; i < argc; i++){
        if ((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0)){
            printf("Concatenate FILE(s), or standard input, to standard output\nWith no FILE, or when FILE is -, read standard input.\n\nUsage: cat [OPTION]... [FILE]...\n\nOptions:\n  -A, --show-all          equivalent to -vET\n  -b, --number-nonblank   number nonempty output lines, overrides -n\n  -e                      equivalent to -vE\n  -E, --show-ends         display $ at end of each line\n  -n, --number            number all output lines\n  -s, --squeeze-blank     suppress repeated empty output lines\n  -t                      equivalent to -vT\n  -T, --show-tabs         display TAB characters at ^I\n  -v, --show-nonprinting  use ^ and M- notation, except for LF (\\n) and TAB (\\t)\n  -u                      (ignored)\n  -h, --help              Print help  -V, --version           Print version\n");
            exit(0);
        } else if ((strcmp(argv[i], "--version") == 0) || (strcmp(argv[i], "-V") == 0)){
            printf("Version 0.1 by Olaf :D\n");
            exit(0);
        } else if ((strcmp(argv[i], "-E") == 0)){
            printmode.newlines = 1;
        } else if ((strcmp(argv[i], "-T") == 0)){
            printmode.tabs = 1;
        } else if ((strcmp(argv[i], "-n") == 0)){
            printmode.numbers = 1;
        } else if ((strcmp(argv[i], "-b") == 0)){
            printmode.nonempty_numbers = 1;
        } else if ((strcmp(argv[i], "-A") == 0)){
            // Activate all non-readable character printing
            printmode.newlines = 1;
            printmode.tabs = 1;
        } else {
            // multiple parameter per dash handling
            if (argv[i][0] == '-'){
                for (int j = 0; j< strlen(argv[i]); j++){
                    char arg = argv[i][j];
                    switch (arg){
                        case 'E':
                            printmode.newlines = 1;
                            break;
                        case 'T':
                            printmode.tabs = 1;
                            break;
                        case 'n':
                            printmode.numbers = 1;
                            break;
                        case 'b':
                            printmode.nonempty_numbers = 1;
                            break;
                        case 'A':
                            printmode.newlines = 1;
                            printmode.tabs = 1;
                            break;
                        default:
                            printf("ERROR - invalid arguments\n");
                            exit(3);
                    }
                }
            } else {
                args->filenames[index] = argv[i];
                index++;
            }
        } 
    }
    if (index > 0){
        args->file_count = index;
        printmode.global_printmode = 0;
    } else {
        printmode.global_printmode = 1;
    }
}

int count_digits_newlines(){
    return log10(printmode.lines)+2;
}

void print_line_numbers(int* ln, char* l_buffer, int dig){
    float digs = log10(*ln);
    int spaces_before = 2 + (dig - digs);
    if ( (int) digs == digs){
        spaces_before--;
    }
    for (int i = 0; i<spaces_before; i++){
        fputs(" ", stdout);
    }
    sprintf(l_buffer, "%d  ", *ln);
    fputs(l_buffer, stdout);
    (*ln)++;
}

int write_buffer(char* buffer, int size){
    // numbering of the lines is done using an index starting with 1
    unsigned int line_number = 1;
    int digits_newlines = count_digits_newlines();
    char ln_buffer[digits_newlines+1];

    if (printmode.numbers){
        print_line_numbers(&line_number, ln_buffer, digits_newlines);    
    } else if (printmode.nonempty_numbers){
        if (buffer[0] != '\n'){
            print_line_numbers(&line_number, ln_buffer, digits_newlines);
        }
    }

    // if any one of the printmodes are active, use the character-by-character writing.

    if (printmode.newlines || printmode.tabs || printmode.newlines || printmode.nonempty_numbers){

        if (printmode.numbers && printmode.nonempty_numbers){
            // Setting precedence to the non-empty numbers
            printmode.numbers = 0;
        }

        for (int i = 0; i < size; i++){
            char act = *(buffer + i);
            switch (act){
                case '\0':
                    exit(0);
                    break;
                case '\t':
                    if (printmode.tabs){ fputs("^I", stdout);}
                    break;
                case '\n':
                    if (printmode.newlines){ fputs("$\n", stdout);}
                    if (printmode.numbers){
                        print_line_numbers(&line_number, ln_buffer, digits_newlines); 
                    } else if (printmode.nonempty_numbers){
                        char next = *(buffer + i + 1);
                        if (next != '\n' && next != '\0'){
                            print_line_numbers(&line_number, ln_buffer, digits_newlines);

                        }
                    }
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

    // file printmode
    if (printmode.global_printmode == 0){
    
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
    } else if (printmode.global_printmode == 1){
        read_stdin();
    }
    

    return 0;
}
