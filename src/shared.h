//
// Created by iinsert on 29.12.2021.
//

#ifndef MVM_SHARED_H
#define MVM_SHARED_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

//////////// Data Definitions ////////////
#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof((xs)[0]))

typedef uint64_t InstAddr;

typedef union {
    uint64_t as_u64;
    int64_t as_i64;
    double as_f64;
    void* as_ptr;
} Word;

static_assert(sizeof(Word) == 8, "The MVM's Word is expected to be 64 bits!");

typedef struct {
    size_t count;
    const char* data;
} StringView;

StringView cstr_as_sv(const char* cstr);
StringView sv_vasmrim(StringView sv);
StringView sv_rtrim(StringView sv);
StringView sv_trim(StringView sv);
StringView sv_chopByDelim(StringView* sv, char delim);
int sv_eq(StringView a, StringView b);
int sv_as_int(StringView sv);

//////////// EXEPTION Definitions ////////////
typedef enum {
    EXEPTION_SATE_OK = 0,
    EXEPTION_STACK_OVERFLOW,
    EXEPTION_STACK_UNDERFLOW,
    EXEPTION_DIV_BY_ZERO,
    EXEPTION_ILLEGAL_INST,
    EXEPTION_ILLEGAL_INST_ACCESS,
    EXEPTION_ILLEGAL_OPERAND,
} ExeptionState;

const char* exeption_as_cstr(ExeptionState exeption);

//////////// INST Definitions ////////////
typedef enum {
    INST_NOP = 0,
    INST_PUSH,
    INST_DUP,
    INST_SWAP,
    INST_PLUSI,
    INST_MINUSI,
    INST_MULTI,
    INST_DIVI,
    INST_PLUSF,
    INST_MINUSF,
    INST_MULTF,
    INST_DIVF,
    INST_JMP,
    INST_JMP_IF,
    INST_EQ,
    INST_NOT,
    INST_GEF,
    INST_HALT,
    INST_PRINT_DEBUG,

    AMOUNT_OF_INSTS
} InstType;

const char* InstName(InstType instType);

int InstHasOperand(InstType instType);

typedef struct {
    InstType type;
    Word operand;
} Inst;

const char* inst_as_cstr(InstType instType);

#define MAKE_INST_PUSH(value)    {.type = INST_PUSH, .operand = (value)}
#define MAKE_INST_DUP(stackAddr) {.type = INST_DUP, .operand = (stackAddr)}
#define MAKE_INST_PLUS           {.type = INST_PLUS}
#define MAKE_INST_MINUS          {.type = INST_MINUS}
#define MAKE_INST_MULT           {.type = INST_MULT}
#define MAKE_INST_DIV            {.type = INST_DIV}
#define MAKE_INST_JMP(addr)      {.type = INST_JMP, .operand = (addr)}
#define MAKE_INST_HALT           {.type = INST_HALT}

#define CONSTRUCT_INST(instType)                (Inst) {.type = (instType)}
#define CONSTRUCT_OPERAND_INST(instType, value) (Inst) {.type = (instType), .operand = (value)}

//////////// Label Definitions ////////////
#define LABEL_CAPACITY 1024
#define DEFERED_OPERANDS_CAPACITY 1024

typedef struct {
    StringView name;
    InstAddr addr;
} Label;

typedef struct {
    StringView label;
    InstAddr addr;
} DeferredOperand;

typedef struct {
    Label labels[LABEL_CAPACITY];
    size_t lables_size;
    DeferredOperand deferredOperands[DEFERED_OPERANDS_CAPACITY];
    size_t deferredOperands_size;
} Vasm;

InstAddr vasm_findLabelAddr(const Vasm* vasm, StringView name);
void vasm_pushLabel(Vasm* vasm, StringView name, InstAddr addr);
void vasm_pushDeferredOperand(Vasm* vasm, InstAddr addr, StringView label);

//////////// MVM Definitions ////////////
#define MVM_STACK_CAPACITY 942 //TODO: Fix stack underflow if lager than 942.
#define MVM_PROGRAM_CAPACITY 1024

