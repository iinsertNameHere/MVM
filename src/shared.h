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
#include <inttypes.h>

#define PRIsv ".*s"
#define SV_FORMAT(sv) (int) sv.count, sv.data

#define MASM_LABEL_CAPACITY 1024
#define MASM_DEFERRED_OPERANDS_CAPACITY 1024
#define MASM_MAX_INCLUDES 42
#define MASM_MEMORY_CAPACITY (1000 * 1000 * 1000) // 1GB
#define MASM_COMMENT_SYMBOL ';'
#define MASM_PP_SYMBOL '%'

#define MVM_STACK_CAPACITY 942 //TODO: Fix stack-underflow if lager than 942.
#define MVM_PROGRAM_CAPACITY 1024
#define MVM_NATIVES_CAPACITY 1024
#define MVM_MEMORY_CAPACITY (640 * 1000) // 640 KB

typedef enum {false, true} bool;

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
StringView sv_ltrim(StringView sv);
StringView sv_rtrim(StringView sv);
StringView sv_trim(StringView sv);
StringView sv_chopByDelim(StringView* sv, char delim);
bool sv_eq(StringView a, StringView b);

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

typedef uint64_t InstAddr;

typedef enum {
    INST_NOP = 0,

    INST_PUSH,
    INST_DUP,
    INST_SWAP,
    INST_DROP,

    INST_PLUSI,
    INST_MINUSI,
    INST_MULTI,
    INST_DIVI,
    INST_PLUSF,
    INST_MINUSF,
    INST_MULTF,
    INST_DIVF,

    INST_ANDB,
    INST_ORB,
    INST_XOR,
    INST_NOTB,
    INST_SHR,
    INST_SHL,

    INST_JMP,
    INST_JMPIF,
    INST_CALL,
    INST_INT,
    INST_RET,

    INST_EQ,
    INST_NOT,
    INST_GEF,
    INST_GEI,

    INST_HALT,

    INST_READ8,
    INST_READ16,
    INST_READ32,
    INST_READ64,
    INST_WRITE8,
    INST_WRITE16,
    INST_WRITE32,
    INST_WRITE64,

    NUMBER_OF_INSTS
} InstType;

const char* InstName(InstType instType);
bool GetInstName(StringView name, InstType* out);
bool InstHasOperand(InstType instType);

typedef struct {
    InstType type;
    Word operand;
} Inst;

typedef struct {
    StringView name;
    Word word;
} Label;

typedef struct {
    StringView label;
    InstAddr addr;
} DeferredOperand;

typedef struct {
    Label labels[MASM_LABEL_CAPACITY];
    size_t lables_size;
    DeferredOperand deferredOperands[MASM_DEFERRED_OPERANDS_CAPACITY];
    size_t deferredOperands_size;
    char memory[MASM_MEMORY_CAPACITY];
    size_t memory_size;
} Masm;

void* masm_alloc(Masm* masm, size_t size);
bool masm_resolveLabel(const Masm* masm, StringView name, Word* out);
bool masm_bindLabel(Masm* masm, StringView name, Word word);
void masm_pushDeferredOperand(Masm* masm, InstAddr addr, StringView label);
StringView masm_slurpFile(Masm* masm, StringView file_path);
bool masm_numberLiteral_as_Word (StringView sv, Word* out);

typedef struct MVM MVM;

typedef ExeptionState (*MvmNative)(MVM*);

struct MVM {
    Word stack[MVM_STACK_CAPACITY];
    uint64_t stack_size;

    Inst program[MVM_PROGRAM_CAPACITY];
    uint64_t program_size;
    InstAddr ip;

    MvmNative natives[MVM_NATIVES_CAPACITY];
    size_t natives_size;

    uint8_t memory[MVM_MEMORY_CAPACITY];

    bool halt;
};

void mvm_pushNative(MVM* mvm, MvmNative);
void mvm_dumpStack(FILE *stream, const MVM* mvm);
void mvm_saveProgramToFile(const MVM* mvm, const char* file_path);
void mvm_loadProgramFromFile(MVM* mvm, const char* file_path);
void mvm_translateSourceFile(MVM* mvm, Masm* masm, StringView inputFile, size_t level);
ExeptionState mvm_execInst(MVM* mvm);
ExeptionState mvm_execProgram(MVM* mvm, int limit);

