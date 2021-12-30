//
// Created by joona on 30.12.2021.
//

#define SVM_IMPLEMENTATION
#include "../shared.h"

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "ERROR: Expected input!\n");
        fprintf(stderr, "Usage: desasm <input.sbc>\n");
        exit(1);
    }
    return 0;
}