typedef struct {
    Word stack[MVM_STACK_CAPACITY];
    uint64_t stack_size;

    Inst program[MVM_PROGRAM_CAPACITY];
    uint64_t program_size;
    InstAddr ip;

    int halt;
} MVM;

ExeptionState mvm_execInst(MVM* mvm);
void mvm_dumpStack(FILE *stream, const MVM* mvm);
void mvm_loadProgramFromMemory(MVM* mvm, Inst* program, size_t program_size);
void mvm_saveProgramToFile(const MVM* mvm, const char* file_path);
void mvm_loadProgramFromFile(MVM* mvm, const char* file_path);
Word numberLiteral_as_Word (StringView sv);
void mvm_translateSource(StringView source, MVM* mvm, Vasm* vasm);
ExeptionState mvm_execProgram(MVM* mvm, int limit);

//////////// Util Definitions ////////////
StringView slurp_file(const char* file_path);
char* shift(int* argc, char*** argv);
#endif //MVM_SHARED_H

#ifdef MVM_SHARED_IMPLEMENTATION

//////////// Data Definitions ////////////
StringView cstr_as_sv(const char* cstr)
{
    return (StringView) {
            .count = strlen(cstr),
            .data = cstr,
    };
}

StringView sv_vasmrim(StringView sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[i])) {
        i += 1;
    }
    return (StringView) {
            .count = sv.count - i,
            .data = sv.data + i,
    };
}

StringView sv_rtrim(StringView sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[sv.count - 1 - i])) {
        i += 1;
    }
    return (StringView) {
            .count = sv.count - i,
            .data = sv.data,
    };
}

StringView sv_trim(StringView sv)
{
    return sv_vasmrim(sv_rtrim(sv));
}

StringView sv_chopByDelim(StringView* sv, char delim)
{
    size_t i = 0;
    while (i < sv->count && sv->data[i] != delim) {
        i += 1;
    }
    StringView resuvasm = {
            .count = i,
            .data = sv->data
    };
    if (i < sv->count) {
        sv->count -= i + 1;
        sv->data  += i + 1;
    } else {
        sv->count -= i;
        sv->data  += i;
    }
    return resuvasm;
}

int sv_eq(StringView a, StringView b)
{
    if (a.count != b.count) {
        return 0;
    }
    else {
        return memcmp(a.data, b.data, a.count) == 0;
    }
}

int sv_as_int(StringView sv)
{
    int resuvasm = 0;
    for (size_t i = 0; i < sv.count && isdigit(sv.data[i]); ++i) {
        resuvasm = resuvasm * 10 + sv.data[i] - '0';
    }
    return resuvasm;
}

//////////// EXEPTION Definitions ////////////
const char* exeption_as_cstr(ExeptionState exeption)
{
    switch (exeption) {
        case EXEPTION_SATE_OK:             return "EXEPTION_STATE_OK";
        case EXEPTION_STACK_OVERFLOW:      return "EXEPTION_STACK_OVERFLOW";
        case EXEPTION_STACK_UNDERFLOW:     return "EXEPTION_STACK_UNDERFLOW";
        case EXEPTION_DIV_BY_ZERO:         return "EXEPTION_DIV_BY_ZERO";
        case EXEPTION_ILLEGAL_INST:        return "EXEPTION_ILLEGAL_INST";
        case EXEPTION_ILLEGAL_INST_ACCESS: return "EXEPTION_ILLEGAL_INST_ACCESS";
        case EXEPTION_ILLEGAL_OPERAND:     return "EXEPTION_ILLEGAL_OPERAND";
        default:
            assert(0 && "exeption_as_cstr: Unreachable");
    }
    assert(0 && "exeption_as_cstr: Unreachable");
    return "";
}