////////////////////////////////////////////
ExeptionState native_alloc(MVM* mvm);
ExeptionState native_free (MVM* mvm);
ExeptionState native_print_char (MVM* mvm);
ExeptionState native_print_f64 (MVM* mvm);
ExeptionState native_print_i64 (MVM* mvm);
ExeptionState native_print_u64(MVM* mvm);
ExeptionState native_print_ptr(MVM* mvm);
////////////////////////////////////////////


char* shift(int* argc, char*** argv);
#endif //MVM_SHARED_H

#ifdef MVM_SHARED_IMPLEMENTATION

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

bool sv_eq(StringView a, StringView b)
{
    if (a.count != b.count) {
        return false;
    }
    else {
        return memcmp(a.data, b.data, a.count) == 0;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////////////

const char* InstName(InstType instType)
{
    switch (instType) {
        case INST_NOP:    return "nop";
        case INST_PUSH:   return "push";
        case INST_DUP:    return "dup";
        case INST_SWAP:   return "swap";
        case INST_DROP:   return "drop";
        case INST_PLUSI:  return "plusi";
        case INST_MINUSI: return "minusi";
        case INST_MULTI:  return "multi";
        case INST_DIVI:   return "divi";
        case INST_PLUSF:  return "plusf";
        case INST_MINUSF: return "minusf";
        case INST_MULTF:  return "multf";
        case INST_DIVF:   return "divf";
        case INST_ANDB:   return "andb";
        case INST_ORB:    return "orb";
        case INST_XOR:    return "xor";
        case INST_NOTB:   return "notb";
        case INST_SHR:    return "shr";
        case INST_SHL:    return "shl";
        case INST_JMP:    return "jmp";
        case INST_JMPIF:  return "jmpif";
        case INST_CALL:   return "call";
        case INST_INT:    return "int";
        case INST_RET:    return "ret";
        case INST_EQ:     return "equal";
        case INST_NOT:    return "not";
        case INST_GEF:    return "geeqf";
        case INST_GEI:    return "geeqi";
        case INST_HALT:   return "hlt";
        case NUMBER_OF_INSTS:
        default:
            assert(0 && "InstName: Unreachable");
    }
    assert(0 && "InstName: Unreachable");
    return "";
}

bool GetInstName(StringView name, InstType* out)
{
    for (InstType type = (InstType)0; type < NUMBER_OF_INSTS; type += 1) {
        if (sv_eq(cstr_as_sv(InstName(type)), name)) {
            *out = type;
            return true;
        }
    }
    return false;
}

bool InstHasOperand(InstType instType)
{
    switch (instType) {
        case INST_NOP:    return false;
        case INST_PUSH:   return true;
        case INST_DUP:    return true;
        case INST_SWAP:   return true;
        case INST_DROP:   return false;
        case INST_PLUSI:  return false;
        case INST_MINUSI: return false;
        case INST_MULTI:  return false;
        case INST_DIVI:   return false;
        case INST_ANDB:   return false;
        case INST_ORB:    return false;
        case INST_XOR:    return false;
        case INST_NOTB:   return false;
        case INST_SHR:    return false;
        case INST_SHL:    return false;
        case INST_PLUSF:  return false;
        case INST_MINUSF: return false;
        case INST_MULTF:  return false;
        case INST_DIVF:   return false;
        case INST_JMP:    return true;
        case INST_JMPIF:  return true;
        case INST_CALL:   return true;
        case INST_INT:    return true;
        case INST_RET:    return false;
        case INST_EQ:     return false;
        case INST_NOT:    return false;
        case INST_GEF:    return false;
        case INST_GEI:    return false;
        case INST_HALT:   return false;
        case NUMBER_OF_INSTS:
        default:
            assert(0 && "InstHasOperand: Unreachable");
    }
    assert(0 && "InstHasOperand: Unreachable");
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void* masm_alloc(Masm* masm, size_t size)
{
    assert(masm->memory_size + size <= MASM_MEMORY_CAPACITY);
    void* ptr = masm->memory + masm->memory_size;
    masm->memory_size += size;
    return ptr;
}

bool masm_resolveLabel(const Masm* masm, StringView name, Word* out)
{
    for (size_t i = 0; i < masm->lables_size; ++i) {
        if (sv_eq(masm->labels[i].name, name)) {
            *out = masm->labels[i].word;
            return true;
        }
    }
    return false;
}

bool masm_bindLabel(Masm* masm, StringView name, Word word)
{
    assert(masm->lables_size < MASM_LABEL_CAPACITY);
    Word ignore =  {0};
    if (!masm_resolveLabel(masm, name, &ignore)) {
        masm->labels[masm->lables_size++] = (Label) {.name = name, .word = word};
        return true;
    }
    return false;
}

void masm_pushDeferredOperand(Masm* masm, InstAddr addr, StringView label)
{
    assert(masm->deferredOperands_size < MASM_DEFERRED_OPERANDS_CAPACITY);
    masm->deferredOperands[masm->deferredOperands_size++] = (DeferredOperand) {
            .addr = addr,
            .label = label
    };
}

StringView masm_slurpFile(Masm* masm, StringView filePath)
{
    char* filePath_cstr = masm_alloc(masm, filePath.count + 1);
    if (filePath_cstr == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory for file-path! : %s\n", strerror(errno));
        exit(1);
    }
    memcpy(filePath_cstr, filePath.data, filePath.count);
    filePath_cstr[filePath.count] = '\0';

    FILE* f = fopen(filePath_cstr, "r");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Could not open file '%s'! : %s\n", filePath_cstr, strerror(errno));
        exit(1);
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s'! : %s\n", filePath_cstr, strerror(errno));
        exit(1);
    }

    long m = ftell(f);
    if (m < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s'! : %s\n", filePath_cstr, strerror(errno));
        exit(1);
    }

    char* buffer = masm_alloc(masm, m);
    if (buffer == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory for file! : %s\n", strerror(errno));
        exit(1);
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: Could not read file '%s'! : %s\n", filePath_cstr, strerror(errno));
        exit(1);
    }

    size_t n = fread(buffer, 1, m, f);
    if (ferror(f)) {
        fprintf(stderr, "ERROR: Could not read file '%s'! : %s\n", filePath_cstr, strerror(errno));
        exit(1);
    }

    fclose(f);

    return (StringView) {
            .count = n,
            .data = buffer,
    };
}

bool masm_numberLiteral_as_Word (StringView sv, Word* out)
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
            return false;
        }
    }
    *out = result;
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void mvm_pushNative(MVM* mvm, MvmNative native)
{
    assert(mvm->natives_size < MVM_NATIVES_CAPACITY);
    mvm->natives[mvm->natives_size++] = native;
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

void mvm_translateSourceFile(MVM* mvm, Masm* masm, StringView inputFile, size_t level)
{
    StringView source_original = masm_slurpFile(masm, inputFile);
    StringView source = source_original;
    mvm->program_size = 0;

    // Pass one
    int lineNum = 0;
    while (source.count > 0) {
        assert(mvm->program_size < MVM_PROGRAM_CAPACITY);
        StringView  line = sv_trim(sv_chopByDelim(&source, '\n'));
        lineNum += 1;
        if (line.count > 0 && *line.data != MASM_COMMENT_SYMBOL) {
            StringView  token = sv_chopByDelim(&line, ' ');

            // Preprocessor
            if (token.count > 0 && *token.data == MASM_PP_SYMBOL) {
                token.count -= 1;
                token.data += 1;
                if (sv_eq(token, cstr_as_sv("define"))) {
                    line = sv_trim(line);
                    StringView label = sv_chopByDelim(&line, ' ');

                    if (label.count > 0) {
                        line = sv_trim(line);
                        StringView value = sv_chopByDelim(&line, ' ');
                        Word word = {0};
                        if (!masm_numberLiteral_as_Word(value, &word)) {
                            fprintf(stderr, "%" PRIsv ":%d: ERORR: '%" PRIsv "' is not a number!\n", SV_FORMAT(inputFile), lineNum,
                                    (int) value.count, value.data);
                            exit(1);
                        }

                        if (!masm_bindLabel(masm, label, word)) {
                            fprintf(stderr, "%" PRIsv ":%d: ERORR: '%" PRIsv "' is already defined!!\n", SV_FORMAT(inputFile), lineNum,
                                    (int) label.count, label.data);
                            exit(1);
                        }
                    } else {
                        fprintf(stderr, "%" PRIsv ":%d: ERORR: Definition name expected!\n", SV_FORMAT(inputFile), lineNum);
                        exit(1);
                    }
                } else if (sv_eq(token, cstr_as_sv("include"))) {
                    line = sv_trim(line);
                    if (line.count > 0) {
                        if (*line.data == '"' && line.data[line.count - 1] == '"') {
                            line.data += 1;
                            line.count -= 2;

                            if (level + 1 < MASM_MAX_INCLUDES) {
                                mvm_translateSourceFile(mvm, masm, line, level + 1);
                            } else {
                                fprintf(stderr, "%" PRIsv ":%d: ERORR: Exceeded maximum-include-level!\n", SV_FORMAT(inputFile), lineNum);
                                exit(1);
                            }
                        } else {
                            fprintf(stderr, "%" PRIsv ":%d: ERORR: Include-Path has to be surrounded with quotation marks!\n", SV_FORMAT(inputFile), lineNum);
                            exit(1);
                        }
                    } else {
                        fprintf(stderr, "%" PRIsv ":%d: ERORR: Include-Path is not provided!\n", SV_FORMAT(inputFile), lineNum);
                        exit(1);
                    }
                } else {

                }
            } else {

                if (token.count > 0 && token.data[token.count - 1] == ':') {
                    StringView label = (StringView) {.count = token.count - 1, .data = token.data};
                    if (!masm_bindLabel(masm, label, (Word) { .as_u64 = mvm->program_size })) {
                        fprintf(stderr, "%" PRIsv ":%d: ERORR: '%" PRIsv "' is already defined!\n", SV_FORMAT(inputFile), lineNum, SV_FORMAT(label));
                        exit(1);
                    }
                    token = sv_trim(sv_chopByDelim(&line, ' '));
                }

                if (token.count > 0) {
                    StringView operand = sv_trim(sv_chopByDelim(&line, MASM_COMMENT_SYMBOL));
                    InstType instType = INST_NOP;

                    if (GetInstName(token, &instType)) {
                        mvm->program[mvm->program_size].type = instType;
                        if (InstHasOperand(instType)) {
                            if (operand.count == 0) {
                                fprintf(stderr, "%" PRIsv ":%d: ERORR: instruction '%" PRIsv "' expects an operand!\n",
                                        SV_FORMAT(inputFile), lineNum, SV_FORMAT(token));
                                exit(1);
                            }

                            if (!masm_numberLiteral_as_Word(
                                    operand,
                                    &mvm->program[mvm->program_size].operand)) {
                                masm_pushDeferredOperand(masm, mvm->program_size, operand);
                            }

                        }
                        mvm->program_size += 1;
                    } else {
                        fprintf(stderr, "%" PRIsv ":%d: ERORR: Unknown instruction '%" PRIsv "'!\n", SV_FORMAT(inputFile), lineNum,
                                (int) token.count, token.data);
                        exit(1);
                    }
                }
            }
        }
    }

    // Pass two
    for (size_t i = 0; i < masm->deferredOperands_size; ++i) {
        StringView label = masm->deferredOperands[i].label;
        Word* operand = &mvm->program[masm->deferredOperands[i].addr].operand;
        if (!masm_resolveLabel(masm, label, operand)) {
            fprintf(stderr, "%" PRIsv ":%d: ERORR: '%" PRIsv "' is not defined!\n", SV_FORMAT(inputFile), lineNum, SV_FORMAT(label));
            exit(1);
        }
    }
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
            if (mvm->stack_size >= MVM_STACK_CAPACITY) {
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

        case INST_DROP: {
            if (mvm->stack_size < 1) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack_size -= 1;
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

        case INST_ANDB: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 2].as_u64 = mvm->stack[mvm->stack_size - 2].as_u64 & mvm->stack[mvm->stack_size - 1].as_u64;
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_ORB: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 2].as_u64 = mvm->stack[mvm->stack_size - 2].as_u64 | mvm->stack[mvm->stack_size - 1].as_u64;
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_XOR: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 2].as_u64 = mvm->stack[mvm->stack_size - 2].as_u64 ^ mvm->stack[mvm->stack_size - 1].as_u64;
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_NOTB: {
            if (mvm->stack_size < 1) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 1].as_u64 = ~mvm->stack[mvm->stack_size - 1].as_u64;
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_SHR: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 2].as_u64 = mvm->stack[mvm->stack_size - 2].as_u64 >> mvm->stack[mvm->stack_size - 1].as_u64;
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_SHL: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 2].as_u64 = mvm->stack[mvm->stack_size - 2].as_u64 << mvm->stack[mvm->stack_size - 1].as_u64;
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case INST_JMP: {
            mvm->ip = inst.operand.as_u64;
            break;
        }

        case INST_JMPIF: {
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

        case INST_CALL: {
            if (mvm->stack_size >= MVM_STACK_CAPACITY) {
                return EXEPTION_STACK_OVERFLOW;
            }
            mvm->stack[mvm->stack_size++].as_u64 = mvm->ip + 1;
            mvm->ip = inst.operand.as_u64;
            break;
        }

        case INST_INT: {
            if (inst.operand.as_u64 > mvm->natives_size) {
                return EXEPTION_ILLEGAL_OPERAND;
            }
            mvm->natives[inst.operand.as_u64](mvm);
            mvm->ip += 1;
            break;
        }

        case INST_RET: {
            if (mvm->stack_size < 1) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->ip = mvm->stack[mvm->stack_size - 1].as_u64;
            mvm->stack_size -= 1;
            break;
        }

        case INST_HALT: {
            mvm->halt = true;
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

        case INST_GEI: {
            if (mvm->stack_size < 2) {
                return EXEPTION_STACK_UNDERFLOW;
            }
            mvm->stack[mvm->stack_size - 2].as_u64 = (mvm->stack[mvm->stack_size - 1].as_u64 >= mvm->stack[mvm->stack_size - 2].as_u64);
            mvm->stack_size -= 1;
            mvm->ip += 1;
            break;
        }

        case NUMBER_OF_INSTS:
        default:
            return EXEPTION_ILLEGAL_INST;
    }
    return EXEPTION_SATE_OK;
}

