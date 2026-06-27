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
    
    // Tenta abrir o arquivo para verificar se o disco já foi formatado antes
    FILE *teste = fopen(arquivo_disco, "r");
    if (teste) {
        fclose(teste);
        printf("Disco encontrado. Montando sistema de arquivos...\n");
        fs_montar_disco(&fs, arquivo_disco);
    } else {
        printf("Disco nao encontrado. Formatando novo sistema de arquivos...\n");
        fs.sb.tamanho_particao = 10485760; // 10 MB
        fs.sb.tamanho_bloco = 4096;        // 4 KB
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
        modo_interativo(&fs);
    }

    printf("\nEncerrando o simulador...\n");
    
    // Limpeza de recursos
    if (fs.arquivo_disco) fclose(fs.arquivo_disco);
    if (fs.bitmap_inodes) free(fs.bitmap_inodes);
    if (fs.bitmap_blocos) free(fs.bitmap_blocos);
    
    return 0;
}
