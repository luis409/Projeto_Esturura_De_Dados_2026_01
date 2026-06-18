//
// Created by luisg on 18/06/2026.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLANK '_'

typedef struct node {
    char sym;
    struct node *next;
} node;


typedef struct
{
    node *top;
} stack;

void stack_init(stack *s) { s->top = NULL; }
int  stack_empty(stack *s) { return s->top == NULL; }

void stack_push(stack *s, char c)
{
    node *new_node = (node*) malloc(sizeof(node));
    new_node->sym = c;
    new_node->next = s->top;
    s->top  = new_node;
}

char stack_pop(stack *s)
{
    if (stack_empty(s)) return BLANK;
    node *n = s->top;
    char sym = n->sym;
    s->top = n->next;
    free(n);
    return sym;
}

char stack_peek(stack *s)
{
    return stack_empty(s) ? BLANK : s->top->sym;
}

void stack_free(stack *s) { while (!stack_empty(s)) stack_pop(s); }

//definir a transicao
typedef struct
{
    int  from_state;
    char read_sym;
    int  to_state;
    char write_sym;
    int  direction;  //-1 = esquerda, +1 = direita
} transition;

// Maquina de Turing
typedef struct
{
    int state;
    stack left;
    stack right;
    transition *trans;
    int n_trans;
    int *accept;
    int n_accept;
} TM;

//inicialização da Máquina de Turing

void tm_init(TM *m, const char *input, transition *t, int nt, int *acc, int nacc, int init_state)
{
    m->state    = init_state;
    m->trans    = t;
    m->n_trans  = nt;
    m->accept   = acc;
    m->n_accept = nacc;
    stack_init(&m->left);
    stack_init(&m->right);

    //O loop percorre a string de trás para frente, fazendo push de cada símbolo em right
    for (int i = (int) strlen(input) - 1; i >= 0; i--)
        stack_push(&m->right, input[i]);
}

char tm_read(TM *m) { return stack_peek(&m->right); } //le o simbolo atual da fita

void tm_write(TM *m, char c)
{
    stack_pop(&m->right);
    stack_push(&m->right, c);
}

void tm_move(TM *m, int dir)
{
    if (dir > 0)
    {                     // direita
        char c = stack_pop(&m->right);
        stack_push(&m->left, c);
    } else
    {                     // esquerda
        char c = stack_pop(&m->left);
        stack_push(&m->right, c);
    }
}

int tm_is_accepting(TM *m) //verifica se o estado atual da máquina é um estado de aceitação
{
    for (int i = 0; i < m->n_accept; i++)
        if (m->accept[i] == m->state) return 1;
    return 0;
}

//executa um único passo da máquina, aplicando uma transição.

int tm_step(TM *m)
{
    char sym = tm_read(m);
    for (int i = 0; i < m->n_trans; i++)
    {
        transition *t = &m->trans[i];
        if (t->from_state == m->state && t->read_sym == sym)
        {
            tm_write(m, t->write_sym);
            tm_move(m, t->direction);
            m->state = t->to_state;
            return 1;
        }
    }
    return 0;  //nenhuma transição aplicável
}

//chama tm_step até a máquina parar

void tm_run(TM *m, int max_steps)
{
    int steps = 0;
    while (steps++ < max_steps && tm_step(m));

    printf("%s (estado final: %d)\n",
           tm_is_accepting(m) ? "ACEITO" : "REJEITADO", m->state);
    stack_free(&m->left);
    stack_free(&m->right);
}

int main(void)
{

    transition trans[] = {

        //Estado 0
        {0, '0', 1, 'X', +1},

        {0, 'Y', 0, 'Y', +1},

        {0, BLANK, 4, BLANK, +1},

        //Estado 1
        {1, '0', 1, '0',  +1},
        {1, 'Y', 1, 'Y',  +1},

        {1, '1', 2, 'Y',  -1},

        //Estado 2
        {2, 'Y', 2, 'Y',  -1},
        {2, '0', 2, '0',  -1},

        {2, 'X', 0, 'X',  +1},

        //Estado 4
        {4, 'X', 4, 'X',  +1},
        {4, 'Y', 4, 'Y',  +1},

    };

    int n_trans = sizeof(trans) / sizeof(trans[0]);

    int accept[] = {4};
    int n_accept = 1;

    const char *tests[] = {
        "0011",
        "000111",
        "01",
        "0001",
        "001",
        "1100",
        "",
        NULL
    };

    for (int i = 0; tests[i] != NULL; i++)
    {
        TM m;
        tm_init(&m, tests[i], trans, n_trans, accept, n_accept, 0);
        printf("%-10s → ", tests[i][0] ? tests[i] : "(vazio)");     //verifica se a string de entrada é vazia ao acessar o primeiro caractere dela
        tm_run(&m, 10000);
    }

    return 0;
}