ExeptionState mvm_execProgram(MVM* mvm, int limit)
{
    while (limit != 0 && !mvm->halt) {
        ExeptionState err = mvm_execInst(mvm);
        if (mvm->stack_size > MVM_STACK_CAPACITY) {
            return EXEPTION_STACK_OVERFLOW;
        }
        if (err != EXEPTION_SATE_OK) {
            return err;
        }
        if (limit > 0) {
            --limit;
        }
    }
    return EXEPTION_SATE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

ExeptionState native_alloc(MVM* mvm)
{
    if (mvm->stack_size < 1) {
        return EXEPTION_STACK_UNDERFLOW;
    }

    mvm->stack[mvm->stack_size - 1].as_ptr = malloc(mvm->stack[mvm->stack_size - 1].as_u64);
    return EXEPTION_SATE_OK;
}

ExeptionState native_free(MVM* mvm)
{
    if (mvm->stack_size < 1) {
        return EXEPTION_STACK_UNDERFLOW;
    }

    free(mvm->stack[mvm->stack_size - 1].as_ptr);
    mvm->stack_size -= 1;
    return  EXEPTION_SATE_OK;
}

ExeptionState native_print_char(MVM* mvm)
{
    if (mvm->stack_size < 1) {
        return EXEPTION_STACK_UNDERFLOW;
    }

    if (mvm->stack[mvm->stack_size - 1].as_u64 != 13) {
        printf("%c", (int) mvm->stack[mvm->stack_size - 1].as_u64);
    } else {
        printf("\n");
    }
    mvm->stack_size -= 1;
    return  EXEPTION_SATE_OK;
}

ExeptionState native_print_f64(MVM* mvm)
{
    if (mvm->stack_size < 1) {
        return EXEPTION_STACK_UNDERFLOW;
    }

    printf("%lf", mvm->stack[mvm->stack_size - 1].as_f64);
    mvm->stack_size -= 1;
    return  EXEPTION_SATE_OK;
}

ExeptionState native_print_i64(MVM* mvm)
{
    if (mvm->stack_size < 1) {
        return EXEPTION_STACK_UNDERFLOW;
    }

    printf("%" PRId64, mvm->stack[mvm->stack_size - 1].as_i64);
    mvm->stack_size -= 1;
    return  EXEPTION_SATE_OK;
}

ExeptionState native_print_u64(MVM* mvm)
{
    if (mvm->stack_size < 1) {
        return EXEPTION_STACK_UNDERFLOW;
    }

    printf("%" PRIu64, mvm->stack[mvm->stack_size - 1].as_u64);
    mvm->stack_size -= 1;
    return  EXEPTION_SATE_OK;
}

ExeptionState native_print_ptr(MVM* mvm)
{
    if (mvm->stack_size < 1) {
        return EXEPTION_STACK_UNDERFLOW;
    }

    printf("0x%llx", (uintptr_t) mvm->stack[mvm->stack_size - 1].as_ptr);
    mvm->stack_size -= 1;
    return  EXEPTION_SATE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

char* shift(int* argc, char*** argv)
{
    assert(*argc > 0);
    char* res = **argv;
    *argv += 1;
    *argc -= 1;
    return res;
}
#endif //MVM_SHARED_IMPLEMENTATION