#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

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
// 1. FUNÇÕES AUXILIARES
// ============================================================================

void limpar_ecra() 
{
    // \033[H  -> Move o cursor para o canto superior esquerdo (posição 1,1)
    // \033[2J -> Limpa o ecrã inteiro
    printf("\033[H\033[2J");
    fflush(stdout); // Garante que o comando é enviado imediatamente
}

#ifdef _WIN32
ssize_t getline(char **lineptr, size_t *n, FILE *stream) // Implementação manual do getline para Windows
{
    size_t pos;
    int c;

    if (lineptr == NULL || stream == NULL || n == NULL) {
        return -1;
    }

    if (*lineptr == NULL) {
        *n = 128;
        if ((*lineptr = malloc(*n)) == NULL) {
            return -1;
        }
    }

    pos = 0;
    while ((c = fgetc(stream)) != EOF) {
        if (pos + 1 >= *n) {
            size_t new_size = *n + 128;
            char *new_ptr = (char*) realloc(*lineptr, new_size);
            if (new_ptr == NULL) {
                return -1;
            }
            *lineptr = new_ptr;
            *n = new_size;
        }

        ((unsigned char *)*lineptr)[pos++] = c;
        if (c == '\n') {
            break;
        }
    }

    if (pos == 0) {
        return -1;
    }

    (*lineptr)[pos] = '\0';
    return pos;
}
#endif

// ============================================================================
// 2. FUNÇÕES DE CRIAR MAPA DE FREQUENCIA
// ============================================================================

int criar_frequencia_universal(const char* nome_arquivo, ssize_t mapa_frequencia[])
{
    FILE* arquivo = fopen(nome_arquivo, "rb");
    if(arquivo == NULL)
    {
        printf("Erro ao abrir o arquivo %s.\n", nome_arquivo);
        return 0;
    }

    unsigned char buffer[4096];
    size_t bytes_lidos;
    int total_do_arquivo = 0;

    while((bytes_lidos = fread(buffer, 1, sizeof(buffer), arquivo)) > 0) // LITERALMENTE SO PARA CONSEGUIR LER BLOCOS DE 4096 BYTES COM MAIS EFICIENCIA
    {
        for(size_t i = 0; i < bytes_lidos; i++)
        {
            mapa_frequencia[buffer[i]]++;                                // BUFFER[i] É O CARACTERE LIDO NO FREAD, QUE REPRESENTA 1 BYTE, POIS TODO CARACTERE É REPRESENTADO POR 8 BITS (1 BYTE)
            total_do_arquivo++;                                          // RETORNA O TOTAL DE CARACTERES LIDOS, OU SEJA, BYTES LIDOS NO TOTAL, SOMANDO TODOS OS BLOCOS DE 4096 BYTES/CARACTERES
        }
    }

    fclose(arquivo);
    return total_do_arquivo;
}

// ============================================================================
// 3. FUNÇÕES DE ÁRVORE E FILA DE PRIORIDADE
// ============================================================================

no_arvore* create_node_arvore(void* data) // CRIA UMA FOLHA DA ARVORE, COM OS FILHOS SENDO NULL E RECEBENDO O DADO PARA O PAI
{
    no_arvore* new_node = (no_arvore*) malloc(sizeof(no_arvore));
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->data = data;

    return new_node;
}

LISTA* create_list()
{
    LISTA* new_lista = (LISTA*) malloc(sizeof(LISTA));
    new_lista->head =  NULL;
    new_lista->tail = NULL;

    return new_lista;
}

