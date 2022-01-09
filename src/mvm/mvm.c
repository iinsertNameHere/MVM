//
// Created by iinsert on 29.12.2021.
//

#define MVM_SHARED_IMPLEMENTATION
#include "../shared.h"

// Slick VM
MVM mvm = {0};

static void usage(FILE* stream)
{
    fprintf(stream, "Usage: mvm -i <input.mbc> [options]\n");
    fprintf(stream, "  -h          Provides a help list.\n");
    fprintf(stream, "  -l <limit>  Sets a execution limit.\n");
    fprintf(stream, "  -d          Enables step-debug mode.\n");
    fprintf(stream, "  -ds         Enables debug-print-stack mode.\n");
}

int main(int argc, char** argv)
{
    shift(&argc, &argv); // Skip program name.
    char* inputFilePath = NULL;
    int limit = -1;
    int debug = 0;
    int debugPrint = 0;

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
            if (debugPrint) {
                fprintf(stderr, "ERROR: '-ds' can't be used with '-d' enabled!\n");
                usage(stderr);
                exit(1);
            }
            debug = 1;
        } else if (strcmp(flag, "-ds") == 0) {
            if (debug) {
                fprintf(stderr, "ERROR: '-ds' can't be used with '-d' enabled!\n");
                usage(stderr);
                exit(1);
            }
            debugPrint = 1;
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

    mvm_loadProgramFromFile(&mvm, inputFilePath);
    if (!debug) {
        ExeptionState state = mvm_execProgram(&mvm, limit);
        if (state != EXEPTION_STACK_OVERFLOW && debugPrint) {
            mvm_dumpStack(stdout, &mvm);
        } else if (state != EXEPTION_SATE_OK) {
            fprintf(stderr, "ERROR: Failed to execute program! : %s\n", exeption_as_cstr(state));
            return 1;
        }
    } else {
        int step = 0;
        while (limit != 0 && !mvm.halt) {
            ExeptionState err = mvm_execInst(&mvm);
            step++;
            if (mvm.stack_size > MVM_STACK_CAPACITY) {
                fprintf(stderr, "ERROR: Failed to execute program! : %s\n", exeption_as_cstr(EXEPTION_STACK_OVERFLOW));
                exit(1);
            }
            if (err != EXEPTION_SATE_OK) {
                fprintf(stderr, "ERROR: Failed to execute program at '%s'! : %s\n", inst_as_cstr(mvm.program[mvm.ip].type), exeption_as_cstr(err));
                exit(1);
            }
            if (limit > 0) {
                --limit;
            }
            mvm_dumpStack(stdout, &mvm);
            printf("\nDEBUG: Executed Step: [%d]", step);
            getchar();
            printf("\n");
        }
    }
    return 0;
}

