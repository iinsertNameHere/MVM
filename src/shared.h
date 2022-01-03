//
// Created by iinsert on 29.12.2021.
//

#ifndef SVM_SHARED_H
#define SVM_SHARED_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

//////////// Data Definitions ////////////
#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof((xs)[0]))

typedef int64_t Word;

typedef struct {
    size_t count;
    const char* data;
} StringView;

StringView cstr_as_sv(const char* cstr);
StringView sv_ltrim(StringView sv);
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
    INST_PLUS,
    INST_MINUS,
    INST_MULT,
    INST_DIV,
    INST_JMP,
    INST_JMP_IF,
    INST_EQ,
    INST_HALT,
    INST_PRINT_DEBUG,
} Inst_Type;

typedef struct {
    Inst_Type type;
    Word operand;
} Inst;

const char* inst_as_cstr(Inst_Type instType);

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
#define UNRESOLVED_JMP_CAPACITY 1024

typedef struct {
    StringView name;
    Word addr;
} Label;

typedef struct {
    StringView label;
    Word addr;
} UnresolvedJmp;

typedef struct {
    Label labels[LABEL_CAPACITY];
    size_t lables_size;
    UnresolvedJmp unresolvedJmps[UNRESOLVED_JMP_CAPACITY];
    size_t unresolvedJmps_size;
} LabelTable;

Word lt_find(const LabelTable* lt, StringView name);
void lt_push(LabelTable* lt, StringView name, Word addr);
void lt_push_ujmp(LabelTable* lt, Word addr, StringView label);

//////////// SVM Definitions ////////////
#define SVM_STACK_CAPACITY 942 //TODO: Fix stack underflow if lager than 942.
#define SVM_PROGRAM_CAPACITY 1024

typedef struct {
    Word stack[SVM_STACK_CAPACITY];
    Word stack_size;

    Inst program[SVM_PROGRAM_CAPACITY];
    Word program_size;
    Word ip;

    int halt;
} SVM;

ExeptionState svm_execInst(SVM* svm);
void svm_dumpStack(FILE *stream, const SVM* svm);
void svm_loadProgramFromMemory(SVM* svm, Inst* program, size_t program_size);
void svm_saveProgramToFile(const SVM* svm, const char* file_path);
void svm_loadProgramFromFile(SVM* svm, const char* file_path);
void svm_translateSource(StringView source, SVM* svm, LabelTable* lt);
ExeptionState svm_execProgram(SVM* svm, int limit);

//////////// Util Definitions ////////////
StringView slurp_file(const char* file_path);
char* shift(int* argc, char*** argv);
#endif //SVM_SHARED_H

#ifdef SVM_IMPLEMENTATION

//////////// Data Definitions ////////////
StringView cstr_as_sv(const char* cstr)
{
    return (StringView) {
            .count = strlen(cstr),
            .data = cstr,
    };
}

StringView sv_ltrim(StringView sv)
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
    return sv_ltrim(sv_rtrim(sv));
}