void inserir_node_ordenado(LISTA* lista, no_arvore* node_arvore) // PEGA O DADO->FREQUENCY DO NODE DE ARVORE E VAI COMPARANDO COM AS FREQUENCIAS DOS NODES QUE ESTÃO NA LISTA, ATÉ ACHAR A POSIÇÃO ORDENADA CORRETAMENTE
{
    NODE* novo_node = (NODE*) malloc(sizeof(NODE));
    novo_node->item = node_arvore;
    novo_node->next = NULL;

    if(lista->head == NULL)
    {
        lista->head = novo_node;
        lista->tail = novo_node;
        return;
    }

    HuffmanData* dados_novo = (HuffmanData*) node_arvore->data;
    HuffmanData* dados_head = (HuffmanData*) ((no_arvore*) lista->head->item)->data;

    if(dados_novo->frequency < dados_head->frequency)
    {
        novo_node->next = lista->head;
        lista->head = novo_node;
        return;
    }

    NODE* atual = lista->head;
    while(atual->next != NULL)
    {
        HuffmanData* dados_proximo = (HuffmanData*) ((no_arvore*) atual->next->item)->data;

        if(dados_proximo->frequency > dados_novo->frequency)
        {
            break;
        }
        atual = atual->next;
    }

    novo_node->next = atual->next;
    atual->next = novo_node;

    if(novo_node->next == NULL)
    {
        lista->tail = novo_node;
    }
}

no_arvore* remover_inicio(LISTA* lista) // NECESSARIO PARA CRIAR A ARVORE, RETIRA O 1º NODE ATUAL DA LISTA, RETORNANDO O NODE ARVORE QUE FOI RETIRADO
{
    if(lista->head == NULL) return NULL;

    NODE* aux = lista->head;
    no_arvore* item_arvore = (no_arvore*) aux->item;

    lista->head = lista->head->next;
    if(lista->head == NULL)
    {
        lista->tail = NULL;
    }

    free(aux);
    return item_arvore;
}

void inserir_fila_ordenada(LISTA* fila_de_frequencia, ssize_t mapa_frequencia[]) // CRIA 1 NÓ DE ARVORE PARA CADA CARACTERE DO ARQUIVO, E INSERE ORDENADAMENTE DE ACORDO COM A FREQUENCIA
{
    for(int i = 0; i < 256; i++)
    {
        if(mapa_frequencia[i] > 0)
        {
            HuffmanData* data_atual = (HuffmanData*) malloc(sizeof(HuffmanData));
            data_atual->byte = i;
            data_atual->frequency = mapa_frequencia[i];

            no_arvore* new_no_arvore = create_node_arvore(data_atual);
            inserir_node_ordenado(fila_de_frequencia, new_no_arvore);
        }
    }
}

no_arvore* criar_arvore(LISTA* fila_de_frequencia) // RETIRA OS 2 PRIMEIROS ELEMENTOS DA LISTA DE FREQUENCIA, CRIA 1 NODE PAI, E REINSERE NOVAMENTE NA LISTA DE FORMA ORDENADA, DE ACORDO COM A FREQUENCIA
{
    if(fila_de_frequencia->head == NULL) return NULL;

    while(fila_de_frequencia->head != fila_de_frequencia->tail)
    {
        no_arvore* esquerdo = remover_inicio(fila_de_frequencia);
        no_arvore* direito = remover_inicio(fila_de_frequencia);

        HuffmanData* dados_esquerdo = (HuffmanData*) esquerdo->data;
        HuffmanData* dados_direito = (HuffmanData*) direito->data;

        HuffmanData* dados_pai = (HuffmanData*) malloc(sizeof(HuffmanData));
        dados_pai->byte = '\\';
        dados_pai->frequency = dados_esquerdo->frequency + dados_direito->frequency;

        no_arvore* no_pai = create_node_arvore(dados_pai);
        no_pai->left = esquerdo;
        no_pai->right = direito;

        inserir_node_ordenado(fila_de_frequencia, no_pai);
    }

    no_arvore* raiz_da_arvore = (no_arvore*) fila_de_frequencia->head->item;

    free(fila_de_frequencia->head);
    fila_de_frequencia->head = NULL;
    fila_de_frequencia->tail = NULL;

    return raiz_da_arvore;
}

