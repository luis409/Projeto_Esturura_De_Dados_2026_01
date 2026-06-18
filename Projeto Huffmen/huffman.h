#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// ============================================================================
// ESTRUTURAS DE DADOS
// ============================================================================

typedef struct HuffmanData {
    unsigned char byte; // Usamos unsigned char para aceitar QUALQUER tipo de arquivo (0 a 255)
    ssize_t frequency;     // Quantas vezes o byte aparece
} HuffmanData;

typedef struct node_arvore{
    struct node_arvore* left; // filho da esquerda
    struct node_arvore* right; // filho da direita
    void* data; // Nesse caso, está servindo como HuffmanData, que armazena 1 byte e a frequencia do byte num arquivo/string
} no_arvore;

typedef struct node_lista{
    struct node_lista* next; // Node da fila de frequencia que indica o proximo da fila de frequencia
    void* item; // Nesse caso, está servindo para armazenar tanto node_arvore quanto as proprias frequencias da fila de frequencia
} NODE;

typedef struct lista{ // Apenas contém o head e o tail da lista, que são nodes da fila de frequencia
    NODE* head;
    NODE* tail;
} LISTA;

// ============================================================================
// PROTÓTIPOS DAS FUNÇÕES (Organizados conforme huffman.c)
// ============================================================================

// 1. INICIALIZAÇÃO DE ESTRUTURAS
no_arvore* create_node_arvore(void* data);
LISTA* create_list();

// 2. CONSTRUÇÃO DA FILA DE FREQUÊNCIA
int criar_frequencia_universal(const char* nome_arquivo, ssize_t mapa_frequencia[]);
void inserir_fila_ordenada(LISTA* fila_de_frequencia, ssize_t mapa_frequencia[]);
void inserir_node_ordenado(LISTA* lista, no_arvore* node_arvore);

// 3. CONSTRUÇÃO DA ÁRVORE DE HUFFMAN
no_arvore* remover_inicio(LISTA* lista);
no_arvore* criar_arvore(LISTA* fila_de_frequencia);
int altura_arvore(no_arvore* raiz);

// 4. DICIONÁRIO DE CODIFICAÇÃO
char** aloca_dicionario(int colunas);
void gerar_dicionario(char** dicionario, no_arvore* raiz, unsigned char* string, int colunas);
void imprime_dicionario(char** dicionario);

// 5. CODIFICAÇÃO E COMPACTAÇÃO
char* codificar(char** dicionario, unsigned char* texto, ssize_t char_lidos, int colunas);
void empacotar_e_escrever(const char* bits_str, FILE* arquivo_saida, unsigned char* bit_buffer, int* bit_count);
void flush_bits(FILE* arquivo_saida, unsigned char* bit_buffer, int* bit_count);
int calcular_tamanho_arvore(no_arvore* raiz);
void salvar_arvore(no_arvore* raiz, FILE* arquivo);

// 6. DECODIFICAÇÃO E DESCOMPACTAÇÃO
no_arvore* reconstruir_arvore(FILE* arquivo, int* tamanho_arvore);
void decodificar();

// 7. UTILITÁRIOS DE SISTEMA
void limpar_ecra();

#endif