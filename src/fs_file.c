#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs_common.h"

static int fs_criar_arquivo_com_dados(SistemaArquivos *fs, uint32_t inode_id, FILE *origem) {
    Inode inode;
    fs_ler_inode(fs, inode_id, &inode);

    uint32_t bloco_tamanho = fs->sb.tamanho_bloco;
    uint8_t *buffer = malloc(bloco_tamanho);
    if (!buffer) {
        return 0;
    }

    uint32_t blocos_usados = 0;
    size_t lido;
    while ((lido = fread(buffer, 1, bloco_tamanho, origem)) > 0) {
        uint32_t bloco_dado = fs_buscar_bloco_livre(fs);
        if (bloco_dado == INODE_INVALID) {
            free(buffer);
            return 0;
        }

        fs_bitmap_set(fs->bitmap_blocos, bloco_dado, 1);
        fs->sb.blocos_livres--;
        fs_salvar_superbloco(fs);
        fs_salvar_bitmaps(fs);

        fs_escrever_bloco(fs, bloco_dado, buffer);
        if (blocos_usados < BLOCOS_DIRETOS) {
            inode.blocos_diretos[blocos_usados] = bloco_dado;
        } else {
            if (inode.bloco_indireto == 0) {
                uint32_t bloco_indireto = fs_buscar_bloco_livre(fs);
                if (bloco_indireto == INODE_INVALID) {
                    free(buffer);
                    return 0;
                }
                fs_bitmap_set(fs->bitmap_blocos, bloco_indireto, 1);
                fs->sb.blocos_livres--;
                fs_salvar_superbloco(fs);
                fs_salvar_bitmaps(fs);
                inode.bloco_indireto = bloco_indireto;
                uint32_t ponteiros_zeros[fs->sb.tamanho_bloco / sizeof(uint32_t)];
                memset(ponteiros_zeros, 0, sizeof(ponteiros_zeros));
                fs_escrever_bloco(fs, bloco_indireto, ponteiros_zeros);
            }
            uint32_t ponteiros_por_bloco = fs->sb.tamanho_bloco / sizeof(uint32_t);
            uint32_t indice_indireto = blocos_usados - BLOCOS_DIRETOS;
            if (indice_indireto >= ponteiros_por_bloco) {
                free(buffer);
                return 0;
            }
            uint32_t *ponteiros = malloc(ponteiros_por_bloco * sizeof(uint32_t));
            if (!ponteiros) {
                free(buffer);
                return 0;
            }
            fs_ler_bloco(fs, inode.bloco_indireto, ponteiros);
            ponteiros[indice_indireto] = bloco_dado;
            fs_escrever_bloco(fs, inode.bloco_indireto, ponteiros);
            free(ponteiros);
        }

        inode.tamanho += lido;
        inode.data_modificacao = time(NULL);
        blocos_usados++;
    }

    free(buffer);
    inode.data_ultimo_acesso = time(NULL);
    fs_escrever_inode(fs, &inode);
    return 1;
}

void fs_criar_arquivo(SistemaArquivos *fs, const char *caminho) {
    uint32_t pai_id;
    char nome[MAX_TAMANHO_NOME + 1];
    if (!fs_resolver_caminho_pai(fs, caminho, &pai_id, nome)) {
        printf("Erro: caminho invalido ou pai nao encontrado: %s\n", caminho);
        return;
    }

    Inode pai;
    fs_ler_inode(fs, pai_id, &pai);
    if (!pai.eh_diretorio) {
        printf("Erro: destino nao e um diretorio: %s\n", caminho);
        return;
    }

    EntradaDiretorio existente;
    if (fs_buscar_entrada(fs, &pai, nome, &existente, NULL, NULL)) {
        printf("Erro: arquivo ou diretorio ja existe: %s\n", caminho);
        return;
    }

    uint32_t novo_inode = fs_criar_inode(fs, 0);
    if (novo_inode == INODE_INVALID) {
        printf("Erro: sem i-nodes livres para criar arquivo.\n");
        return;
    }

    if (!fs_adicionar_entrada_diretorio(fs, &pai, novo_inode, nome)) {
        printf("Erro: nao foi possivel adicionar entrada de arquivo.\n");
        fs_bitmap_set(fs->bitmap_inodes, novo_inode, 0);
        fs->sb.inodes_livres++;
        fs_salvar_superbloco(fs);
        fs_salvar_bitmaps(fs);
        return;
    }

    fs_escrever_inode(fs, &pai);
    printf("Arquivo criado: %s\n", caminho);
}