StringView sv_chopByDelim(StringView* sv, char delim)
{
    size_t i = 0;
    while (i < sv->count && sv->data[i] != delim) {
        i += 1;
    }
    StringView result = {
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
    return result;
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
    int result = 0;
    for (size_t i = 0; i < sv.count && isdigit(sv.data[i]); ++i) {
        result = result * 10 + sv.data[i] - '0';
    }
    return result;
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
const char* inst_as_cstr(Inst_Type instType) {
    switch (instType) {
        case INST_NOP:         return "INST_NOP";
        case INST_PUSH:        return "INST_PUSH";
        case INST_PLUS:        return "INST_PLUS";
        case INST_MINUS:       return "INST_MINUS";
        case INST_MULT:        return "INST_MULT";
        case INST_DIV:         return "INST_DIV";
        case INST_JMP:         return "INST_JMP";
        case INST_JMP_IF:      return "INST_JMP_IF";
        case INST_EQ:          return "INST_EQ";
        case INST_HALT:        return "INST_HALT";
        case INST_PRINT_DEBUG: return "INST_DEBUG";
        case INST_DUP:         return "INST_DUP";
        default:
            assert(0 && "inst_as_cstr: Unreachable");
    }
    assert(0 && "inst_as_cstr: Unreachable");
    return "";
}

//////////// SVM Definitions ////////////
ExeptionState svm_execProgram(SVM* svm, int limit)
{
    while (limit != 0 && !svm->halt) {
        ExeptionState err = svm_execInst(svm);
        if (svm->stack_size > SVM_STACK_CAPACITY) {
            return EXEPTION_STACK_OVERFLOW;
        }
        //svm_dumpStack(stdout, svm);
        if (err != EXEPTION_SATE_OK) {
            return err;
        }
        if (limit > 0) {
            --limit;
        }
    }
    return EXEPTION_SATE_OK;
}

ExeptionState svm_execInst(SVM* svm)
{
    if (svm->ip < 0 || svm->ip >= svm->program_size) {
        return EXEPTION_ILLEGAL_INST_ACCESS;
    }

    Inst inst = svm->program[svm->ip];

    switch (inst.type) {
        case INST_NOP: {
            svm->ip += 1;
            break;
        }
        case INST_PUSH: {
            if (svm->stack_size >= SVM_STACK_CAPACITY) {
                return EXEPTION_STACK_OVERFLOW;
            }
            svm->stack[svm->stack_size++] = inst.operand;
            svm->ip += 1;
            break;
        }

        case INST_DUP: {
            if (svm->stack_size > SVM_STACK_CAPACITY) {
                return EXEPTION_STACK_OVERFLOW;
            }
            if (svm->stack_size - inst.operand <= 0) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            if (inst.operand < 0) {
                return EXEPTION_ILLEGAL_OPERAND;
            }
            svm->stack[svm->stack_size] = svm->stack[svm->stack_size - 1 - inst.operand];
            svm->stack_size += 1;
            svm->ip += 1;
            break;
        }

        case INST_PLUS: {
            if (svm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            svm->stack[svm->stack_size - 2] += svm->stack[svm->stack_size - 1];
            svm->stack_size -= 1;
            svm->ip += 1;
            break;
        }

        case INST_MINUS: {
            if (svm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            svm->stack[svm->stack_size - 2] -= svm->stack[svm->stack_size - 1];
            svm->stack_size -= 1;
            svm->ip += 1;
            break;
        }

        case INST_MULT: {
            if (svm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            svm->stack[svm->stack_size - 2] *= svm->stack[svm->stack_size - 1];
            svm->stack_size -= 1;
            svm->ip += 1;
            break;
        }

        case INST_DIV: {
            if (svm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            if (svm->stack[svm->stack_size - 1] == 0) {
                return EXEPTION_DIV_BY_ZERO;
            }
            svm->stack[svm->stack_size - 2] /= svm->stack[svm->stack_size - 1];
            svm->stack_size -= 1;
            svm->ip += 1;
            break;
        }

        case INST_JMP: {
            svm->ip = inst.operand;
            break;
        }

        case INST_HALT: {
            svm->halt = 1;
            break;
        }

        case INST_EQ: {
            if (svm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            svm->stack[svm->stack_size - 2] = (svm->stack[svm->stack_size - 1] == svm->stack[svm->stack_size - 2]);
            svm->stack_size -= 1;
            svm->ip += 1;
            break;
        }

        case INST_JMP_IF: {
            if (svm->stack_size < 1) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            if (svm->stack[svm->stack_size - 1]) {
                svm->stack_size -= 1;
                svm->ip = inst.operand;
            } else {
                svm->ip += 1;
            }
            break;
        }

        case INST_PRINT_DEBUG: {
            if (svm->stack_size < 1) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            printf("%lld\n", svm->stack[svm->stack_size-1]);
            svm->stack_size -= 1;
            svm->ip += 1;
            break;
        }

        default:
            return EXEPTION_ILLEGAL_INST;
    }
    return EXEPTION_SATE_OK;
}

void svm_addInst(SVM* svm, Inst inst)
{
    assert(svm->program_size++ < SVM_PROGRAM_CAPACITY);
    svm->program[svm->program_size++] = inst;
}

void svm_dumpStack(FILE *stream, const SVM* svm)
{
    fprintf(stream, "STACK:\n");
    if (svm->stack_size > 0) {
        for (Word i = 0; i < svm->stack_size; ++i) {
            fprintf(stream, " %lld\n", svm->stack[i]);
        }
    } else {
        fprintf(stream, " [empty]\n");
    }
}

void svm_loadProgramFromMemory(SVM* svm, Inst* program, size_t program_size)
{
    assert(program_size < SVM_PROGRAM_CAPACITY);
    memcpy(svm->program, program, sizeof(program[0]) * program_size);
    svm->program_size = program_size;
}

void svm_saveProgramToFile(const SVM* svm, const char* file_path)
{
    FILE* f = fopen(file_path, "wb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Could not open file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }

    fwrite(svm->program, sizeof(svm->program[0]), svm->program_size, f);
    if (ferror(f)) {
        fprintf(stderr, "ERROR: Could not write to file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }

    fclose(f);
}

void svm_loadProgramFromFile(SVM* svm, const char* file_path)
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

    assert(m % sizeof(svm->program[0]) == 0);
    assert((size_t) m <= SVM_PROGRAM_CAPACITY * sizeof(svm->program[0]));

    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }

    size_t size = fread(svm->program, sizeof(svm->program[0]), m / sizeof(svm->program[0]), f);
    if (ferror(f)) {
        fprintf(stderr, "ERROR: Could not read file '%s'! : %s\n", file_path, strerror(errno));
        exit(1);
    }
    svm->program_size = (int)size;

    fclose(f);
}

void svm_translateSource(StringView source, SVM* svm, LabelTable* lt)
{
    svm->program_size = 0;

    // Pass one
    while (source.count > 0) {
        assert(svm->program_size < SVM_PROGRAM_CAPACITY);
        StringView  line = sv_trim(sv_chopByDelim(&source, '\n'));
        if (line.count > 0 && *line.data != '#') {
            StringView  inst_name = sv_chopByDelim(&line, ' ');
            StringView operand = sv_trim(sv_chopByDelim(&line, '#'));

            if (inst_name.count > 0 && inst_name.data[inst_name.count - 1] == ':') {
                StringView label = (StringView) { .count = inst_name.count - 1, .data = inst_name.data };
                lt_push(lt, label, svm->program_size);

            } else if (sv_eq(inst_name, cstr_as_sv("nop"))) {
                svm->program[svm->program_size++] = (Inst) {
                  .type = INST_NOP
                };
            } else if (sv_eq(inst_name, cstr_as_sv("push"))) {
                svm->program[svm->program_size++] = (Inst) {
                    .type = INST_PUSH,
                    .operand = sv_as_int(operand)
                };
            } else if (sv_eq(inst_name, cstr_as_sv("dup"))) {
                svm->program[svm->program_size++] = (Inst) {
                        .type = INST_DUP,
                        .operand = sv_as_int(operand)
                };
            } else if (sv_eq(inst_name, cstr_as_sv("plus"))) {
                svm->program[svm->program_size++] = (Inst) {
                    .type = INST_PLUS
                };
            } else if (sv_eq(inst_name, cstr_as_sv("jmp"))) {
                if (operand.count > 0 && isdigit(*operand.data)) {
                    svm->program[svm->program_size++] = (Inst) {
                            .type = INST_JMP,
                            .operand = sv_as_int(operand)
                    };
                } else {
                    lt_push_ujmp(lt, svm->program_size, operand);
                    svm->program[svm->program_size++] = (Inst) {
                            .type = INST_JMP
                    };
                }
            } else {
                fprintf(stderr, "ERORR: Unknown instruction '%.*s'!\n", (int)inst_name.count, inst_name.data);
                exit(1);
            }
        }
    }

    // Pass two
    for (size_t i = 0; i < lt->unresolvedJmps_size; ++i) {
        Word addr = lt_find(lt, lt->unresolvedJmps[i].label);
        svm->program[lt->unresolvedJmps[i].addr].operand = addr;
    }
}

//////////// Label Definitions ////////////
Word lt_find(const LabelTable* lt, StringView name)
{
    for (size_t i = 0; i < lt->lables_size; ++i) {
        if (sv_eq(lt->labels[i].name, name)) {
            return lt->labels[i].addr;
        }
    }
    fprintf(stderr, "ERROR: Label '%.*s' does not exist!\n",
            (int)name.count, name.data);
    exit(1);
    return -1;
}

void lt_push(LabelTable* lt, StringView name, Word addr)
{
    assert(lt->lables_size < LABEL_CAPACITY);
    lt->labels[lt->lables_size++] = (Label) { .name = name, .addr = addr };
}

void lt_push_ujmp(LabelTable* lt, Word addr, StringView label)
{
    assert(lt->unresolvedJmps_size < UNRESOLVED_JMP_CAPACITY);
    lt->unresolvedJmps[lt->unresolvedJmps_size++] = (UnresolvedJmp) {
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
#endif //SVM_IMPLEMENTATION