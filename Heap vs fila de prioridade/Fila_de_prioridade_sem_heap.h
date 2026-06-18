#ifndef LEARNING_ALGORITHMS_FILA_DE_PRIORIDADE_SEM_HEAP_H
#define LEARNING_ALGORITHMS_FILA_DE_PRIORIDADE_SEM_HEAP_H

#include "item.h"

typedef struct node {
    struct node *next;
    struct node *prev;
    item_t item;
} node_f;

typedef struct fila {
    int size;
    long long comparacoes;  // Contador de comparações
    struct node *inicio;
    struct node *fim;
} fila_t;

fila_t *fila_criar();
node_f *insert_node_f(fila_t *fila, item_t item);
node_f *remove_node_f(fila_t *fila);
void display_fila(fila_t *fila);
void free_fila(fila_t *fila);
void free_node_f(node_f *node);
void reset_comparacoes_fila(fila_t *fila);

#endif