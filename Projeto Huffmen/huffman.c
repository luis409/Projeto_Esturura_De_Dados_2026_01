#include "huffman.h"

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
            mapa_frequencia[buffer[i]]++;                                // BUFFER[i] É O CARACTERE LIDO NO FREAD, QUE REPRESENTA 1 BYTE, POIS TODO CHARACTERE É REPRESENTADO POR 8 BITS (1 BYTE)
            total_do_arquivo++;                                          // RETORNA O TOTAL DE CARACTERES LIDOS, OU SEJA, BYTES LIDOS NO TOTAL, SOMANDO TODOS OS BLOCOS DE 4096 BYTES/CARACTERES
        }
    }

    fclose(arquivo);
    return total_do_arquivo;
}

// ============================================================================
// 3. FUNÇÕES DE ÁRVORE E FILA DE PRIORIDADE
// ============================================================================

no_arvore* create_node_arvore(void* data)
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

void inserir_node_ordenado(LISTA* lista, no_arvore* node_arvore)
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

no_arvore* remover_inicio(LISTA* lista)
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

void inserir_fila_ordenada(LISTA* fila_de_frequencia, ssize_t mapa_frequencia[])
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

no_arvore* criar_arvore(LISTA* fila_de_frequencia)
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

int altura_arvore(no_arvore* raiz)
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

int calcular_tamanho_arvore(no_arvore* raiz) 
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

void salvar_arvore(no_arvore* raiz, FILE* arquivo) 
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

no_arvore* reconstruir_arvore(FILE* arquivo, int* tamanho_arvore) 
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

char** aloca_dicionario(int colunas)
{
    char** dicionario;
    dicionario = malloc(sizeof(char*) * 256);
    for(int i = 0; i < 256; i++) dicionario[i] = calloc(colunas, sizeof(char));
    
    return dicionario;
}

void gerar_dicionario(char** dicionario, no_arvore* raiz, unsigned char* string, int colunas)
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

void imprime_dicionario(char** dicionario)
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

void empacotar_e_escrever(const char* bits_str, FILE* arquivo_saida, unsigned char* bit_buffer, int* bit_count) 
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

void flush_bits(FILE* arquivo_saida, unsigned char* bit_buffer, int* bit_count) 
{
    if (*bit_count > 0) {
        fputc(*bit_buffer, arquivo_saida);
        *bit_buffer = 0;
        *bit_count = 0;
    }
}

// ============================================================================
// 6. FUNÇÕES DE CODIFICAR E DECODIFICAR
// ============================================================================

char* codificar(char** dicionario, unsigned char* texto, ssize_t bytes_lidos, int colunas)
{
    char* codigo = calloc((bytes_lidos * colunas) + 1, sizeof(char));

    for(ssize_t i = 0; i < bytes_lidos; i++)
    {
        strcat(codigo, dicionario[texto[i]]);
    }

    return codigo;
}

void decodificar()
{
    char nome_comprimido[256];
    printf("Digite o nome do arquivo comprimido para descompactar (Ex: bin_comprimido.txt):\n");
    if (scanf("%255s", nome_comprimido) != 1) return;

    FILE* arquivo_comprimido = fopen(nome_comprimido, "rb");
    if(arquivo_comprimido == NULL) {
        printf("Erro ao abrir o arquivo %s.\n", nome_comprimido);
        return;
    }

    unsigned char byte1 = fgetc(arquivo_comprimido);
    unsigned char byte2 = fgetc(arquivo_comprimido);
    
    int lixo = byte1 >> 5;
    int tamanho_arvore = ((byte1 & 0x1F) << 8) | byte2;

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

    no_arvore* atual = raiz;
    int c = fgetc(arquivo_comprimido);
    if (c == EOF) {
        fclose(arquivo_comprimido);
        fclose(arquivo_descomprimido);
        return;
    }
    
    unsigned char byte_atual = (unsigned char)c;
    
    while(1) {
        c = fgetc(arquivo_comprimido);
        
        if (c == EOF) {
            int bits_limite = 8 - lixo;
            for(int bit_pos = 0; bit_pos < bits_limite; bit_pos++) {
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
                int bit = (byte_atual >> (7 - bit_pos)) & 1;
                
                if(bit == 0) atual = atual->left;
                else if(bit == 1) atual = atual->right;

                if(atual->left == NULL && atual->right == NULL) {
                    HuffmanData* data = (HuffmanData*) atual->data;
                    fputc(data->byte, arquivo_descomprimido);
                    atual = raiz;
                }
            }
            byte_atual = proximo_byte;
        }
    }

    fclose(arquivo_comprimido);
    fclose(arquivo_descomprimido);

    printf("Arquivo decodificado e restaurado com sucesso em '%s'!\n", nome_saida);
}