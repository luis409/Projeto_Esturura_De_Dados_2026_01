//
// Created by luisg on 20/06/2026.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#define MAX_RESTR 100

#define OP_LE 0  // <=
#define OP_GE 1  // >=
#define OP_LT 2  // <
#define OP_GT 3  // >
#define OP_EQ 4  // ==

typedef struct {
    int coef;   // coeficiente de x
    int op;     // operador (OP_LE, OP_GE, ...)
    int rhs;    // lado direito da restrição
} Restricao;

typedef struct {
    Restricao restricoes[MAX_RESTR];
    int num_restricoes;
} LIA;

int ler_operador(const char* token) {
    if (strcmp(token, "<=") == 0) return OP_LE;
    if (strcmp(token, ">=") == 0) return OP_GE;
    if (strcmp(token, "<")  == 0) return OP_LT;
    if (strcmp(token, ">")  == 0) return OP_GT;
    if (strcmp(token, "==") == 0) return OP_EQ;
    printf("Operador invalido: %s\n", token);
    exit(1);
}

void ler_arquivo_lia(const char* nome_arquivo, LIA* problema) {
    FILE* arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) { printf("Erro ao abrir arquivo.\n"); exit(1); }

    char linha[256];
    int idx = 0;

    while (fgets(linha, sizeof(linha), arquivo)) {
        if (linha[0] == 'c') continue; // comentario, ignora

        if (linha[0] == 'p') {
            // linha "p lia N" -> so usamos para validar, num_restricoes real
            // sera contado pelas linhas lidas abaixo
            continue;
        }

        char op_str[3];
        int coef, rhs;

        // le no formato: coef op rhs   (ex: "2 <= 8")
        if (sscanf(linha, "%d %2s %d", &coef, op_str, &rhs) == 3) {
            problema->restricoes[idx].coef = coef;
            problema->restricoes[idx].op   = ler_operador(op_str);
            problema->restricoes[idx].rhs  = rhs;
            idx++;
        }
    }

    problema->num_restricoes = idx;
    fclose(arquivo);
}

// ========== NORMALIZACAO: transforma cada restricao em um intervalo [lo, hi] ==========
//
// Para "coef * x OP rhs", isolamos x dividindo por coef.
// Se coef for negativo, o sentido da desigualdade INVERTE (regra de aritmetica).
// Usamos divisao inteira com cuidado (igual fariamos para garantir x inteiro).

void normalizar_restricao(Restricao r, int* lo, int* hi) {
    *lo = INT_MIN;
    *hi = INT_MAX;

    if (r.coef == 0) {
        printf("Coeficiente 0 invalido.\n");
        exit(1);
    }

    // valor limite (real) seria rhs / coef; tratamos os casos de
    // arredondamento para manter o dominio dos INTEIROS correto.
    switch (r.op) {
        case OP_LE: // coef*x <= rhs
            if (r.coef > 0) *hi = (int) floor((double) r.rhs / r.coef);
            else            *lo = (int) ceil((double) r.rhs / r.coef);
            break;
        case OP_GE: // coef*x >= rhs
            if (r.coef > 0) *lo = (int) ceil((double) r.rhs / r.coef);
            else            *hi = (int) floor((double) r.rhs / r.coef);
            break;
        case OP_LT: // coef*x < rhs  ->  coef*x <= rhs-1 (em inteiros)
            if (r.coef > 0) *hi = (int) floor((double) (r.rhs - 1) / r.coef);
            else            *lo = (int) ceil((double) (r.rhs - 1) / r.coef);
            break;
        case OP_GT: // coef*x > rhs  ->  coef*x >= rhs+1
            if (r.coef > 0) *lo = (int) ceil((double) (r.rhs + 1) / r.coef);
            else            *hi = (int) floor((double) (r.rhs + 1) / r.coef);
            break;
        case OP_EQ: // coef*x == rhs  -> x = rhs/coef, so se divisao for exata
            if (r.rhs % r.coef != 0) { *lo = 1; *hi = 0; return; } // intervalo vazio
            *lo = *hi = r.rhs / r.coef;
            break;
    }
}

// ========== INTERSECAO (papel do Theory Solver) ==========
// Igual ao verificar_formula() do SAT, mas em vez de percorrer uma
// lista encadeada de literais, percorremos o vetor de restricoes
// fazendo a interseccao acumulada dos intervalos.

int resolver_lia(LIA* problema, int* lo_final, int* hi_final) {
    int lo_atual = INT_MIN, hi_atual = INT_MAX;

    for (int i = 0; i < problema->num_restricoes; i++) {
        int lo, hi;
        normalizar_restricao(problema->restricoes[i], &lo, &hi);

        // interseccao: pega o maior dos minimos e o menor dos maximos
        if (lo > lo_atual) lo_atual = lo;
        if (hi < hi_atual) hi_atual = hi;

        // poda antecipada: se o intervalo ja esta vazio, UNSAT
        if (lo_atual > hi_atual) {
            *lo_final = lo_atual;
            *hi_final = hi_atual;
            return 0; // UNSAT
        }
    }

    *lo_final = lo_atual;
    *hi_final = hi_atual;
    return (lo_atual <= hi_atual); // SAT se sobrou pelo menos um inteiro
}

// ========== VERIFICACAO FINAL (igual ao "Check Solution" do slide) ==========
// Reconfirma cada restricao original com o x escolhido, igual o slide faz
// substituindo x=3 e x=4 nas formulas originais.

int verificar_solucao(LIA* problema, int x) {
    for (int i = 0; i < problema->num_restricoes; i++) {
        Restricao r = problema->restricoes[i];
        int valor = r.coef * x;
        switch (r.op) {
            case OP_LE: if (!(valor <= r.rhs)) return 0; break;
            case OP_GE: if (!(valor >= r.rhs)) return 0; break;
            case OP_LT: if (!(valor <  r.rhs)) return 0; break;
            case OP_GT: if (!(valor >  r.rhs)) return 0; break;
            case OP_EQ: if (!(valor == r.rhs)) return 0; break;
        }
    }
    return 1;
}

// ========== MAIN ==========
int main() {
    LIA problema = {0};
    char nome_arq[100];

    printf("Digite o nome do arquivo (.lia): ");
    fgets(nome_arq, sizeof(nome_arq), stdin);
    nome_arq[strcspn(nome_arq, "\n")] = 0;

    ler_arquivo_lia(nome_arq, &problema);

    int lo, hi;
    int status = resolver_lia(&problema, &lo, &hi);

    if (!status) {
        printf("\nUNSAT!\n");
        return 0;
    }

    printf("\nSAT!\n");
    printf("Intervalo final apos interseccao: x in [%d, %d]\n\n", lo, hi);

    // Enumera e confirma cada solucao inteira do intervalo, como no slide
    int alguma_valida = 0;
    for (int x = lo; x <= hi; x++) {
        if (verificar_solucao(&problema, x)) {
            printf("x = %d  -> valida\n", x);
            alguma_valida = 1;
        }
    }

    if (!alguma_valida) {
        printf("Nenhuma solucao inteira valida encontrada (UNSAT na pratica).\n");
    }

    return 0;
}