int altura_arvore(no_arvore* raiz) // CALCULA A ALTURA DA ARVORE, PRIMEIRO FAZ COM TODOS OS NODES DA ESQUERDA, PARA DEPOIS FAZER COM OS NODES DA DIREITA, E O VALOR VAI ACUMULANDO AO COMPARAR ESQ E DIR E RETORNAR O MAIOR
{
    int esq, dir;
    if(raiz == NULL) return -1;
    else
    {
        esq = altura_arvore(raiz->left) + 1;
        dir = altura_arvore(raiz->right) + 1;

        if(esq > dir) return esq;
        else return dir;
    }
}

int calcular_tamanho_arvore(no_arvore* raiz) // CALCULA QUANTOS CARACTERES FORAM ADICIONADOS NA ARVORE, CONTANDO COM AS FOLHAS QUE NÃO SÃO CARACTERES DE FATO DA ARVORE, MAS OCUPAM ESPAÇO
{
    if (raiz == NULL) return 0;
    
    HuffmanData* data = (HuffmanData*) raiz->data;
    
    if (raiz->left == NULL && raiz->right == NULL) {
        if (data->byte == '*' || data->byte == '\\') {
            return 2;
        }
        return 1;
    }
    return 1 + calcular_tamanho_arvore(raiz->left) + calcular_tamanho_arvore(raiz->right);
}

void salvar_arvore(no_arvore* raiz, FILE* arquivo) // FAZ A 2ª PARTE DO CABEÇALHO DO ARQUIVO .huff, SALVANDO A ARVORE EM PRÉ-ORDEM, exemplo: "*AB", UTILIZANDO UMA QUANTIDADE VARIÁVEL DE BYTES DE ACORDO COM O ARQUIVO USADO
{
    if (raiz == NULL) return;
    
    HuffmanData* data = (HuffmanData*) raiz->data;
    
    if (raiz->left == NULL && raiz->right == NULL) {
        if (data->byte == '*' || data->byte == '\\') {
            fputc('\\', arquivo);
        }
        fputc(data->byte, arquivo);
    } else {
        fputc('*', arquivo);
        salvar_arvore(raiz->left, arquivo);
        salvar_arvore(raiz->right, arquivo);
    }
}

no_arvore* reconstruir_arvore(FILE* arquivo, int* tamanho_arvore) // LÊ O ARQUIVO BYTE POR BYTE, E RECRIA A ARVORE DO ARQUIVO COMPRIMIDO ENQUANTO O TAMANHO DA ARVORE FOR MAIOR QUE OU IGUAL A ZERO
{
    if (*tamanho_arvore <= 0) return NULL;
    
    int byte = fgetc(arquivo);
    (*tamanho_arvore)--;
    
    HuffmanData* data = (HuffmanData*) malloc(sizeof(HuffmanData));
    no_arvore* novo = create_node_arvore(data);
    
    if (byte == '\\') {
        data->byte = fgetc(arquivo);
        (*tamanho_arvore)--;
        novo->left = NULL;
        novo->right = NULL;
    } else if (byte == '*') {
        data->byte = '*';
        novo->left = reconstruir_arvore(arquivo, tamanho_arvore);
        novo->right = reconstruir_arvore(arquivo, tamanho_arvore);
    } else {
        data->byte = byte;
        novo->left = NULL;
        novo->right = NULL;
    }
    
    return novo;
}

// ============================================================================
// 4. FUNÇÕES DE CRIAR DICIONÁRIO
// ============================================================================

char** aloca_dicionario(int colunas) // CRIA UM ESPAÇO NA MEMÓRIA PARA ARMAZENAR QUAIS SERÃO AS SEQUENCIAS DE BITS QUE REPRESENTARÃO CADA CARACTERE DO ARQUIVO LIDO, AO SABER O NUMERO DE COLUNAS/ALTURA DA ARVORE
{
    char** dicionario;
    dicionario = malloc(sizeof(char*) * 256);
    for(int i = 0; i < 256; i++) dicionario[i] = calloc(colunas, sizeof(char));
    
    return dicionario;
}

