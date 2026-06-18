#include "Fila_de_prioridade_sem_heap.h"
#include <stdio.h>
#include <stdlib.h>

fila_t *fila_criar() {
    fila_t *fila = (fila_t *)malloc(sizeof(fila_t));
    if (fila) {
        fila->inicio = NULL;
        fila->fim = NULL;
        fila->size = 0;
        fila->comparacoes = 0;
    }
    return fila;
}

node_f *insert_node_f(fila_t *fila, item_t item) {
    node_f *newNode = (node_f *)malloc(sizeof(node_f));
    if (!newNode) {
        fprintf(stderr, "Falha na alocação de memória\n");
        return NULL;
    }

    newNode->item = item;
    newNode->next = NULL;
    newNode->prev = fila->fim;

    if (fila->inicio == NULL) {
        fila->inicio = newNode;
        fila->fim = newNode;
    } else {
        fila->fim->next = newNode;
        fila->fim = newNode;
    }

    fila->size++;
    return newNode;
}

node_f *remove_node_f(fila_t *fila) {
    if (fila == NULL || fila->inicio == NULL) {
        return NULL;
    }

    // Caso especial: apenas um elemento
    if (fila->inicio == fila->fim) {
        node_f *removed = fila->inicio;
        fila->inicio = NULL;
        fila->fim = NULL;
        fila->size = 0;
        return removed;
    }

    node_f *max_priority = fila->inicio;
    node_f *aux = fila->inicio->next;

    // Encontra máximo
    while (aux != NULL) {
        fila->comparacoes++;
        if (aux->item.key > max_priority->item.key) {
            fila->comparacoes++;
            max_priority = aux;
        }
        aux = aux->next;
    }

    // Remove da lista
    if (max_priority->prev != NULL) {
        max_priority->prev->next = max_priority->next;
    } else {
        fila->inicio = max_priority->next;
    }

    if (max_priority->next != NULL) {
        max_priority->next->prev = max_priority->prev;
    } else {
        fila->fim = max_priority->prev;
    }

    max_priority->next = NULL;
    max_priority->prev = NULL;
    fila->size--;

    return max_priority;
}

void display_fila(fila_t *fila) {
    printf("\n\tDisplay Fila:\n");
    if (fila == NULL || fila->inicio == NULL) {
        printf("\tFila Vazia\n");
        return;
    }

    node_f *aux = fila->inicio;
    while (aux != NULL) {
        printf("(key: %d | value: %s) ", aux->item.key, aux->item.value);
        aux = aux->next;
    }
    printf("\n");
}

void free_node_f(node_f *node) {
    if (node == NULL) return;
    if (node->item.value != NULL) {
        free(node->item.value);
    }
    free(node);
}

void free_fila(fila_t *fila) {
    if (fila == NULL) return;
    node_f *current = fila->inicio;
    node_f *next;
    while (current != NULL) {
        next = current->next;
        free_node_f(current);
        current = next;
    }
    free(fila);
}

void reset_comparacoes_fila(fila_t *fila) {
    if (fila) fila->comparacoes = 0;
}