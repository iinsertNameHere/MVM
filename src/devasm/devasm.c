//
// Created by joona on 30.12.2021.
//

#define SVM_SHARED_IMPLEMENTATION
#include "../shared.h"

SVM svm = {0};

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "ERROR: Expected input!\n");
        fprintf(stderr, "Usage: desasm <input.sbc>\n");
        exit(1);
    }

    const char* inputFilePath = argv[1];

    svm_loadProgramFromFile(&svm, inputFilePath);

    for (InstAddr i = 0; i < svm.program_size; ++i) {
        printf("%s", InstNames[svm.program[i].type]);
        if (HasInstOperand[svm.program[i].type]) {
            printf(" %lld", svm.program[i].operand.as_i64);
        }
        printf("\n");
    }

    return 0;
}