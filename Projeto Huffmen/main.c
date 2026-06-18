#include "huffman.h"

no_arvore* huffman()
{
    ssize_t mapa_frequencia[256] = {0};
    unsigned char* nome_arquivo = NULL;
    size_t tamanho_buffer = 0;
    ssize_t char_lidos;

    printf("Digite o nome do arquivo para compactar (Exemplo: gatos.jpg , chave.txt, notas.pdf):\n");
    char_lidos = getline((char**) &nome_arquivo, &tamanho_buffer, stdin);
    nome_arquivo[char_lidos-1] = '\0';
    char nome_saida[512];
    snprintf(nome_saida, sizeof(nome_saida), "%s.huff", nome_arquivo);

    if(char_lidos > 0 && nome_arquivo[char_lidos - 1] == '\n') nome_arquivo[char_lidos-1] = '\0';

    int total_bytes = criar_frequencia_universal(nome_arquivo, mapa_frequencia);

    if(total_bytes == 0)
    {
        printf("Não conseguiu ler.\n");
        return NULL;
    }

    LISTA* fila_de_frequencia = create_list();

    inserir_fila_ordenada(fila_de_frequencia, mapa_frequencia);

    no_arvore* raiz_huffman = criar_arvore(fila_de_frequencia);

    int colunas = altura_arvore(raiz_huffman) + 1;
    char** dicionario = aloca_dicionario(colunas);
    gerar_dicionario(dicionario, raiz_huffman, "", colunas);
    imprime_dicionario(dicionario);

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

    salvar_arvore(raiz_huffman, arquivo_saida);

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

    fseek(arquivo_saida, 0 , SEEK_SET);

    unsigned char byte1 = (lixo << 5) | (tamanho_arvore >> 8);
    unsigned char byte2 = tamanho_arvore & 0xFF;

    fputc(byte1, arquivo_saida);
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
    printf("\n\tEscolha uma das opções abaixo:\n");
    printf("\t0 - Sair da Central de Projetos\n");
    printf("\t1 - Compactar Arquivo\n");
    printf("\t2 - Descompactar Arquivo\n");
    printf("\tSua Opcao: ");

    short n = -1;
    no_arvore* raiz_compartilhada = NULL;
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
                huffman();
                break;
            case 2:
                limpar_ecra();
                decodificar();
                break;
            case 0:
                limpar_ecra();
                printf("\n\tSaindo da Central de Projetos. Até mais!\n");
                return 0;
                break;
            default:
                printf("\n\t[Opcao Invalida] Tente novamente.\n\n");
                break;
        }

    
        printf("\n\tEscolha uma das opções abaixo:\n");
        printf("\t0 - Sair da Central de Projetos\n");
        printf("\t1 - Compactar Arquivo\n");
        printf("\t2 - Descompactar Arquivo\n");
        printf("\tSua Opcao: ");
    }

    return 0;
}