#include <stdio.h>
#include "fs.h"
#include "comandos.h"

int main(int argc, char *argv[]) {
    printf("=========================================\n");
    printf("  Simulador de Sistema de Arquivos UFV\n");
    printf("=========================================\n");
    
    // Inicialização da simulação (Superbloco)
    Superbloco sb;
    
    // Na sua implementação completa, você pode querer ler isso via argumentos do main ou no início interativamente.
    sb.tamanho_particao = 10485760; // Exemplo: partição de 10 MB
    sb.tamanho_bloco = 4096;         // Exemplo: bloco de 4 KB
    sb.quantidade_blocos = sb.tamanho_particao / sb.tamanho_bloco;
    // sb.quantidade_inodes = ... (definir heurística)
    
    printf("Iniciando simulacao...\n");
    printf("Tamanho da Particao: %u bytes\n", sb.tamanho_particao);
    printf("Tamanho do Bloco:    %u bytes\n", sb.tamanho_bloco);
    printf("Total de Blocos:     %u\n", sb.quantidade_blocos);

    // O simulador permite ler de um arquivo (batch) ou direto do terminal
    // Estas funções agora estão centralizadas em comandos.h / comandos.c
    if (argc > 1) {
        // O primeiro argumento após o executável é o arquivo de entrada
        modo_arquivo(argv[1]);
    } else {
        // Nenhum arquivo fornecido, entra no modo interativo de comandos
        modo_interativo();
    }

    printf("\nEncerrando o simulador...\n");
    return 0;
}
