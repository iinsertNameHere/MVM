//
// Created by iinsert on 29.12.2021.
//

#define SVM_IMPLEMENTATION
#include "../shared.h"

// Slick VM
SVM svm = {0};

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "ERROR: Expected input!\n");
        fprintf(stderr, "Usage: svm <input.sbc>\n");
        exit(1);
    }

    svm_loadProgramFromFile(&svm, argv[1]);

    // TODO: svm_execProgram()
    for (int i = 0; i < SVM_EXECUTION_LIMIT && !svm.halt; ++i) {
        ExeptionState state = svm_execInst(&svm);
        if (state != EXEPTION_SATE_OK) {
            fprintf(stderr, "ERROR: Failed to execute! : %s\n", exeption_as_cstr(state));
            exit(1);
        }
    }
    svm_dumpStack(stdout, &svm);

    return 0;
}

