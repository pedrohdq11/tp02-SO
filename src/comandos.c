#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comandos.h"
#include "fs.h"

// Função para entender e redirecionar o comando digitado
void processar_comando(char *comando) {
    // 1. O strtok modifica a string original, então é bom trabalhar em uma cópia.
    char linha_copia[512];
    strncpy(linha_copia, comando, sizeof(linha_copia));

    // 2. Extrai a primeira palavra (a ação). Ex: "criar_dir"
    char *acao = strtok(linha_copia, " ");
    
    // Se o usuário só deu "Enter" sem digitar nada
    if (acao == NULL) {
        return; 
    }

    // 3. Extrai a segunda e terceira palavra (se existirem). Ex: "/docs" ou "arquivo.txt"
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
void modo_interativo() {
    char comando[512];
    printf("\n--- Modo Interativo ---\n");
    printf("Digite os comandos (ou 'sair' para encerrar):\n");
    
    while (1) {
        printf("simulador-fs> ");
        if (fgets(comando, sizeof(comando), stdin) == NULL) {
            break;
        }
        
        // Remover a quebra de linha do final
        comando[strcspn(comando, "\n")] = 0;
        
        if (strcmp(comando, "sair") == 0 || strcmp(comando, "exit") == 0) {
            break;
        }
        
        // Ignora linhas vazias
        if (strlen(comando) > 0) {
            processar_comando(comando);
        }
    }
}

// Modo 2: Leitura de comandos em lote (batch) através de um arquivo
void modo_arquivo(const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de comandos");
        return;
    }

    char linha[512];
    printf("\n--- Modo Arquivo (Lote) ---\n");
    printf("Lendo comandos do arquivo: %s\n", nome_arquivo);
    
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        // Remover a quebra de linha do final
        linha[strcspn(linha, "\n")] = 0;
        
        // Ignora linhas vazias
        if (strlen(linha) > 0) {
            processar_comando(linha);
        }
    }

    fclose(arquivo);
}
