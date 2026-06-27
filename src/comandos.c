#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comandos.h"
#include "fs.h"

// Função para entender e redirecionar o comando digitado
void processar_comando(SistemaArquivos *fs, char *comando) {
    char linha_copia[512];
    strncpy(linha_copia, comando, sizeof(linha_copia));
    char *acao = strtok(linha_copia, " ");
    if (acao == NULL) {
        return; 
    }
    char *arg1 = strtok(NULL, " ");
    char *arg2 = strtok(NULL, " ");

    // 4. Árvore de decisões: qual comando foi chamado?
    if (strcmp(acao, "criar_dir") == 0) {
        if (arg1 != NULL) {
            printf("[Log] Executando logica para CRIAR DIRETORIO: %s\n", arg1);
            // -> Aqui chamaremos a função fs_criar_dir(arg1) no futuro
        } else {
            printf("Erro: Faltou o nome do diretorio (Ex: criar_dir /pasta).\n");
        }
    } 
    else if (strcmp(acao, "apagar_dir") == 0) {
        if (arg1 != NULL) {
            printf("[Log] Executando logica para APAGAR DIRETORIO: %s\n", arg1);
            // -> fs_apagar_dir(arg1)
        } else {
            printf("Erro: Faltou o nome do diretorio a ser apagado.\n");
        }
    }
    else if (strcmp(acao, "listar") == 0) {
        // Se listar não tiver argumento, listamos o diretório atual
        printf("[Log] Executando logica para LISTAR: %s\n", arg1 ? arg1 : "diretorio atual");
        // -> fs_listar(arg1)
    }
    else if (strcmp(acao, "importar_arquivo") == 0) {
        if (arg1 != NULL && arg2 != NULL) {
            printf("[Log] Executando logica para IMPORTAR ARQUIVO.\n");
            printf("      Destino no simulador: %s\n", arg1);
            printf("      Arquivo real base: %s\n", arg2);
            // -> fs_importar_arquivo(arg1, arg2)
        } else {
            printf("Erro: Faltou argumentos (Ex: importar_arquivo /destino.txt origem.txt).\n");
        }
    }
    else {
        printf("Comando desconhecido ou nao implementado: '%s'\n", acao);
    }
}

// Modo 1: Leitura iterativa via terminal (tempo real)
void modo_interativo(SistemaArquivos *fs) {
    char comando[512];
    printf("\n--- Modo Interativo ---\n");
    printf("Digite os comandos (ou 'sair' para encerrar):\n");
    
    while (1) {
        printf("simulador-fs> ");
        if (fgets(comando, sizeof(comando), stdin) == NULL) {
            break;
        }
        comando[strcspn(comando, "\n")] = 0;
        if (strcmp(comando, "sair") == 0 || strcmp(comando, "exit") == 0) {
            break;
        }
        if (strlen(comando) > 0) {
            processar_comando(fs, comando);
        }
    }
}

// Modo 2: Leitura de comandos em lote (batch) através de um arquivo
void modo_arquivo(SistemaArquivos *fs, const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de comandos");
        return;
    }

    char linha[512];
    printf("\n--- Modo Arquivo (Lote) ---\n");
    printf("Lendo comandos do arquivo: %s\n", nome_arquivo);
    
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        linha[strcspn(linha, "\n")] = 0;
        
        if (strlen(linha) > 0) {
            processar_comando(fs, linha);
        }
    }

    fclose(arquivo);
}
