//=============================================================================
// src\binary.c
//-----------------------------------------------------------------------------
// Define as funções usadas para manipular as estruturas de representação do
// bytecode Shiro
//=============================================================================
#include "parser.h"

#include "vm.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//-----------------------------------------------------------------------------
// Cria um novo nó do shiro
//      code    : Código do bytecode do nó
//      ...     : Argumentos do nó
//-----------------------------------------------------------------------------
shiro_node* new_node(const shiro_bytecode code, const shiro_uint n_args, ...) {
    shiro_node* node = malloc(sizeof(shiro_node));
    node->code   = code;
    node->n_args = n_args;
    node->args   = calloc(n_args, sizeof(void*));

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
shiro_node* clone_node(const shiro_node* other) {
    shiro_node* node = malloc(sizeof(shiro_node));
    node->code = other->code;
    node->n_args = other->n_args;
    node->args = malloc(node->n_args * sizeof(void*));
    memcpy(node->args, other->args, other->n_args * sizeof(void*));

    return node;
}
//-----------------------------------------------------------------------------
// Limpa um nó do shiro da memória
//      node    : Nó a ser limpo da memória
//-----------------------------------------------------------------------------
void free_node(shiro_node* node) {
    int i;
    for (i = 0; i < node->n_args; i++)
        free_value(node->args[i]);
    free(node->args);
    free(node);
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
// Adiciona um nó a um binário shiro
//      binary  : Binário onde adicionar o nó
//      node    : Nó a ser adicionado
//
// Nota.: O nó adicionado ao binário na verdade é um clone do original, que
// pode ser liberado da memória assim que necessário
//-----------------------------------------------------------------------------
shiro_binary* push_node(shiro_binary* binary, const shiro_node* node) {
    if (binary == SHIRO_NIL || node == SHIRO_NIL)
        return binary;

    if (binary->used >= binary->allocated) {
        binary->allocated *= 2;
        binary->nodes = realloc(
            binary->nodes,
            sizeof(shiro_node*) * binary->allocated
        );
    }

    binary->nodes[binary->used++] = clone_node(node);

    return binary;
}
//-----------------------------------------------------------------------------
// Combina dois binários em um só
//      binary  : Binário que receberá a combinação (e será combinado)
//      other   : Outro binário que será adicionado ao final do outro
//------------------------------------------------- ----------------------------
shiro_binary* concat_binary(shiro_binary* binary, const shiro_binary* other) {
    if (binary == SHIRO_NIL || other == SHIRO_NIL)
        return binary;

    int i;
    for (i = 0; i < other->used; i++)
        push_node(binary, other->nodes[i]);

    return binary;
}
//-----------------------------------------------------------------------------
// Libera a memória usada por um binário do shiro
//      binary  : Binário que será liberado da memória
//-----------------------------------------------------------------------------
void free_binary(shiro_binary* binary) {
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
    free_binary(other);

    return binary;
}
