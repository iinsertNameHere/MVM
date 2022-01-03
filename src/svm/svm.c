//
// Created by iinsert on 29.12.2021.
//

#define SVM_IMPLEMENTATION
#include "../shared.h"

// Slick VM
SVM svm = {0};

//NOTE: GitHub https://github.com/iinsertNameHere/Slick

static void usage(FILE* stream)
{
    fprintf(stream, "Usage: svm -i <input.sbc> [options]\n");
    fprintf(stream, "  -h          Provides a help list.\n");
    fprintf(stream, "  -l <limit>  Sets a execution limit.\n");
}

int main(int argc, char** argv)
{
    shift(&argc, &argv); // Skip program name.
    char* inputFilePath = NULL;
    int limit = -1;

    while (argc > 0) {
        const char* flag = shift(&argc, &argv);
        if (strcmp(flag, "-i") == 0) {
            if (argc == 0) {
                fprintf(stderr, "ERROR: No argument is provided for flag '%s'\n", flag);
                usage(stderr);
                exit(1);
            }
            inputFilePath = shift(&argc, &argv);
        } else if (strcmp(flag, "-l") == 0) {
            if (argc == 0) {
                fprintf(stderr, "ERROR: No argument is provided for flag '%s'\n", flag);
                usage(stderr);
                exit(1);
            }
            limit = atoi(shift(&argc, &argv));
        } else if (strcmp(flag, "-h") == 0) {
            usage(stdout);
            exit(0);
        } else {
            fprintf(stderr, "ERROR: Unknown flag '%s'\n", flag);
            usage(stderr);
            exit(1);
        }
    }

    if (inputFilePath == NULL) {
        fprintf(stderr, "ERROR: Expected input file!\n");
        usage(stderr);
        exit(1);
    }

    svm_loadProgramFromFile(&svm, inputFilePath);
    ExeptionState state = svm_execProgram(&svm, limit);
    if (state != EXEPTION_STACK_OVERFLOW) {
        svm_dumpStack(stdout, &svm);
    } else if (state != EXEPTION_SATE_OK) {
        fprintf(stderr, "ERROR: Failed to execute program! : %s\n", exeption_as_cstr(state));
        return 1;
    }
    return 0;
}

