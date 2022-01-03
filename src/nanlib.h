#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#ifndef SVM_NANLIB_H
#define SVM_NANLIB_H

void printBits(uint8_t* bytes, size_t bytes_size);

#define EXP_MASK (((1LL << 11LL) - 1LL) << 52LL)
#define FRACTION_MASK ((1LL << 52LL) - 1LL)
#define TYPE_MASK (((1LL << 4LL) - 1LL) << 48LL)
#define VALUE_MASK ((1LL << 48LL) - 1LL)

#define TYPE(n) ((1LL << 3LL) + n)

double mkInf();
double setType(double x, uint64_t type);
uint64_t getType(double x);
double setValue(double x, uint64_t value);
uint64_t getValue(double x);

#define DOUBLE_TYPE 0
#define INTEGER_TYPE 1
#define POINTER_TYPE 2

int isDouble(double x);
int isInteger(double x);
int isPointer(double x);
double asDouble(double x);
uint64_t asInteger(double x);
void* asPointer(double x);
double boxDouble(double x);
double boxInteger(uint64_t x);
double boxPointer(void* x);

#define INSPECT_VALUE(type, value, label)           \
    {                                               \
        type vari = (value) ;                       \
        printf("%s =>\n  ", label) ;                \
        printBits((uint8_t*)&vari, sizeof(vari));   \
        printf("  isnan = %d\n", isnan(vari));      \
    }

#endif //SVM_NANLIB_H

#ifdef SVM_NANLIB_IMPLEMENTATION

void printBits(uint8_t* bytes, size_t bytes_size)
{
    for (int i = (int)bytes_size - 1; i >= 0; --i) {
        uint8_t byte = bytes[i];
        for (int j = 7; j >= 0; --j) {
            printf("%d", !!(byte & (1 << j)));
        }
        printf(" ");
    }
    printf("\n");
}

double mkInf()
{
    uint64_t y = EXP_MASK;
    return *(double*)&y;
}

double setType(double x, uint64_t type)
{
    uint64_t y = *(uint64_t*)&x;
    y = (y & (~TYPE_MASK)) | (((TYPE_MASK >> 48LL) & type) << 48LL);
    return *(double*)&y;
}

uint64_t getType(double x)
{
    uint64_t y = (*(uint64_t*)&x);
    return (y & TYPE_MASK) >> 48LL;
}

double setValue(double x, uint64_t value)
{
    uint64_t y = *(uint64_t*)&x;
    y = (y & (~VALUE_MASK)) | (value & VALUE_MASK);
    return *(double*)&y;
}

uint64_t getValue(double x)
{
    uint64_t y = (*(uint64_t*)&x);
    return (y & VALUE_MASK);
}

int isDouble(double x)
{
    return !isnan(x);
}

int isInteger(double x)
{
    return isnan(x) && getType(x) == TYPE(INTEGER_TYPE);
}

int isPointer(double x)
{
    return isnan(x) && getType(x) == TYPE(POINTER_TYPE);
}

double asDouble(double x)
{
    return x;
}

uint64_t asInteger(double x)
{
    return getValue(x);
}

void* asPointer(double x)
{
    return (void*)getValue(x);
}

double boxDouble(double x)
{
    return x;
}

double boxInteger(uint64_t x)
{
    return setValue(setType(mkInf(), TYPE(INTEGER_TYPE)), x);
}

double boxPointer(void* x)
{
    return setValue(setType(mkInf(), TYPE(POINTER_TYPE)), (uint64_t)x);
}

#endif //SVM_NANLIB_IMPLEMENTATION