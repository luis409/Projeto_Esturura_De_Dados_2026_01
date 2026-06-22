#include "Fila_de_prioridade_com_heap.h"
#include "Fila_de_prioridade_sem_heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_SIMULACOES 30
#define MAX_LINHA 100

// Estrutura para armazenar resultados
typedef struct {
    long long heap_comparacoes[NUM_SIMULACOES];
    long long fila_comparacoes[NUM_SIMULACOES];
    int tamanhos[NUM_SIMULACOES];
} resultados_t;

// Gera dados aleatórios
void gerar_dados_aleatorios(int num_linhas) {
    FILE *arq = fopen("processos.txt", "w");
    if (!arq) {
        fprintf(stderr, "Erro ao criar arquivo\n");
        return;
    }

    srand(time(NULL));
    for (int i = 0; i < num_linhas; i++) {
        int prioridade = rand() % 6;
        char parte1[4], parte2[4], parte3[4], parte4[4];

        for (int j = 0; j < 3; j++) {
            parte1[j] = 'A' + (rand() % 26);
            parte2[j] = '0' + (rand() % 10);
            parte3[j] = 'A' + (rand() % 26);
            parte4[j] = '0' + (rand() % 10);
        }
        parte1[3] = parte2[3] = parte3[3] = parte4[3] = '\0';

        fprintf(arq, "%d/%s-%s-%s-%s\n", prioridade, parte1, parte2, parte3, parte4);
    }
    fclose(arq);
}

// Preenche estruturas a partir do arquivo
void preencher_estruturas(fila_t *f, heap_t *h) {
    FILE *arq = fopen("processos.txt", "r");
    if (!arq) {
        fprintf(stderr, "Erro ao abrir arquivo\n");
        return;
    }

    char value[50];
    int key;
    int count = 0;

    while (fscanf(arq, "%d/%49s", &key, value) == 2) {
        count++;

        // Para a fila
        item_t item_fila;
        item_fila.key = key;
        item_fila.value = strdup(value);
        insert_node_f(f, item_fila);

        // Para o heap
        item_t item_heap;
        item_heap.key = key;
        item_heap.value = strdup(value);
        add_item(h, item_heap);
    }

    printf("Carregados %d itens do arquivo\n", count);
    fclose(arq);
}

// Realiza testes de remoção
void realizar_testes(fila_t *f, heap_t *h, int num_remocoes,
                     long long *comp_heap, long long *comp_fila) {
    // Reseta contadores
    reset_comparacoes_heap(h);
    reset_comparacoes_fila(f);

    int max_remocoes = num_remocoes;
    if (max_remocoes > f->size) max_remocoes = f->size;
    if (max_remocoes > h->size) max_remocoes = h->size;

    // Remove apenas do heap - NÃO LIBERA A MEMÓRIA AQUI
    for (int i = 0; i < max_remocoes; i++) {
        remove_item(h);  // Não libera o value aqui
    }

    // Remove apenas da fila - NÃO LIBERA A MEMÓRIA AQUI
    for (int i = 0; i < max_remocoes; i++) {
        remove_node_f(f);  // Não libera o value aqui
    }

    *comp_heap = h->comparacoes;
    *comp_fila = f->comparacoes;
}

