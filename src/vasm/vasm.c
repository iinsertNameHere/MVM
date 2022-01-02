//
// Created by iinsert on 29.12.2021.
//

#define SVM_IMPLEMENTATION
#include "../shared.h"

SVM svm = {0};

void usage(FILE* stream) {
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

    svm.program_size = svm_translateSource(
            source_code,
            svm.program,
            SVM_PROGRAM_CAPACITY);

    svm_saveProgramToFile(&svm, outputFilePath);

    return  0;
}