void gerar_dicionario(char** dicionario, no_arvore* raiz, const char* string, int colunas) // CRIA A SEQUENCIA DE BITS EM FORMATO DE STRING, QUE SERÁ SALVA PARA CADA CARACTERE DO ARQUIVO LIDO, E COLOCA NO DICIONARIO
{
    char esquerda[colunas], direita[colunas];

    if(raiz->left == NULL && raiz->right == NULL)
    {
        HuffmanData* data = (HuffmanData*) raiz->data;
        strcpy(dicionario[data->byte], string);
    }
    else
    {
        strcpy(esquerda, string);
        strcpy(direita, string);

        strcat(esquerda, "0");
        strcat(direita, "1");

        gerar_dicionario(dicionario, raiz->left, esquerda, colunas);
        gerar_dicionario(dicionario, raiz->right, direita, colunas);
    }
}

void imprime_dicionario(char** dicionario) // INTUITIVO, APENAS IMPRIME BONITINHO O DICIONARIO ENQUANTO EXISTIR CARACTERES QUE REALMENTE FIZERAM PARTE DA ARVORE
{
    printf("\tDicionario:\n");
    for(int i = 0; i < 256; i++)
    {
        if(strcmp(dicionario[i], "") != 0) printf("\t%3c (Decimal: %3d): %s\n", i, i, dicionario[i]);
    }
    printf("\n");
}

// ============================================================================
// 5. FUNÇÕES DE MANIPULAÇÃO DE BITS E BYTES
// ============================================================================

void empacotar_e_escrever(const char* bits_str, FILE* arquivo_saida, unsigned char* bit_buffer, int* bit_count) // CRIAÇÃO DO BYTE COMPRIMIDO (USANDO MASCARA), QUE SERÁ ENVIADO PARA O ARQUIVO .huff APÓS SALVAR O CABEÇALHO E A ARVORE
{
    for (int i = 0; bits_str[i] != '\0'; i++) 
    {
        if (bits_str[i] == '1') {
            *bit_buffer |= (1 << (7 - *bit_count));
        }
        
        (*bit_count)++;

        if (*bit_count == 8) {
            fputc(*bit_buffer, arquivo_saida);
            *bit_buffer = 0;
            *bit_count = 0;
        }
    }
}

void flush_bits(FILE* arquivo_saida, unsigned char* bit_buffer, int* bit_count) // SE HOUVER SOBRA DE BITS APÓS O FIM DE EMPACOTAR/ESCREVER, SIGNIFICA QUE TEMOS QUE COLOCAR O QUE RESTOU DO BUFFER NO ARQUIVO DE SAIDA
{
    if (*bit_count > 0) {
        fputc(*bit_buffer, arquivo_saida);
        *bit_buffer = 0;
        *bit_count = 0;
    }
}

// ============================================================================
// 6. FUNÇÃO DE CODIFICAR O DICIONÁRIO
// ============================================================================

char* codificar(char** dicionario, unsigned char* texto, ssize_t bytes_lidos, int colunas) // LÊ O DICIONARIO E COLOCA A STRING DE BITS CORRESPONDENTE A CADA CARACTERE DENTRO DA STRING CODIGO, QUE SÃO BLOCOS DE 4096KB
{
    char* codigo = calloc((bytes_lidos * colunas) + 1, sizeof(char));

    for(ssize_t i = 0; i < bytes_lidos; i++)
    {
        strcat(codigo, dicionario[texto[i]]);
    }

    return codigo;
}

// ============================================================================
// 7. FUNÇÕES DE COMPACTAR, DESCOMPACTAR E MAIN
// ============================================================================

