//
// Created by iinsert on 29.12.2021.
//

#define SVM_IMPLEMENTATION
#include "../shared.h"

SVM svm = {0};
Vasm vasm = {0};

// TODO: Watch https://www.youtube.com/watch?v=9td67NTtNCg&list=PLpM-Dvs8t0VY73ytTCQqgvgCWttV3m8LM&index=5 at 3:23:01

static void usage(FILE* stream)
{
    fprintf(stream, "Usage: vasm <input.vsm> <output.sbc>\n");
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

    svm_translateSource(source_code, &svm, &vasm);
    svm_saveProgramToFile(&svm, outputFilePath);

    return  0;
}