//////////// INST Definitions ////////////
const char* inst_as_cstr(InstType instType) {
    switch (instType) {
        case INST_NOP:         return "INST_NOP";
        case INST_PUSH:        return "INST_PUSH";
        case INST_DUP:         return "INST_DUP";
        case INST_SWAP:        return "INST_SWAP";
        case INST_PLUSI:       return "INST_PLUSI";
        case INST_MINUSI:      return "INST_MINUSI";
        case INST_MULTI:       return "INST_MULTI";
        case INST_DIVI:        return "INST_DIVI";
        case INST_PLUSF:       return "INST_PLUSF";
        case INST_MINUSF:      return "INST_MINUSF";
        case INST_MULTF:       return "INST_MULTF";
        case INST_DIVF:        return "INST_DIVF";
        case INST_JMP:         return "INST_JMP";
        case INST_JMP_IF:      return "INST_JMP_IF";
        case INST_EQ:          return "INST_EQ";
        case INST_NOT:         return "INST_NOT";
        case INST_GEF:         return "INST_GEF";
        case INST_HALT:        return "INST_HALT";
        case INST_PRINT_DEBUG: return "INST_DEBUG";
        case AMOUNT_OF_INSTS:
        default:
            assert(0 && "inst_as_cstr: Unreachable");
    }
    assert(0 && "inst_as_cstr: Unreachable");
    return "";
}

const char* InstName(InstType instType)
{
    switch (instType) {
        case INST_NOP:         return "nop";
        case INST_PUSH:        return "push";
        case INST_DUP:         return "dup";
        case INST_SWAP:        return "swap";
        case INST_PLUSI:       return "plusi";
        case INST_MINUSI:      return "minusi";
        case INST_MULTI:       return "multi";
        case INST_DIVI:        return "divi";
        case INST_PLUSF:       return "plusf";
        case INST_MINUSF:      return "minusf";
        case INST_MULTF:       return "multf";
        case INST_DIVF:        return "divf";
        case INST_JMP:         return "jmp";
        case INST_JMP_IF:      return "jmpif";
        case INST_EQ:          return "eq";
        case INST_NOT:         return "not";
        case INST_GEF:         return "gef";
        case INST_HALT:        return "hlt";
        case INST_PRINT_DEBUG: return "dbgPrint";
        case AMOUNT_OF_INSTS:
        default:
            assert(0 && "InstName: Unreachable");
    }
    assert(0 && "InstName: Unreachable");
    return "";
}

int InstHasOperand(InstType instType)
{
    switch (instType) {
        case INST_NOP:         return 0;
        case INST_PUSH:        return 1;
        case INST_DUP:         return 1;
        case INST_SWAP:        return 1;
        case INST_PLUSI:       return 0;
        case INST_MINUSI:      return 0;
        case INST_MULTI:       return 0;
        case INST_DIVI:        return 0;
        case INST_PLUSF:       return 0;
        case INST_MINUSF:      return 0;
        case INST_MULTF:       return 0;
        case INST_DIVF:        return 0;
        case INST_JMP:         return 1;
        case INST_JMP_IF:      return 1;
        case INST_EQ:          return 0;
        case INST_NOT:         return 0;
        case INST_GEF:         return 0;
        case INST_HALT:        return 0;
        case INST_PRINT_DEBUG: return 0;
        case AMOUNT_OF_INSTS:
        default:
            assert(0 && "InstHasOperand: Unreachable");
    }
    assert(0 && "InstHasOperand: Unreachable");
    return 0;
}

//////////// MVM Definitions ////////////
ExeptionState mvm_execProgram(MVM* mvm, int limit)
{
    while (limit != 0 && !mvm->halt) {
        ExeptionState err = mvm_execInst(mvm);
        if (mvm->stack_size > MVM_STACK_CAPACITY) {
            return EXEPTION_STACK_OVERFLOW;
        }
        //mvm_dumpStack(stdout, mvm);
        if (err != EXEPTION_SATE_OK) {
            return err;
        }
        if (limit > 0) {
            --limit;
        }
    }
    return EXEPTION_SATE_OK;
}