void descompactar()
{
    char nome_comprimido[256];
    printf("Digite o nome do arquivo comprimido para descompactar (Ex: bin_comprimido.txt.huff):\n");
    if (scanf("%255s", nome_comprimido) != 1) return;

    FILE* arquivo_comprimido = fopen(nome_comprimido, "rb");
    if(arquivo_comprimido == NULL) {
        printf("Erro ao abrir o arquivo %s.\n", nome_comprimido);
        return;
    }

    unsigned char byte1 = fgetc(arquivo_comprimido);
    unsigned char byte2 = fgetc(arquivo_comprimido);
    
    int lixo = byte1 >> 5; // O PRIMEIRO BYTE GUARDA TANTO O LIXO QUANTO OS 5 PRIMEIROS BITS DO TAMANHO, AQUI SEPARAMOS O QUE É O LIXO (3 PRIMEIROS BITS)
    int tamanho_arvore = ((byte1 & 0x1F) << 8) | byte2; // AQUI SEPARAMOS O TAMANHO DA ARVORE (5 BITS RESTANTES DO 1º BYTE + 8 BITS DO 2º BYTE), QUE DÁ UM NUMERO DE 13BITS, NO MÁXIMO

    printf("Lixo: %d bits | Tamanho da Arvore: %d bytes\n", lixo, tamanho_arvore);

    no_arvore* raiz = reconstruir_arvore(arquivo_comprimido, &tamanho_arvore);

    char nome_restaurado[256];
    size_t len = strlen(nome_comprimido);

    if (len > 5 && strcmp(&nome_comprimido[len - 5], ".huff") == 0) 
    {
        strncpy(nome_restaurado, nome_comprimido, len - 5);
        nome_restaurado[len - 5] = '\0';
    } 
    else 
    {
        strncpy(nome_restaurado, nome_comprimido, sizeof(nome_restaurado) - 1);
        nome_restaurado[sizeof(nome_restaurado) - 1] = '\0';
    }

    char nome_saida[300];
    snprintf(nome_saida, sizeof(nome_saida), "descompactado_%s", nome_restaurado);

    FILE* arquivo_descomprimido = fopen(nome_saida, "wb");

    if(arquivo_descomprimido == NULL) {
        printf("Erro ao criar o arquivo de saida: %s.\n", nome_saida);
        fclose(arquivo_comprimido);
        return;
    }

    no_arvore* atual = raiz; // PARA ANDAR PELA ARVORE SEM PERDER A RAIZ
    int c = fgetc(arquivo_comprimido); // PEGA O 1º BYTE DO ARQUIVO COMPRIMIDO
    if (c == EOF) {
        fclose(arquivo_comprimido);
        fclose(arquivo_descomprimido);
        return;
    }
    
    unsigned char byte_atual = (unsigned char)c; // PARA ANDAR PELOS BYTES DO ARQUIVO COMPRIMIDO SEM ESTRAGAR O PRÓPRIO
    
    while(1) {
        c = fgetc(arquivo_comprimido); // VAI PEGANDO OS PROXIMOS BYTES DO ARQUIVO, DESCOMPRIME O BYTE DE ACORDO COM A ARVORE E COLOCA NO ARQUIVO DESCOMPRIMIDO
        
        if (c == EOF) {
            int bits_limite = 8 - lixo; // AQUI ACABOU OS BYTES PARA LER, OU SEJA, O BYTE QUE SOBROU PODE CONTER LIXO, ENTÃO ANDAREMOS NA ARVORE UMA ULTIMA VEZ PARA TER CERTEZA DE QUE O FIM ESTARÁ CERTO
            for(int bit_pos = 0; bit_pos < bits_limite; bit_pos++) { // REPETE O MESMO PROCESSO DO LAÇO RECURSIVO DO ELSE DESSA CONDIÇÃO (c == EOF)
                int bit = (byte_atual >> (7 - bit_pos)) & 1;
                
                if(bit == 0) atual = atual->left;
                else atual = atual->right;

                if(atual->left == NULL && atual->right == NULL) {
                    HuffmanData* data = (HuffmanData*) atual->data;
                    fputc(data->byte, arquivo_descomprimido);
                    atual = raiz;
                }
            }
            break;
        } else {
            unsigned char proximo_byte = (unsigned char)c;
            for(int bit_pos = 0; bit_pos < 8; bit_pos++) {
                int bit = (byte_atual >> (7 - bit_pos)) & 1; // SÓ PARA SABER ONDE ANDAR NA ARVORE, DE ACORDO COM O BYTE QUE ESTÁ SENDO LIDO
                
                if(bit == 0) atual = atual->left;
                else atual = atual->right;

                if(atual->left == NULL && atual->right == NULL) { // QUANDO ACHA UMA FOLHA DA ARVORE, BOTA O BYTE "VERDADEIRO" E VOLTA O ATUAL PARA A RAIZ, PARA CONTINUAR O PROCESSO ATÉ NÃO EXISTIR MAIS BYTES PARA LER
                    HuffmanData* data = (HuffmanData*) atual->data;
                    fputc(data->byte, arquivo_descomprimido);
                    atual = raiz;
                }
            }
            byte_atual = proximo_byte; // FAZ NO FINAL, PARA NÃO GERAR ERRO SE O FGETC PEGASSE NULL E O BYTE_ATUAL TENTARIA PEGAR O NULL, MAS NÃO EXISTE CHAR NULL, PORTANTO IRIA GERAR ERRO DE EXECUÇÃO OU LOOP
        }
    }

    fclose(arquivo_comprimido);
    fclose(arquivo_descomprimido);

    printf("Arquivo descompactado e restaurado com sucesso em '%s'!\n", nome_saida);
}

