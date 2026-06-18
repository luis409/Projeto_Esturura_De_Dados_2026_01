#include "Fila_de_prioridade_com_heap.h"
#include <stdio.h>
#include <stdlib.h>

int parent(int i) {
    return (i - 1) / 2;
}

int left(int i) {
    return 2 * i + 1;
}

int right(int i) {
    return 2 * i + 2;
}

void max_Heapify(heap_t *h, int i) {
    int l = left(i);
    int r = right(i);
    int maior = i;

    // Conta comparações
    if (l < h->size) {
        h->comparacoes++;  // Comparação l < h->size
        if (h->items[l].key > h->items[maior].key) {
            maior = l;
        }
    }

    if (r < h->size) {
        h->comparacoes++;  // Comparação r < h->size
        if (h->items[r].key > h->items[maior].key) {
            maior = r;
        }
    }

    if (maior != i) {
        item_t aux = h->items[maior];
        h->items[maior] = h->items[i];
        h->items[i] = aux;
        max_Heapify(h, maior);
    }
}

void build_max_Heap(heap_t *h) {
    for (int i = h->size / 2 - 1; i >= 0; i--)
        max_Heapify(h, i);
}

heap_t *create_empty_heap() {
    heap_t *h = (heap_t *)malloc(sizeof(heap_t));
    if (h) {
        h->size = 0;
        h->comparacoes = 0;
    }
    return h;
}

void add_item(heap_t *h, item_t item) {
    if (h->size >= MAX_SIZE_HEAP) {
        printf("\n\tHeap já está cheia\n");
        return;
    }

    int i = h->size;
    h->items[i] = item;
    h->size++;

    // Bubble up com contagem
    while (i > 0) {
        h->comparacoes++;  // Comparação i > 0
        int p = parent(i);
        h->comparacoes++;  // Comparação de chaves
        if (h->items[p].key < h->items[i].key) {
            item_t aux = h->items[p];
            h->items[p] = h->items[i];
            h->items[i] = aux;
            i = p;
        } else {
            break;
        }
    }
}

item_t remove_item(heap_t *h) {
    if (h == NULL || h->size == 0) {
        printf("\n\tHeap está vazia\n");
        item_t vazio = {0, NULL};
        return vazio;
    }

    item_t max = h->items[0];
    h->items[0] = h->items[h->size - 1];
    h->size--;
    max_Heapify(h, 0);
    return max;
}

void display_heap(heap_t *h) {
    printf("\n\tDisplay heap:\n");
    for (int i = 0; i < h->size; i++)
        printf("(key: %d | value: %s) ", h->items[i].key, h->items[i].value);
    printf("\n");
}

void free_heap(heap_t *h) {
    if (h == NULL) return;
    for (int i = 0; i < h->size; i++) {
        if (h->items[i].value != NULL) {
            free(h->items[i].value);
        }
    }
    free(h);
}

void reset_comparacoes_heap(heap_t *h) {
    if (h) h->comparacoes = 0;
}
