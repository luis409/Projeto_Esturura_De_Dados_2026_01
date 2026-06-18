#ifndef LEARNING_ALGORITHMS_FILA_DE_PRIORIDADE_COM_HEAP_H
#define LEARNING_ALGORITHMS_FILA_DE_PRIORIDADE_COM_HEAP_H

#define MAX_SIZE_HEAP 1000
#include "item.h"

typedef struct heap {
    item_t items[MAX_SIZE_HEAP];
    int size;
    long long comparacoes;  // Contador de comparações
} heap_t;

// Funções auxiliares
int parent(int i);
int left(int i);
int right(int i);

// Operações do heap
void max_Heapify(heap_t *h, int i);
void build_max_Heap(heap_t *h);
heap_t *create_empty_heap();
void add_item(heap_t *h, item_t item);
item_t remove_item(heap_t *h);
void display_heap(heap_t *h);
void free_heap(heap_t *h);
void reset_comparacoes_heap(heap_t *h);

#endif