ExeptionState mvm_execInst(MVM* mvm)
{
    if (mvm->ip >= mvm->program_size) {
        return EXEPTION_ILLEGAL_INST_ACCESS;
    }

    Inst inst = mvm->program[mvm->ip];

    switch (inst.type) {
        case INST_NOP: {
            mvm->ip += 1;
            break;
        }
        case INST_PUSH: {
            if (mvm->stack_size >= MVM_STACK_CAPACITY) {
                return EXEPTION_STACK_OVERFLOW;
            }
            mvm->stack[mvm->stack_size++] = inst.operand;
            mvm->ip += 1;
            break;
        }

        case INST_DUP: {
            if (mvm->stack_size > MVM_STACK_CAPACITY) {
                return EXEPTION_STACK_OVERFLOW;
            }
            if (mvm->stack_size - inst.operand.as_u64 <= 0) {
                return EXEPTION_STACK_UNDERFLOW;
            }

            mvm->stack[mvm->stack_size] = mvm->stack[mvm->stack_size - 1 - inst.operand.as_u64];
            mvm->stack_size += 1;
            mvm->ip += 1;
            break;
        }

        case INST_SWAP: {
            if (inst.operand.as_u64 >= mvm->stack_size) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            const uint64_t a = mvm->stack_size - 1;
            const uint64_t b = mvm->stack_size - 1 - inst.operand.as_u64;
            Word tmp = mvm->stack[a];
            mvm->stack[a] = mvm->stack[b];
            mvm->stack[b] = tmp;
            mvm->ip += 1;
            break;
        }

        case INST_PLUSI: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 2].as_u64 += mvm->stack[mvm->stack_size - 1].as_u64;
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_MINUSI: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 2].as_u64 -= mvm->stack[mvm->stack_size - 1].as_u64;
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_MULTI: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 2].as_u64 *= mvm->stack[mvm->stack_size - 1].as_u64;
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_DIVI: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            if (mvm->stack[mvm->stack_size - 1].as_u64 == 0) {
                return EXEPTION_DIV_BY_ZERO;
            }
            mvm->stack[mvm->stack_size - 2].as_u64 /= mvm->stack[mvm->stack_size - 1].as_u64;
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_PLUSF: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 2].as_f64 += mvm->stack[mvm->stack_size - 1].as_f64;
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_MINUSF: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 2].as_f64 -= mvm->stack[mvm->stack_size - 1].as_f64;
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_MULTF: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 2].as_f64 *= mvm->stack[mvm->stack_size - 1].as_f64;
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_DIVF: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }

            mvm->stack[mvm->stack_size - 2].as_f64 /= mvm->stack[mvm->stack_size - 1].as_f64 ;
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_JMP: {
            mvm->ip = inst.operand.as_u64;
            break;
        }

        case INST_HALT: {
            mvm->halt = 1;
            break;
        }

        case INST_EQ: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 2].as_u64 = (mvm->stack[mvm->stack_size - 1].as_u64 == mvm->stack[mvm->stack_size - 2].as_u64);
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_NOT: {
            if (mvm->stack_size < 1) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 1].as_u64 = !mvm->stack[mvm->stack_size - 1].as_u64;
            mvm->ip += 1;
            break;
        }

        case INST_GEF: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 2].as_f64 = (mvm->stack[mvm->stack_size - 1].as_f64 >= mvm->stack[mvm->stack_size - 2].as_f64);
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_JMP_IF: {
            if (mvm->stack_size < 1) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            if (mvm->stack[mvm->stack_size - 1].as_u64) {
                mvm->ip = inst.operand.as_u64;
            } else {
                mvm->ip += 1;
            }
            mvm->stack_size -= 1;
            break;
        }

        case INST_PRINT_DEBUG: {
            if (mvm->stack_size < 1) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            printf("  u64: %llu | i64: %lld | f64: %lf",
                    mvm->stack[mvm->stack_size - 1].as_u64,
                    mvm->stack[mvm->stack_size - 1].as_i64,
                    mvm->stack[mvm->stack_size - 1].as_f64);
            if ((uintptr_t)mvm->stack[mvm->stack_size].as_ptr > 0) {
                printf(" | ptr: 0x%llx\n", (uintptr_t) mvm->stack[mvm->stack_size - 1].as_ptr);
            } else {
                printf(" | ptr: (nil)\n");
            }
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case AMOUNT_OF_INSTS:
        default:
            return EXEPTION_ILLEGAL_INST;
    }
    return EXEPTION_SATE_OK;
}