void fs_renomear_arquivo(SistemaArquivos *fs, const char *caminho_antigo, const char *novo_nome) {
    if (strchr(novo_nome, '/')) {
        printf("Erro: novo nome nao pode conter '/'.\n");
        return;
    }
    char caminho_novo[512];
    strncpy(caminho_novo, caminho_antigo, sizeof(caminho_novo) - 1);
    caminho_novo[sizeof(caminho_novo) - 1] = '\0';
    char *ultimo_slash = strrchr(caminho_novo, '/');
    if (ultimo_slash) {
        *(ultimo_slash + 1) = '\0';
        strncat(caminho_novo, novo_nome, sizeof(caminho_novo) - strlen(caminho_novo) - 1);
    } else {
        snprintf(caminho_novo, sizeof(caminho_novo), "%s", novo_nome);
    }
    if (!fs_mover_entrada(fs, caminho_antigo, caminho_novo)) {
        printf("Erro ao renomear arquivo: %s\n", caminho_antigo);
        return;
    }
    printf("Arquivo renomeado para %s\n", caminho_novo);
}

void fs_mover_arquivo(SistemaArquivos *fs, const char *caminho_antigo, const char *novo_caminho) {
    if (!fs_mover_entrada(fs, caminho_antigo, novo_caminho)) {
        printf("Erro ao mover arquivo ou diretorio: %s\n", caminho_antigo);
        return;
    }
    printf("Movido para: %s\n", novo_caminho);
}

void fs_apagar_arquivo(SistemaArquivos *fs, const char *caminho) {
    uint32_t id = fs_resolver_caminho(fs, caminho);
    if (id == INODE_INVALID) {
        printf("Erro: arquivo nao encontrado: %s\n", caminho);
        return;
    }

    Inode inode;
    fs_ler_inode(fs, id, &inode);
    if (inode.eh_diretorio) {
        printf("Erro: caminho e um diretorio, use apagar_dir: %s\n", caminho);
        return;
    }

    uint32_t pai_id;
    char nome[MAX_TAMANHO_NOME + 1];
    if (!fs_resolver_caminho_pai(fs, caminho, &pai_id, nome)) {
        printf("Erro: pai do arquivo nao encontrado.\n");
        return;
    }

    Inode pai;
    fs_ler_inode(fs, pai_id, &pai);
    EntradaDiretorio entrada;
    uint32_t bloco_id, indice;
    if (!fs_buscar_entrada(fs, &pai, nome, &entrada, &bloco_id, &indice)) {
        printf("Erro: entrada do arquivo nao encontrada no pai.\n");
        return;
    }

    fs_remover_entrada_diretorio(fs, &pai, bloco_id, indice);
    fs_escrever_inode(fs, &pai);
    fs_liberar_inode(fs, id);
    printf("Arquivo apagado: %s\n", caminho);
}

void fs_importar_arquivo(SistemaArquivos *fs, const char *caminho_simulado, const char *caminho_real) {
    FILE *origem = fopen(caminho_real, "rb");
    if (!origem) {
        printf("Erro: nao foi possivel abrir arquivo real: %s\n", caminho_real);
        return;
    }

    if (fs_resolver_caminho(fs, caminho_simulado) != INODE_INVALID) {
        printf("Erro: caminho simulado ja existe: %s\n", caminho_simulado);
        fclose(origem);
        return;
    }

    uint32_t pai_id;
    char nome[MAX_TAMANHO_NOME + 1];
    if (!fs_resolver_caminho_pai(fs, caminho_simulado, &pai_id, nome)) {
        printf("Erro: caminho simulado invalido: %s\n", caminho_simulado);
        fclose(origem);
        return;
    }

    Inode pai;
    fs_ler_inode(fs, pai_id, &pai);
    if (!pai.eh_diretorio) {
        printf("Erro: pai do caminho simulado nao e um diretorio.\n");
        fclose(origem);
        return;
    }

    EntradaDiretorio existente;
    if (fs_buscar_entrada(fs, &pai, nome, &existente, NULL, NULL)) {
        printf("Erro: ja existe arquivo ou diretorio com este nome: %s\n", nome);
        fclose(origem);
        return;
    }

    uint32_t novo_inode = fs_criar_inode(fs, 0);
    if (novo_inode == INODE_INVALID) {
        printf("Erro: sem i-nodes livres para importar arquivo.\n");
        fclose(origem);
        return;
    }

    if (!fs_adicionar_entrada_diretorio(fs, &pai, novo_inode, nome)) {
        printf("Erro: nao foi possivel adicionar arquivo ao diretorio.\n");
        fs_bitmap_set(fs->bitmap_inodes, novo_inode, 0);
        fs->sb.inodes_livres++;
        fs_salvar_superbloco(fs);
        fs_salvar_bitmaps(fs);
        fclose(origem);
        return;
    }

    fs_escrever_inode(fs, &pai);
    if (!fs_criar_arquivo_com_dados(fs, novo_inode, origem)) {
        printf("Erro: nao foi possivel gravar dados do arquivo.\n");
        fs_apagar_arquivo(fs, caminho_simulado);
        fclose(origem);
        return;
    }

    fclose(origem);
    printf("Arquivo importado: %s -> %s\n", caminho_real, caminho_simulado);
}