no_arvore* compactar()
{
    ssize_t mapa_frequencia[256] = {0};
    unsigned char* nome_arquivo = NULL;
    size_t tamanho_buffer = 0;
    ssize_t char_lidos;

    printf("Digite o nome do arquivo para compactar (Exemplo: gatos.jpg , chave.txt, notas.pdf):\n");
    char_lidos = getline((char**) &nome_arquivo, &tamanho_buffer, stdin);

    // REMOVE OS CARACTERES DE QUEBRA DE LINHA DO WINDOWS E DO LINUX, COMO \r E \n
    if (char_lidos > 0) 
    {
        nome_arquivo[strcspn((char*)nome_arquivo, "\r\n")] = '\0';
    } 
    else 
    {
        printf("Erro ao ler a entrada do usuario.\n");
        return NULL;
    }

    char nome_saida[512];
    snprintf(nome_saida, sizeof(nome_saida), "%s.huff", nome_arquivo); // SO BOTA O NOME CORRETO DE SAIDA PARA O .huff

    int total_bytes = criar_frequencia_universal((const char*) nome_arquivo, mapa_frequencia); // CRIA A FREQUENCIA UNIVERSAL E VÊ QUANTOS CARACTERES/BYTES DIFERENTES EXISTEM NO ARQUIVO

    if(total_bytes == 0)
    {
        printf("Nao conseguiu ler.\n");
        return NULL;
    }

    LISTA* fila_de_frequencia = create_list();

    inserir_fila_ordenada(fila_de_frequencia, mapa_frequencia);

    no_arvore* raiz_huffman = criar_arvore(fila_de_frequencia);

    int colunas = altura_arvore(raiz_huffman) + 1; // JÁ CONTA COM A RAIZ, POR ISSO O +1
    char** dicionario = aloca_dicionario(colunas);
    gerar_dicionario(dicionario, raiz_huffman, "", colunas);
    // imprime_dicionario(dicionario); // FICA A CRITERIO DE COLOCAR OU NÃO, POIS PARA PODER VER O DICIONARIO BONITINHO, É NECESSARIO TIRAR A CHAMADA DO LIMPAR_ECRA NA MAIN, POIS ELA FAZ O CLEAR DO TERMINAL

    FILE* arquivo_entrada = fopen((char*) nome_arquivo, "rb");
    FILE* arquivo_saida = fopen(nome_saida, "wb");

    if(arquivo_entrada == NULL || arquivo_saida == NULL)
    {
        printf("Erro ao abrir os arquivos.\n");
        free(nome_arquivo);
        return NULL;
    }

    fputc(0, arquivo_saida);
    fputc(0, arquivo_saida);

    salvar_arvore(raiz_huffman, arquivo_saida); // SALVA A ARVORE INTEIRA NO RESTANTE DO ARQUIVO

    unsigned char bit_buffer = 0;
    int bit_count = 0;

    unsigned char buffer_leitura[4096];
    size_t bytes_lidos;

    while((bytes_lidos = fread(buffer_leitura, 1, sizeof(buffer_leitura), arquivo_entrada)) > 0)
    {
        char* codificado = codificar(dicionario, buffer_leitura, bytes_lidos, colunas);

        empacotar_e_escrever(codificado, arquivo_saida, &bit_buffer, &bit_count);
        free(codificado);
    }

    int lixo = (bit_count == 0) ? 0 : (8 - bit_count);
    int tamanho_arvore = calcular_tamanho_arvore(raiz_huffman);

    flush_bits(arquivo_saida, &bit_buffer, &bit_count);

    fseek(arquivo_saida, 0 , SEEK_SET); // ENCONTRA OS 2 BYTES INICIAIS

    unsigned char byte1 = (lixo << 5) | (tamanho_arvore >> 8); // FAZ UMA MASCARA PARA O 1º BYTE, OU SEJA, ANTES O BYTE1 É 00000000, DEPOIS ELE COLOCARÁ OS 3 PRIMEIROS BITS COMO LIXO, E O RESTO (5 BITS) PARA CASO A ARVORE NECESSITE
    unsigned char byte2 = tamanho_arvore & 0xFF; // SALVA O TAMANHO DA ARVORE NO BYTE2, UTILIZANDO O RESTO DOS BITS QUE TALVEZ SOBRARAM NO BYTE1 PARA PREENCHER O BYTE2, OU SEJA, O TAMANHO PODE TER 13 BITS

    fputc(byte1, arquivo_saida); // COLOCA AMBOS OS BYTES ALTERADOS NO ARQUIVO DE SAIDA, NAQUELA POSIÇÃO DO BYTE 1 E DEPOIS DO BYTE 2
    fputc(byte2, arquivo_saida);

    fclose(arquivo_entrada);
    fclose(arquivo_saida);

    free(nome_arquivo);

    limpar_ecra();

    printf("Arquivo Binário compactado com sucesso em '%s'!.\n", nome_saida);

    return raiz_huffman;
}