void mvm_dumpStack(FILE *stream, const MVM* mvm)
{
    fprintf(stream, "STACK:\n");
    if (mvm->stack_size > 0) {
        for (InstAddr i = 0; i < mvm->stack_size; ++i) {
            fprintf(stream, "  u64: %llu | i64: %lld | f64: %lf",
                    mvm->stack[i].as_u64,
                    mvm->stack[i].as_i64,
                    mvm->stack[i].as_f64);
            if ((uintptr_t)mvm->stack[i].as_ptr > 0) {
                fprintf(stream, " | ptr: 0x%llx\n", (uintptr_t) mvm->stack[i].as_ptr);
            } else {
                fprintf(stream, " | ptr: (nil)\n");
            }
        }
    } else {
        fprintf(stream, " [empty]\n");
    }
}

void mvm_loadProgramFromMemory(MVM* mvm, Inst* program, size_t program_size)
{
    assert(program_size < MVM_PROGRAM_CAPACITY);
    memcpy(mvm->program, program, sizeof(program[0]) * program_size);
    mvm->program_size = program_size;
}

void mvm_saveProgramToFile(const MVM* mvm, const char* file_path)
{
    FILE* f = fopen(file_path, "wb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Could not open file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }

    fwrite(mvm->program, sizeof(mvm->program[0]), mvm->program_size, f);
    if (ferror(f)) {
        fprintf(stderr, "ERROR: Could not write to file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }

    fclose(f);
}

void mvm_loadProgramFromFile(MVM* mvm, const char* file_path)
{
    FILE* f = fopen(file_path, "rb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Could not open file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }

    long m = ftell(f);
    if (m < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }

    assert(m % sizeof(mvm->program[0]) == 0);
    assert((size_t) m <= MVM_PROGRAM_CAPACITY * sizeof(mvm->program[0]));

    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }

    size_t size = fread(mvm->program, sizeof(mvm->program[0]), m / sizeof(mvm->program[0]), f);
    if (ferror(f)) {
        fprintf(stderr, "ERROR: Could not read file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }
    mvm->program_size = (int)size;

    fclose(f);
}

Word numberLiteral_as_Word (StringView sv)
{
    assert(sv.count < 1024);
    char cstr[sv.count + 1];
    memcpy(cstr, sv.data, sv.count);
    cstr[sv.count] = '\0';

    Word result = {0};

    char* endptr = 0;
    result.as_u64 = strtoull(cstr, &endptr, 10);
    if ((size_t)(endptr - cstr) != sv.count) {
        result.as_f64 = strtod(cstr, &endptr);
        if ((size_t)(endptr - cstr) != sv.count) {
            fprintf(stderr, "ERROR: '%s' is not a number literal!", cstr);
            exit(1);
        }
    }
    return result;
}

void mvm_translateSource(StringView source, MVM* mvm, Vasm* vasm)
{
    mvm->program_size = 0;

    // Pass one
    while (source.count > 0) {
        assert(mvm->program_size < MVM_PROGRAM_CAPACITY);
        StringView  line = sv_trim(sv_chopByDelim(&source, '\n'));
        if (line.count > 0 && *line.data != '#') {
            StringView  instName = sv_chopByDelim(&line, ' ');

            if (instName.count > 0 && instName.data[instName.count - 1] == ':') {
                StringView label = (StringView) { .count = instName.count - 1, .data = instName.data };
                vasm_pushLabel(vasm, label, mvm->program_size);
                instName = sv_trim(sv_chopByDelim(&line, ' '));
            }

            if (instName.count > 0) {
                StringView operand = sv_trim(sv_chopByDelim(&line, '#'));
                if (sv_eq(instName, cstr_as_sv(InstName(INST_NOP)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_NOP
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_PUSH)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_PUSH,
                            .operand = numberLiteral_as_Word(operand)
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_DUP)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_DUP,
                            .operand = { .as_i64 = sv_as_int(operand)}
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_SWAP)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_SWAP,
                            .operand = { .as_i64 = sv_as_int(operand)}
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_PLUSI)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_PLUSI
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_MINUSI)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_MINUSI
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_MULTI)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_MULTI
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_DIVI)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_DIVI
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_PLUSF)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_PLUSF
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_MINUSF)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_MINUSF
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_MULTF)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_MULTF
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_DIVF)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_DIVF
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_HALT)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_HALT
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_EQ)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_EQ
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_NOT)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_NOT
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_GEF)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_GEF
                    };
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_PRINT_DEBUG)))) {
                    mvm->program[mvm->program_size++] = (Inst) {
                            .type = INST_PRINT_DEBUG
                    };

                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_JMP)))) {
                    if (operand.count > 0 && isdigit(*operand.data)) {
                        mvm->program[mvm->program_size++] = (Inst) {
                                .type = INST_JMP,
                                .operand = { .as_i64 = sv_as_int(operand)}
                        };
                    } else {
                        vasm_pushDeferredOperand(vasm, mvm->program_size, operand);
                        mvm->program[mvm->program_size++] = (Inst) {
                                .type = INST_JMP
                        };
                    }
                } else if (sv_eq(instName, cstr_as_sv(InstName(INST_JMP_IF)))) {
                    if (operand.count > 0 && isdigit(*operand.data)) {
                        mvm->program[mvm->program_size++] = (Inst) {
                                .type = INST_JMP_IF,
                                .operand = { .as_i64 = sv_as_int(operand)}
                        };
                    } else {
                        vasm_pushDeferredOperand(vasm, mvm->program_size, operand);
                        mvm->program[mvm->program_size++] = (Inst) {
                                .type = INST_JMP_IF
                        };
                    }
                } else {
                    fprintf(stderr, "ERORR: Unknown instruction '%.*s'!\n", (int) instName.count, instName.data);
                    exit(1);
                }
            }
        }
    }

    // Pass two
    for (size_t i = 0; i < vasm->deferredOperands_size; ++i) {
        InstAddr addr = vasm_findLabelAddr(vasm, vasm->deferredOperands[i].label);
        mvm->program[vasm->deferredOperands[i].addr].operand.as_u64 = addr;
    }
}

