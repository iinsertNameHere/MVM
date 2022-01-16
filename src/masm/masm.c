//
// Created by iinsert on 29.12.2021.
//

#define MVM_SHARED_IMPLEMENTATION
#include "../shared.h"

MVM mvm = {0};
Masm masm = {0};

static void usage(FILE* stream)
{
    fprintf(stream, "Usage: masm <input.msm> <output.mbc>\n");
}

int main(int argc, char** argv)
{
    shift(&argc, &argv); // Skip program name.
    if (argc == 0) {
        fprintf(stderr, "ERROR: Expected input!\n");
        usage(stderr);
        exit(1);
    }
    const char* inputFilePath = shift(&argc, &argv);

    if (argc == 0) {
        fprintf(stderr, "ERROR: Expected output!\n");
        usage(stderr);
        exit(1);
    }
    const char* outputFilePath = shift(&argc, &argv);

    StringView source_code = slurp_file(inputFilePath);

    mvm_translateSource(source_code, inputFilePath, &mvm, &masm);
    mvm_saveProgramToFile(&mvm, outputFilePath);

    return  0;
}

