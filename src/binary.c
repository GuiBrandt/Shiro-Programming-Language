//=============================================================================
// src\binary.c
//-----------------------------------------------------------------------------
// Define as fun��es usadas para manipular as estruturas de representa��o do
// bytecode Shiro
//=============================================================================
#include "parser.h"

#include "vm.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//-----------------------------------------------------------------------------
// Cria um novo n� do shiro
//      code    : C�digo do bytecode do n�
//      ...     : Argumentos do n�
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
// Clona um n� do shiro
//      other   : N� que ser� clonado
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
// Limpa um n� do shiro da mem�ria
//      node    : N� a ser limpo da mem�ria
//-----------------------------------------------------------------------------
void free_node(shiro_node* node) {
    int i;
    for (i = 0; i < node->n_args; i++)
        free_value(node->args[i]);
    free(node->args);
    free(node);
}
//-----------------------------------------------------------------------------
// Cria um novo bin�rio do shiro
//-----------------------------------------------------------------------------
shiro_binary* new_binary(void) {
    shiro_binary* bin = malloc(sizeof(shiro_binary));
    bin->used = 0;
    bin->allocated = 1;
    bin->nodes = calloc(bin->allocated, sizeof(shiro_node*));
    return bin;
}
//-----------------------------------------------------------------------------
// Adiciona um n� a um bin�rio shiro
//      binary  : Bin�rio onde adicionar o n�
//      node    : N� a ser adicionado
//
// Nota.: O n� adicionado ao bin�rio na verdade � um clone do original, que
// pode ser liberado da mem�ria assim que necess�rio
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
// Combina dois bin�rios em um s�
//      binary  : Bin�rio que receber� a combina��o (e ser� combinado)
//      other   : Outro bin�rio que ser� adicionado ao final do outro
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
// Libera a mem�ria usada por um bin�rio do shiro
//      binary  : Bin�rio que ser� liberado da mem�ria
//-----------------------------------------------------------------------------
void free_binary(shiro_binary* binary) {
    int i;
    for (i = 0; i < binary->used; i++)
        free_node(binary->nodes[i]);
    free(binary->nodes);
    free(binary);
}
//-----------------------------------------------------------------------------
// Combina dois bin�rios em um s� e libera o que foi adicionado ao final do
// outro da mem�ria
//      binary  : Bin�rio que receber� a combina��o (e ser� combinado)
//      other   : Outro bin�rio que ser� adicionado ao final do outro e
//                liberado da mem�ria
//-----------------------------------------------------------------------------
shiro_binary* concat_and_free_binary(shiro_binary* binary, shiro_binary* other) {
    concat_binary(binary, other);
    free_binary(other);

    return binary;
}