//////////// Label Definitions ////////////
InstAddr vasm_findLabelAddr(const Vasm* vasm, StringView name)
{
    for (size_t i = 0; i < vasm->lables_size; ++i) {
        if (sv_eq(vasm->labels[i].name, name)) {
            return vasm->labels[i].addr;
        }
    }
    fprintf(stderr, "ERROR: Label '%.*s' does not exist!\n",
            (int)name.count, name.data);
    exit(1);
    return -1;
}

void vasm_pushLabel(Vasm* vasm, StringView name, InstAddr addr)
{
    assert(vasm->lables_size < LABEL_CAPACITY);
    vasm->labels[vasm->lables_size++] = (Label) { .name = name, .addr = addr };
}

void vasm_pushDeferredOperand(Vasm* vasm, InstAddr addr, StringView label)
{
    assert(vasm->deferredOperands_size < DEFERED_OPERANDS_CAPACITY);
    vasm->deferredOperands[vasm->deferredOperands_size++] = (DeferredOperand) {
        .addr = addr,
        .label = label
    };
}

//////////// Util Definitions ////////////
StringView slurp_file(const char* file_path)
{
    FILE* f = fopen(file_path, "r");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Could not open file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }

    long m = ftell(f);
    if (m < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }

    char* buffer = malloc(m);
    if (buffer == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory for file! : %s\n", strerror(errno));
        exit(1);
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }

    size_t n = fread(buffer, 1, m, f);
    if (ferror(f)) {
        fprintf(stderr, "ERROR: Could not read file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }

    fclose(f);

    return (StringView) {
            .count = n,
            .data = buffer,
    };
}

char* shift(int* argc, char*** argv)
{
    assert(*argc > 0);
    char* res = **argv;
    *argv += 1;
    *argc -= 1;
    return res;
}
#endif //MVM_SHARED_IMPLEMENTATION