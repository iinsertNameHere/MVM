//
// Created by iinsert on 29.12.2021.
//

#define SVM_SHARED_IMPLEMENTATION
#include "../shared.h"

// Slick VM
SVM svm = {0};

static void usage(FILE* stream)
{
    fprintf(stream, "Usage: svm -i <input.sbc> [options]\n");
    fprintf(stream, "  -h          Provides a help list.\n");
    fprintf(stream, "  -l <limit>  Sets a execution limit.\n");
    fprintf(stream, "  -d          Enables debug mode.\n");
}

int main(int argc, char** argv)
{
    shift(&argc, &argv); // Skip program name.
    char* inputFilePath = NULL;
    int limit = -1;
    int debug = 0;

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
        } else if (strcmp(flag, "-d") == 0) {
            debug = 1;
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
    if (!debug) {
        ExeptionState state = svm_execProgram(&svm, limit);
        if (state != EXEPTION_STACK_OVERFLOW) {
            svm_dumpStack(stdout, &svm);
        } else if (state != EXEPTION_SATE_OK) {
            fprintf(stderr, "ERROR: Failed to execute program! : %s\n", exeption_as_cstr(state));
            return 1;
        }
    } else {
        int step = 0;
        while (limit != 0 && !svm.halt) {
            ExeptionState err = svm_execInst(&svm);
            step++;
            if (svm.stack_size > SVM_STACK_CAPACITY) {
                fprintf(stderr, "ERROR: Failed to execute program! : %s\n", exeption_as_cstr(EXEPTION_STACK_OVERFLOW));
            }
            if (err != EXEPTION_SATE_OK) {
                fprintf(stderr, "ERROR: Failed to execute program at '%s'! : %s\n", inst_as_cstr(svm.program[svm.ip].type), exeption_as_cstr(err));
            }
            if (limit > 0) {
                --limit;
            }
            svm_dumpStack(stdout, &svm);
            printf("\nDEBUG: Executed Step: [%d]", step);
            getchar();
            printf("\n");
        }
    }
    return 0;
}

