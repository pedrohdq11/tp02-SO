#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "comandos.h"
int main(int argc, char *argv[]) {
    printf("=========================================\n");
    printf("  Simulador de Sistema de Arquivos UFV\n");
    printf("=========================================\n");
    
    SistemaArquivos fs;
    memset(&fs, 0, sizeof(SistemaArquivos));
    
    const char *arquivo_disco = "disco.bin";
    
    // Verifica se o disco já foi formatado
    FILE *teste = fopen(arquivo_disco, "r");
    if (teste) {
        fclose(teste);
        printf("Disco encontrado. Montando sistema de arquivos...\n");
        fs_montar_disco(&fs, arquivo_disco);
    } else {
        printf("Disco nao encontrado. Formatando novo sistema de arquivos...\n");
        fs.sb.tamanho_particao = 10485760;
        fs.sb.tamanho_bloco = 4096;
        fs.sb.quantidade_inodes = 0;
        
        fs_formatar_disco(&fs, arquivo_disco);
    }
    
    printf("\n--- Status do Disco ---\n");
    printf("Tamanho da Particao: %u bytes\n", fs.sb.tamanho_particao);
    printf("Tamanho do Bloco:    %u bytes\n", fs.sb.tamanho_bloco);
    printf("Total de Blocos:     %u (Livres: %u)\n", fs.sb.quantidade_blocos, fs.sb.blocos_livres);
    printf("Total de I-nodes:    %u (Livres: %u)\n", fs.sb.quantidade_inodes, fs.sb.inodes_livres);

    if (argc > 1) {
        modo_arquivo(&fs, argv[1]);
    } else {
        printf("\n--- Menu de Execução ---\n");
        printf("1. Modo Iterativo\n");
        printf("2. Executar arquivo de teste (digite o nome do arquivo)\n");
        printf("Escolha uma opção (1-2): ");
        
        int opcao;
        if (scanf("%d", &opcao) != 1) {
            printf("Opção inválida!\n");
            opcao = 1;
        }
        getchar(); // Limpar o '\n' do buffer
        
        switch (opcao) {
            case 1:
                modo_interativo(&fs);
                break;
            case 2: {
                char nome_arquivo[256];
                printf("Digite o nome do arquivo de teste (ex: testes/renomear_mover_importar.txt ou testes/limpar.txt): ");
                if (fgets(nome_arquivo, sizeof(nome_arquivo), stdin) == NULL) {
                    printf("Erro ao ler o nome do arquivo.\n");
                    break;
                }
                nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';
                if (nome_arquivo[0] == '\0') {
                    printf("Nome do arquivo vazio. Usando modo interativo...\n");
                    modo_interativo(&fs);
                } else {
                    modo_arquivo(&fs, nome_arquivo);
                }
                break;
            }
            default:
                printf("Opção inválida! Usando modo interativo...\n");
                modo_interativo(&fs);
                break;
        }
    }

    printf("\nEncerrando o simulador...\n");
    
    if (fs.arquivo_disco) fclose(fs.arquivo_disco);
    if (fs.bitmap_inodes) free(fs.bitmap_inodes);
    if (fs.bitmap_blocos) free(fs.bitmap_blocos);
    
    return 0;
}