int main()
{
    printf("\tBem-Vindos a Central de Projetos 2º Periodo - Marcio Ribeiro!\n");
    printf("\n\tEscolha uma das alternativas abaixo:\n");
    printf("\t0 - Sair da Central de Projetos\n");
    printf("\t1 - Compactar Arquivo\n");
    printf("\t2 - Descompactar Arquivo\n");
    printf("\tSua Opcao: ");

    short n = -1;
    while(n != 0)
    {
        if(scanf("%hd", &n) != 1)
        {
            printf("\t[Erro] Por favor, digite um numero valido.\n");
            getchar();
            continue;
        }
        
        getchar();

        switch (n)
        {
            case 1:
                limpar_ecra();
                compactar();
                break;
            case 2:
                limpar_ecra();
                descompactar();
                break;
            case 0:
                limpar_ecra();
                printf("\n\tSaindo da Central de Projetos. Ate mais!\n");
                return 0;
                break;
            default:
                printf("\n\t[Opcao Invalida] Tente novamente.\n\n");
                break;
        }

    
        printf("\n\tEscolha uma das alternativas abaixo:\n");
        printf("\t0 - Sair da Central de Projetos\n");
        printf("\t1 - Compactar Arquivo\n");
        printf("\t2 - Descompactar Arquivo\n");
        printf("\tSua Escolha: ");
    }

    return 0;
}