// Gera script R para plotagem
void gerar_script_r(resultados_t *resultados) {
    FILE *r_script = fopen("plot_comparacoes.R", "w");
    if (!r_script) {
        fprintf(stderr, "Erro ao criar script R\n");
        return;
    }

    fprintf(r_script, "# Script R para comparação de estruturas\n");
    fprintf(r_script, "library(ggplot2)\n");
    fprintf(r_script, "library(reshape2)\n\n");

    fprintf(r_script, "tamanhos <- c(");
    for (int i = 0; i < NUM_SIMULACOES; i++) {
        fprintf(r_script, "%d%s", resultados->tamanhos[i],
                (i < NUM_SIMULACOES - 1) ? ", " : "");
    }
    fprintf(r_script, ")\n\n");

    fprintf(r_script, "heap_comp <- c(");
    for (int i = 0; i < NUM_SIMULACOES; i++) {
        fprintf(r_script, "%lld%s", resultados->heap_comparacoes[i],
                (i < NUM_SIMULACOES - 1) ? ", " : "");
    }
    fprintf(r_script, ")\n\n");

    fprintf(r_script, "fila_comp <- c(");
    for (int i = 0; i < NUM_SIMULACOES; i++) {
        fprintf(r_script, "%lld%s", resultados->fila_comparacoes[i],
                (i < NUM_SIMULACOES - 1) ? ", " : "");
    }
    fprintf(r_script, ")\n\n");

    fprintf(r_script, "dados <- data.frame(Tamanho = tamanhos, Heap = heap_comp, Fila = fila_comp)\n");
    fprintf(r_script, "dados_long <- melt(dados, id.vars = \"Tamanho\", \n");
    fprintf(r_script, "                   variable.name = \"Estrutura\", \n");
    fprintf(r_script, "                   value.name = \"Comparacoes\")\n\n");

    fprintf(r_script, "ggplot(dados_long, aes(x = Tamanho, y = Comparacoes, color = Estrutura)) +\n");
    fprintf(r_script, "  geom_point(size = 2) +\n");
    fprintf(r_script, "  geom_smooth(method = \"loess\", se = TRUE) +\n");
    fprintf(r_script, "  labs(title = \"Heap vs Lista Encadeada\",\n");
    fprintf(r_script, "       x = \"Elementos\", y = \"Comparações\", color = \"Estrutura\") +\n");
    fprintf(r_script, "  theme_minimal() +\n");
    fprintf(r_script, "  theme(legend.position = \"bottom\", plot.title = element_text(hjust = 0.5)) +\n");
    fprintf(r_script, "  scale_color_manual(values = c(\"Heap\" = \"#E74C3C\", \"Fila\" = \"#3498DB\"))\n\n");
    fprintf(r_script, "ggsave(\"comparacao_estruturas.png\", width = 10, height = 6, dpi = 300)\n");

    fclose(r_script);
    printf("Script R gerado: plot_comparacoes.R\n");
}

int main() {
    printf("=== ANÁLISE DE COMPARAÇÕES: HEAP vs FILA ENCADEADA ===\n\n");

    resultados_t resultados;
    srand(time(NULL));

    // Tamanhos para teste
    int tamanhos_base[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100,
                           120, 140, 160, 180, 200, 220, 240, 260, 280, 300,
                           350, 400, 450, 500, 550, 600, 650, 700, 750, 800};

    for (int i = 0; i < NUM_SIMULACOES; i++) {
        resultados.tamanhos[i] = tamanhos_base[i];
        resultados.heap_comparacoes[i] = 0;
        resultados.fila_comparacoes[i] = 0;
    }

    printf("Iniciando %d simulações...\n\n", NUM_SIMULACOES);
    fflush(stdout);

    for (int sim = 0; sim < NUM_SIMULACOES; sim++) {
        int num_linhas = resultados.tamanhos[sim];
        printf("Simulação %d/%d: Testando com %d elementos...",
               sim + 1, NUM_SIMULACOES, num_linhas);
        fflush(stdout);

        // Gera dados aleatórios
        gerar_dados_aleatorios(num_linhas);

        // Cria estruturas
        heap_t *h = create_empty_heap();
        fila_t *f = fila_criar();

        if (!h || !f) {
            fprintf(stderr, "\nErro ao criar estruturas\n");
            return 1;
        }

        // Preenche estruturas
        preencher_estruturas(f, h);

        // Realiza testes
        long long comp_heap = 0, comp_fila = 0;
        int num_remocoes = (num_linhas > 20) ? 15 : num_linhas / 2;
        realizar_testes(f, h, num_remocoes, &comp_heap, &comp_fila);

        resultados.heap_comparacoes[sim] = comp_heap;
        resultados.fila_comparacoes[sim] = comp_fila;

        printf(" Heap: %lld comp | Fila: %lld comp\n", comp_heap, comp_fila);
        fflush(stdout);

        // Libera memória
        free_heap(h);
        free_fila(f);
    }

    // Exibe resumo
    printf("\n=== RESUMO DOS RESULTADOS ===\n");
    printf("Tamanho | Heap | Fila\n");
    printf("--------|------|------\n");
    for (int i = 0; i < NUM_SIMULACOES; i++) {
        printf("%7d | %4lld | %4lld\n",
               resultados.tamanhos[i],
               resultados.heap_comparacoes[i],
               resultados.fila_comparacoes[i]);
    }

    // Gera script R
    printf("\nGerando script R para plotagem...\n");
    gerar_script_r(&resultados);

    // Salva CSV
    FILE *csv = fopen("resultados.csv", "w");
    if (csv) {
        fprintf(csv, "Tamanho,Heap,Fila\n");
        for (int i = 0; i < NUM_SIMULACOES; i++) {
            fprintf(csv, "%d,%lld,%lld\n",
                    resultados.tamanhos[i],
                    resultados.heap_comparacoes[i],
                    resultados.fila_comparacoes[i]);
        }
        fclose(csv);
        printf("Dados salvos em 'resultados.csv'\n");
    }
    return 0;
}