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

    for (Word i = 0; i < svm.program_size; ++i) {
        switch (svm.program[i].type) {
            case INST_NOP:
                printf("nop\n");
                break;
            case INST_PUSH:
                printf("push %lld\n", svm.program[i].operand);
                break;
            case INST_DUP:
                printf("dup %lld\n", svm.program[i].operand);
                break;
            case INST_PLUS:
                printf("plus\n");
                break;
            case INST_MINUS:
                printf("minus\n");
                break;
            case INST_MULT:
                printf("mult\n");
                break;
            case INST_DIV:
                printf("div\n");
                break;
            case INST_JMP:
                printf("jmp %lld\n", svm.program[i].operand);
                break;
            case INST_JMP_IF:
                printf("jmpif %lld\n", svm.program[i].operand);
                break;
            case INST_EQ:
                printf("eq\n");
                break;
            case INST_HALT:
                printf("halt\n");
                break;
            case INST_PRINT_DEBUG:
                printf("dbgPrint\n");
                break;
        }
    }

    return 0;
}