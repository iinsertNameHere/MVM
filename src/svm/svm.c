//
// Created by iinsert on 29.12.2021.
//

#define SVM_IMPLEMENTATION
#include "../shared.h"

// Slick VM
SVM svm = {0};

//TODO: Watch https://www.youtube.com/watch?v=9td67NTtNCg&list=PLpM-Dvs8t0VY73ytTCQqgvgCWttV3m8LM&index=4 at 1:10:02.
//NOTE: GitHub https://github.com/iinsertNameHere/Slick

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "ERROR: Expected input!\n");
        fprintf(stderr, "Usage: svm <input.sbc>\n");
        exit(1);
    }

    svm_loadProgramFromFile(&svm, argv[1]);
    ExeptionState state = svm_execProgram(&svm, 69);
    if (state != EXEPTION_STACK_OVERFLOW) {
        svm_dumpStack(stdout, &svm);
    }
    if (state != EXEPTION_SATE_OK) {
        fprintf(stderr, "ERROR: Failed to execute program! : %s\n", exeption_as_cstr(state));
        return 1;
    }
    return 0;
}

