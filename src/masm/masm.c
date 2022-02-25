//
// Created by iinsert on 29.12.2021.
//

#define MVM_SHARED_IMPLEMENTATION
#include "../shared.h"

Masm masm = {0};

static void usage(FILE* stream)
{
    fprintf(stream, "Usage: masm -i <input.msm> -o <output.mbc> [options]\n");
    fprintf(stream, "  -h          Provides a help list.\n");
    fprintf(stream, "  -d          Enables step-debug mode.\n");
    fprintf(stream, "  -c          Enables Compatibility Warnings.\n");
}

int main(int argc, char** argv)
{
    shift(&argc, &argv); // Skip program name.
    const char* inputFilePath = NULL;
    const char* outputFilePath = NULL;
    int debug = 0;
    int error = 0;
    bool wos = false;
    const char* errorFlag = NULL;

    while (argc > 0) {
        const char* flag = shift(&argc, &argv);
        if (strcmp(flag, "-i") == 0) {
            if (argc == 0) {
                fprintf(stderr, "ERROR: No argument is provided for flag '%s'\n", flag);
                usage(stderr);
                exit(1);
            }
            inputFilePath = shift(&argc, &argv);
        } else if (strcmp(flag, "-o") == 0) {
            if (argc == 0) {
                fprintf(stderr, "ERROR: No argument is provided for flag '%s'\n", flag);
                usage(stderr);
                exit(1);
            }
            outputFilePath = shift(&argc, &argv);
        } else if (strcmp(flag, "-h") == 0) {
            usage(stdout);
            exit(0);
        } else if (strcmp(flag, "-d") == 0) {
            debug = 1;
        } else if (strcmp(flag, "-c") == 0) {
            wos = true;
        } else {
            error = 1;
            errorFlag = flag;
        }
    }

    if (inputFilePath == NULL) {
        fprintf(stderr, "ERROR: Expected input file!\n");
        usage(stderr);
        exit(1);
    }

    if (outputFilePath == NULL) {
        fprintf(stderr, "ERROR: Expected output file!\n");
        usage(stderr);
        exit(1);
    }

    if (error) {
        fprintf(stderr, "ERROR: Unknown flag '%s'!\n", errorFlag);
        usage(stderr);
        exit(1);
    }

    mvm_translateSourceFile(&masm, cstr_as_sv(inputFilePath), 0);
    masm_saveToFile(&masm, outputFilePath, wos);

    if (debug) {
        printf("[DEBUG]: Consumed %lld bytes of memory.\n", masm.memarena_size);
    }

    return  0;
}

