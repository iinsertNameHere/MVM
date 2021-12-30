//
// Created by iinsert on 29.12.2021.
//

#define SVM_IMPLEMENTATION
#include "../shared.h"

SVM svm = {0};

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "ERROR: Expected input and output!\n");
        fprintf(stderr, "Usage: sasm <input.sasm> <output.sbc>\n");
        exit(1);
    }

    const char* inputFilePath = argv[1];
    const char* outputFilePath = argv[2];

    StringView source_code = slurp_file(inputFilePath);

    svm.program_size = svm_translateSource(
            source_code,
            svm.program,
            SVM_PROGRAM_CAPACITY);

    svm_saveProgramToFile(
            svm.program,
            svm.program_size,
            outputFilePath);

    return  0;
}

