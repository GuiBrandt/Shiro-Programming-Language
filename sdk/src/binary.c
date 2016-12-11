//=============================================================================
// src\binary.c
//-----------------------------------------------------------------------------
// Define as funções usadas para manipular as estruturas de representação do
// bytecode Shiro
//=============================================================================
#include "parser.h"

#include "vm.h"
#include "errors.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//-----------------------------------------------------------------------------
// Cria um novo nó do shiro
//      code    : Código do bytecode do nó
//      n_args  : Número de argumentos do nó
//      ...     : Argumentos do nó
//-----------------------------------------------------------------------------
shiro_node* new_node(const shiro_bytecode code, const shiro_uint n_args, ...) {
    shiro_node* node = malloc(sizeof(shiro_node));
    node->code   = code;
    node->n_args = n_args;
    node->args   = calloc(n_args, sizeof(void*));
    node->being_used = 1;

    va_list args;
    va_start(args, n_args);

    int i;
    for (i = 0; i < n_args; i++) {
        shiro_value* arg = va_arg(args, shiro_value*);
        node->args[i] = arg;
    }

    va_end(args);

    return node;
}
//-----------------------------------------------------------------------------
// Clona um nó do shiro
//      other   : Nó que será clonado
//-----------------------------------------------------------------------------
shiro_node* use_node(shiro_node* other) {
    other->being_used++;
    return other;
}
//-----------------------------------------------------------------------------
// Limpa um nó do shiro da memória
//      node    : Nó a ser limpo da memória
//-----------------------------------------------------------------------------
void free_node(shiro_node* node) {
    if (--node->being_used > 0)
        return;

    int i;
    for (i = 0; i < node->n_args; i++)
        shiro_free_value(node->args[i]);
    free(node->args);
    free(node);
}
//-----------------------------------------------------------------------------
// Retorna o número de valores que um nó põe no stack
//-----------------------------------------------------------------------------
shiro_int node_change_stack(const shiro_node* node) {
    switch (node->code) {
        case COND:
        case DROP:
        case SET_VAR:
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case MOD:
        case B_AND:
        case B_OR:
        case B_XOR:
        case COMPARE_EQ:
        case COMPARE_GT:
        case COMPARE_LT:
        case COMPARE_GT_EQ:
        case COMPARE_LT_EQ:
        case RETURN:
            return -1;
        case FN_CALL:
            return -(shiro_int)get_uint(node->args[1]) + 1;
        case PUSH:
        case PUSH_BY_NAME:
            return 1;
        default:
            return 0;
    }
}
//-----------------------------------------------------------------------------
// Cria um novo binário do shiro
//-----------------------------------------------------------------------------
shiro_binary* new_binary(void) {
    shiro_binary* bin = malloc(sizeof(shiro_binary));
    bin->used = 0;
    bin->allocated = 1;
    bin->nodes = calloc(bin->allocated, sizeof(shiro_node*));
    return bin;
}
//-----------------------------------------------------------------------------
// Clona um binário do shiro
//      bin : Binário a ser clonado
//-----------------------------------------------------------------------------
shiro_binary* clone_binary(shiro_binary* bin) {
    shiro_binary* binary = new_binary();

    shiro_uint i;
    for (i = 0; i < bin->used; i++)
        push_node(binary, bin->nodes[i]);

    return binary;
}
//-----------------------------------------------------------------------------
// Adiciona um nó a um binário shiro
//      binary  : Binário onde adicionar o nó
//      node    : Nó a ser adicionado
//
// Nota.: O nó adicionado ao binário na verdade é um clone do original, que
// pode ser liberado da memória assim que necessário
//-----------------------------------------------------------------------------
shiro_binary* push_node(shiro_binary* binary, shiro_node* node) {
    if (binary == NULL || node == NULL)
        return binary;

    if (binary->used >= binary->allocated) {
        binary->allocated *= 2;
        binary->nodes = realloc(
            binary->nodes,
            sizeof(shiro_node*) * binary->allocated
        );
    }

    binary->nodes[binary->used++] = use_node(node);

    return binary;
}
//-----------------------------------------------------------------------------
// Combina dois binários em um só
//      binary  : Binário que receberá a combinação (e será combinado)
//      other   : Outro binário que será adicionado ao final do outro
//-----------------------------------------------------------------------------
shiro_binary* concat_binary(shiro_binary* binary, const shiro_binary* other) {
    if (binary == NULL || other == NULL)
        return binary;

    int i;
    for (i = 0; i < other->used; i++)
        push_node(binary, other->nodes[i]);

    return binary;
}
//-----------------------------------------------------------------------------
// Verifica se um binário retorna alguma coisa
//      binary  : Binário a ser verificado
//-----------------------------------------------------------------------------
bool binary_returns_value(const shiro_binary* binary) {
    int n = 0, i;
    for (i = 0; i < binary->used; i++) {
        shiro_node* node = binary->nodes[i];

        if (node->code == RETURN)
            return true;
        else
            n += node_change_stack(node);
    }
    return n > 0;
}
//-----------------------------------------------------------------------------
// Escreve um binário compilado do shiro para um arquivo
//-----------------------------------------------------------------------------
SHIRO_API void shiro_write_binary(FILE* file, shiro_binary* binary) {
    int magic = 1;
    fwrite(&magic, 1, sizeof(magic), file);

    fwrite(&binary->used, 1, sizeof(shiro_uint), file);

    shiro_uint i;
    for (i = 0; i < binary->used; i++) {
        shiro_node* node = binary->nodes[i];

        shiro_uint n_args = node->n_args;

        char code = (char)node->code;
        fwrite(&code, 1, sizeof(code), file);
        fwrite(&n_args, 1, sizeof(shiro_uint), file);

        shiro_uint j;
        for (j = 0; j < n_args; j++) {
            shiro_value* v = node->args[j];

            char type = (char)v->type;
            fwrite(&type, 1, sizeof(type), file);
            switch (v->type) {
                case s_tInt:
                    fwrite(&get_int(v), 1, sizeof(get_int(v)), file);
                    break;
                case s_tFloat:
                    fwrite(&get_float(v), 1, sizeof(get_float(v)), file);
                    break;
                case s_tString:
                {
                    shiro_uint len = shiro_get_field(v, ID("length"))->value.u;
                    fwrite(&len, 1, sizeof(len), file);
                    fwrite(get_string(v), len, sizeof(shiro_character), file);
                    break;
                }
                case s_tFunction:
                {
                    shiro_function* fn = get_func(v);
                    fwrite(&fn->n_args, 1, sizeof(shiro_uint), file);
                    if (fn->type == s_fnShiroBinary) {
                        shiro_write_binary(file, fn->s_binary);
                        break;
                    }
                }
                default:
                    shiro_error(0, "CompileError", "Requested to write an invalid node");
                    break;
            }
        }
    }

}
//-----------------------------------------------------------------------------
// Lê um binário compilado do shiro de um arquivo
//-----------------------------------------------------------------------------
SHIRO_API shiro_binary* shiro_read_binary(FILE* file) {
    int magic = 0;
    fread(&magic, 1, sizeof(magic), file);

    if (magic != 1) {
        shiro_error(0, "CompileError", "Invalid compiled file");
        return NULL;
    }

    shiro_binary* binary = malloc(sizeof(shiro_binary));

    shiro_uint sz = 0;
    fread(&sz, 1, sizeof(shiro_uint), file);

    binary->allocated = binary->used = sz;
    binary->nodes = calloc(sz, sizeof(shiro_node*));

    shiro_uint i;
    for (i = 0; i < sz; i++) {
        shiro_node* node = malloc(sizeof(shiro_node));
        node->being_used = 1;

        char code = 0;
        fread(&code, 1, 1, file);
        node->code = (shiro_bytecode)code;

        fread(&node->n_args, 1, sizeof(shiro_uint), file);

        node->args = calloc(node->n_args, sizeof(shiro_value*));

        shiro_uint j;
        for (j = 0; j < node->n_args; j++) {
            shiro_value* v;

            char type = 0;
            fread(&type, 1, 1, file);
            shiro_type t = (shiro_type)type;

            switch (t) {
            case s_tInt:
            {
                shiro_int fix = 0;
                fread(&fix, 1, sizeof(shiro_int), file);
                v = shiro_new_int(fix);
                break;
            }
            case s_tFloat:
            {
                shiro_float f = 0.0;
                fread(&f, 1, sizeof(shiro_float), file);
                v = shiro_new_float(f);
                break;
            }
            case s_tString:
            {
                shiro_uint len = 0;
                fread(&len, 1, sizeof(len), file);

                shiro_string str = malloc(len * sizeof(shiro_character) + 1);
                fread(str, len, sizeof(shiro_character), file);
                str[len] = 0;

                v = malloc(sizeof(shiro_value));
                shiro_value val = {s_tString, 2, NULL, 1};
                memcpy(v, &val, sizeof(val));
                v->fields = calloc(2, sizeof(shiro_field*));

                shiro_set_field(v, ID_VALUE, s_fString, (union __field_value)str);
                shiro_set_field(v, ID("length"), s_fUInt, (union __field_value)len);
                break;
            }
            case s_tFunction:
            {
                shiro_uint n_args = 0;
                fread(&n_args, 1, sizeof(shiro_uint), file);
                shiro_binary* bin = shiro_read_binary(file);
                shiro_function* fn = shiro_new_fn(n_args, bin);
                v = shiro_new_function(fn);
                break;
            }
            default:
                shiro_error(0, "CompileError", "Invalid compiled file");
                return NULL;
            }

            node->args[j] = v;
        }

        binary->nodes[i] = node;
    }

    return binary;
}
//-----------------------------------------------------------------------------
// Libera a memória usada por um binário do shiro
//      binary  : Binário que será liberado da memória
//-----------------------------------------------------------------------------
SHIRO_API void shiro_free_binary(shiro_binary* binary) {
    int i;
    for (i = 0; i < binary->used; i++)
        free_node(binary->nodes[i]);
    free(binary->nodes);
    free(binary);
}
//-----------------------------------------------------------------------------
// Combina dois binários em um só e libera o que foi adicionado ao final do
// outro da memória
//      binary  : Binário que receberá a combinação (e será combinado)
//      other   : Outro binário que será adicionado ao final do outro e
//                liberado da memória
//-----------------------------------------------------------------------------
shiro_binary* concat_and_free_binary(shiro_binary* binary, shiro_binary* other) {
    concat_binary(binary, other);
    shiro_free_binary(other);

    return binary;
}
