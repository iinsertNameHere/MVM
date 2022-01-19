//
// Created by iinsert on 30.12.2021.
//

#define MVM_SHARED_IMPLEMENTATION
#include "../shared.h"

MVM mvm = {0};

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "ERROR: Expected input!\n");
        fprintf(stderr, "Usage: demasm <input.mbc>\n");
        exit(1);
    }

    const char* inputFilePath = argv[1];

    mvm_loadProgramFromFile(&mvm, inputFilePath);

    for (InstAddr i = 0; i < mvm.program_size; ++i) {
        printf(InstName(mvm.program[i].type));
        if (InstHasOperand(mvm.program[i].type)) {
            printf(" %lld\n", mvm.program[i].operand.as_i64);
        } else {
            printf("\n");
        }
    }

    return 0;
}
