#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comandos.h"
#include "fs.h"

// Processa comando digitado pelo usuário
void processar_comando(SistemaArquivos *fs, char *comando) {
    char linha_copia[512];
    strncpy(linha_copia, comando, sizeof(linha_copia));
    linha_copia[sizeof(linha_copia) - 1] = '\0';

    char *cursor = linha_copia;
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    if (*cursor == '\0' || *cursor == '#') {
        return;
    }

    char *acao = cursor;
    while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t') {
        cursor++;
    }
    if (*cursor != '\0') {
        *cursor++ = '\0';
    }

    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    char *arg1 = NULL;
    char *arg2 = NULL;
    if (*cursor != '\0' && *cursor != '#') {
        arg1 = cursor;
        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t') {
            cursor++;
        }
        if (*cursor != '\0') {
            *cursor++ = '\0';
        }

        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }

        if (*cursor != '\0' && *cursor != '#') {
            arg2 = cursor;
            char *fim = arg2;
            while (*fim != '\0' && *fim != '#') {
                fim++;
            }
            if (*fim == '#') {
                *fim = '\0';
            }
        }
    }

    if (strcmp(acao, "criar_dir") == 0) {
        if (arg1 != NULL) {
            fs_criar_diretorio(fs, arg1);
        } else {
            printf("Erro: Faltou o nome do diretorio (Ex: criar_dir /pasta).\n");
        }
    } else if (strcmp(acao, "apagar_dir") == 0) {
        if (arg1 != NULL) {
            fs_apagar_diretorio(fs, arg1);
        } else {
            printf("Erro: Faltou o nome do diretorio a ser apagado.\n");
        }
    } else if (strcmp(acao, "criar_arquivo") == 0) {
        if (arg1 != NULL) {
            fs_criar_arquivo(fs, arg1);
        } else {
            printf("Erro: Faltou o nome do arquivo (Ex: criar_arquivo /arquivo.txt).\n");
        }
    } else if (strcmp(acao, "apagar_arquivo") == 0) {
        if (arg1 != NULL) {
            fs_apagar_arquivo(fs, arg1);
        } else {
            printf("Erro: Faltou o nome do arquivo a ser apagado.\n");
        }
    } else if (strcmp(acao, "renomear_arquivo") == 0) {
        if (arg1 != NULL && arg2 != NULL) {
            fs_renomear_arquivo(fs, arg1, arg2);
        } else {
            printf("Erro: Faltou argumentos (Ex: renomear_arquivo /origem.txt novo_nome.txt).\n");
        }
    } else if (strcmp(acao, "mover_arquivo") == 0) {
        if (arg1 != NULL && arg2 != NULL) {
            fs_mover_arquivo(fs, arg1, arg2);
        } else {
            printf("Erro: Faltou argumentos (Ex: mover_arquivo /origem.txt /destino.txt).\n");
        }
    } else if (strcmp(acao, "renomear_dir") == 0) {
        if (arg1 != NULL && arg2 != NULL) {
            fs_renomear_diretorio(fs, arg1, arg2);
        } else {
            printf("Erro: Faltou argumentos (Ex: renomear_dir /origem/ novo_nome).\n");
        }
    } else if (strcmp(acao, "listar") == 0) {
        if (arg1 != NULL) {
            fs_listar_conteudo(fs, arg1);
        } else {
            fs_listar_conteudo(fs, NULL);
        }
    } else if (strcmp(acao, "tree") == 0 || strcmp(acao, "arvore") == 0) {
        if (arg1 != NULL) {
            fs_tree(fs, arg1);
        } else {
            fs_tree(fs, NULL);
        }
    } else if (strcmp(acao, "status") == 0) {
        fs_status(fs);
    } else if (strcmp(acao, "formatar") == 0) {
        fs_limpar_disco(fs);
    } else if (strcmp(acao, "importar_arquivo") == 0) {
        if (arg1 != NULL && arg2 != NULL) {
            fs_importar_arquivo(fs, arg1, arg2);
        } else {
            printf("Erro: Faltou argumentos (Ex: importar_arquivo /destino.txt origem.txt).\n");
        }
    } else {
        printf("Comando desconhecido ou nao implementado: '%s'\n", acao);
    }
}

// Modo interativo: lê comandos do terminal
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

// Modo arquivo: lê comandos de um arquivo
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
        
        if (strlen(linha) == 0 || linha[0] == '#') {
            continue;
        }
        if (strcmp(linha, "sair") == 0) {
            break;
        }
        processar_comando(fs, linha);
    }

    fclose(arquivo);
}
