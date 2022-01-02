//
// Created by iinsert on 29.12.2021.
//

#define SVM_IMPLEMENTATION
#include "../shared.h"

SVM svm = {0};

int main(int argc, char** argv)
{
    char* programName = shift(&argc, &argv);
    char* input = shift(&argc, &argv);

    if (argc < 3) {
        fprintf(stderr, "ERROR: Expected input and output!\n");
        fprintf(stderr, "Usage: %s <input.vsm> <output.sbc>\n", programName);
        exit(1);
    }

    const char* inputFilePath = argv[1];
    const char* outputFilePath = argv[2];

    StringView source_code = slurp_file(inputFilePath);

    svm.program_size = svm_translateSource(
            source_code,
            svm.program,
            SVM_PROGRAM_CAPACITY);

    svm_saveProgramToFile(&svm, outputFilePath);

    return  0;
}

