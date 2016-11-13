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
//      n_args  : N�mero de argumentos do n�
//      ...     : Argumentos do n�
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
// Clona um n� do shiro
//      other   : N� que ser� clonado
//-----------------------------------------------------------------------------
shiro_node* use_node(shiro_node* other) {
    other->being_used++;
    return other;
}
//-----------------------------------------------------------------------------
// Limpa um n� do shiro da mem�ria
//      node    : N� a ser limpo da mem�ria
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
// Clona um bin�rio do shiro
//      bin : Bin�rio a ser clonado
//-----------------------------------------------------------------------------
shiro_binary* clone_binary(shiro_binary* bin) {
    shiro_binary* binary = new_binary();

    shiro_uint i;
    for (i = 0; i < bin->used; i++)
        push_node(binary, bin->nodes[i]);

    return binary;
}
//-----------------------------------------------------------------------------
// Adiciona um n� a um bin�rio shiro
//      binary  : Bin�rio onde adicionar o n�
//      node    : N� a ser adicionado
//
// Nota.: O n� adicionado ao bin�rio na verdade � um clone do original, que
// pode ser liberado da mem�ria assim que necess�rio
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
// Combina dois bin�rios em um s�
//      binary  : Bin�rio que receber� a combina��o (e ser� combinado)
//      other   : Outro bin�rio que ser� adicionado ao final do outro
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
// Verifica se um bin�rio retorna alguma coisa
//      binary  : Bin�rio a ser verificado
//-----------------------------------------------------------------------------
bool binary_returns_value(const shiro_binary* binary) {
    int n = 0, i;
    for (i = 0; i < binary->used; i++)
        if ((binary->nodes[i]->code & PUSH) == PUSH ||
            binary->nodes[i]->code == FN_CALL ||
            (binary->nodes[i]->code & OPERATE) == OPERATE)
            n++;
        else if (binary->nodes[i]->code == DROP)
            n--;
    return n > 0;
}
//-----------------------------------------------------------------------------
// Libera a mem�ria usada por um bin�rio do shiro
//      binary  : Bin�rio que ser� liberado da mem�ria
//-----------------------------------------------------------------------------
SHIRO_API void shiro_free_binary(shiro_binary* binary) {
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
    shiro_free_binary(other);

    return binary;
}
