//
// Created by luisg on 18/06/2026.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LIT  500
#define MAX_CLAU 1000

#define SAT       1
#define UNSAT     0
#define UNDEFINED 2

// 1. ESTRUTURA DE DADOS: Lista Encadeada para os Literais de uma Cláusula
typedef struct NoLiteral {
    int valor;
    struct NoLiteral* proximo;
} NoLiteral;

typedef struct {
    NoLiteral* cabeca;
    int tamanho;
} Clausula;

typedef struct {
    Clausula clausulas[MAX_CLAU];
    int num_clausulas;
    int num_literais;
} CNF;

// 2. ESTRUTURA DE DADOS: Árvore Binária Dinâmica de Decisões
typedef struct Arvore {
    int variavel;
    int atribuicoes[MAX_LIT];
    struct Arvore *esq; // Caminho se a variável for FALSA (-1)
    struct Arvore *dir; // Caminho se a variável for VERDADEIRA (1)
} Arvore;

// ========== GESTÃO DA LISTA ENCADEADA ==========

// Adiciona um número no fim da lista encadeada da cláusula
void inserir_literal(Clausula* c, int valor) {
    NoLiteral* novo = (NoLiteral*)malloc(sizeof(NoLiteral));
    novo->valor = valor;
    novo->proximo = NULL;

    if (c->cabeca == NULL) {
        c->cabeca = novo;
    } else {
        NoLiteral* atual = c->cabeca;
        while (atual->proximo != NULL) atual = atual->proximo;
        atual->proximo = novo;
    }
    c->tamanho++;
}

// Limpa todas as listas encadeadas de literais criadas no Heap
void limpar_listas_cnf(CNF* cnf) {
    for (int i = 0; i < cnf->num_clausulas; i++) {
        NoLiteral* atual = cnf->clausulas[i].cabeca;
        while (atual != NULL) {
            NoLiteral* aux = atual;
            atual = atual->proximo;
            free(aux);
        }
    }
}

// ========== LÓGICA DO SAT SOLVER ==========

void ler_arquivo_cnf(const char* nome_arquivo, CNF* problema) {
    FILE* arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) { printf("Erro ao abrir arquivo.\n"); exit(1); }

    char linha[256];
    int idx = 0;

    while (fgets(linha, sizeof(linha), arquivo)) {
        if (linha[0] == 'c') continue;
        if (linha[0] == 'p') {
            sscanf(linha, "p cnf %d %d", &problema->num_literais, &problema->num_clausulas);
            continue;
        }

        Clausula c = {NULL, 0};
        char *token = strtok(linha, " \t\n");
        while (token != NULL) {
            int val = atoi(token);
            if (val == 0) break; // 0 indica fim da cláusula
            inserir_literal(&c, val);
            token = strtok(NULL, " \t\n");
        }

        if (c.tamanho > 0) problema->clausulas[idx++] = c;
    }
    fclose(arquivo);
}

// Percorre a lista encadeada e analisa se as cláusulas são verdadeiras ou falsas
int verificar_formula(CNF* cnf, int atribuido[]) {
    int alguma_indefinida = 0;

    for (int i = 0; i < cnf->num_clausulas; i++) {
        int clausula_satisfeita = 0;

        // Caminhando pela lista encadeada
        NoLiteral* atual = cnf->clausulas[i].cabeca;
        while (atual != NULL) {
            int lit = atual->valor;
            int var = abs(lit);
            int val = atribuido[var];

            if ((lit > 0 && val == 1) || (lit < 0 && val == -1)) {
                clausula_satisfeita = 1;
                break; // Se um literal deu TRUE, a cláusula inteira é TRUE
            }
            atual = atual->proximo;
        }

        // Se a cláusula não foi satisfeita, avalia o estado dela
        if (!clausula_satisfeita) {
            // Se não tem nenhuma variável livre nela, ela está violada (FALSA)
            int tem_livre = 0;
            atual = cnf->clausulas[i].cabeca;
            while (atual != NULL) {
                if (atribuido[abs(atual->valor)] == 0) { tem_livre = 1; break; }
                atual = atual->proximo;
            }

            if (!tem_livre) return UNSAT; // Uma cláusula falsa mata a fórmula inteira
            alguma_indefinida = 1;       // Ainda pode ser resolvida no futuro
        }
    }
    return alguma_indefinida ? UNDEFINED : SAT;
}

// Algoritmo de Backtracking com Árvore Dinâmica
int resolver_sat(Arvore *no, CNF *cnf, int solucao[]) {
    int status = verificar_formula(cnf, no->atribuicoes);

    if (status == SAT) {
        memcpy(solucao, no->atribuicoes, sizeof(int) * (cnf->num_literais + 1));
        return SAT;
    }
    if (status == UNSAT) return UNSAT;

    // Procura a próxima variável livre para decidir
    int var_escolhida = -1;
    for (int i = 1; i <= cnf->num_literais; i++) {
        if (no->atribuicoes[i] == 0) { var_escolhida = i; break; }
    }
    if (var_escolhida == -1) return UNSAT;

    no->variavel = var_escolhida;

    // Ramo Direito: Tenta criar nó assumindo VERDADEIRO (1)
    no->dir = (Arvore*)malloc(sizeof(Arvore));
    no->dir->esq = no->dir->dir = NULL;
    memcpy(no->dir->atribuicoes, no->atribuicoes, sizeof(int) * MAX_LIT);
    no->dir->atribuicoes[var_escolhida] = 1;

    if (resolver_sat(no->dir, cnf, solucao) == SAT) return SAT;

    free(no->dir); // Poda o galho direito se falhar
    no->dir = NULL;

    // Ramo Esquerdo: Tenta criar nó assumindo FALSO (-1)
    no->esq = (Arvore*)malloc(sizeof(Arvore));
    no->esq->esq = no->esq->dir = NULL;
    memcpy(no->esq->atribuicoes, no->atribuicoes, sizeof(int) * MAX_LIT);
    no->esq->atribuicoes[var_escolhida] = -1;

    if (resolver_sat(no->esq, cnf, solucao) == SAT) return SAT;

    free(no->esq); // Poda o galho esquerdo se falhar
    no->esq = NULL;

    return UNSAT;
}

void limpar_arvore(Arvore *no) {
    if (no == NULL) return;
    limpar_arvore(no->esq);
    limpar_arvore(no->dir);
    free(no);
}

// ========== MAIN ==========
int main() {
    CNF problema = {0};
    char nome_arq[100];

    printf("Nome do arquivo (.cnf): ");
    fgets(nome_arq, sizeof(nome_arq), stdin);
    nome_arq[strcspn(nome_arq, "\n")] = 0;

    ler_arquivo_cnf(nome_arq, &problema);

    // Inicializa o nó raiz na Stack
    Arvore raiz = {0};

    int solucao[MAX_LIT] = {0};

    if (resolver_sat(&raiz, &problema, solucao)) {
        printf("\nSAT!\n");
        for (int i = 1; i <= problema.num_literais; i++) {
            printf("x%d = %s\n", i, solucao[i] == 1 ? "true" : "false");
        }
    } else {
        printf("\nUNSAT!\n");
    }

    // Limpeza do Heap
    limpar_arvore(raiz.esq);
    limpar_arvore(raiz.dir);
    limpar_listas_cnf(&problema);

    return 0;
}