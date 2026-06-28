#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs_common.h"

void fs_criar_diretorio(SistemaArquivos *fs, const char *caminho) {
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
        printf("Erro: nome ja existe no diretorio: %s\n", caminho);
        return;
    }

    uint32_t novo_inode = fs_criar_inode(fs, 1);
    if (novo_inode == INODE_INVALID) {
        printf("Erro: sem i-nodes livres para criar diretorio.\n");
        return;
    }

    Inode dir;
    fs_ler_inode(fs, novo_inode, &dir);
    uint32_t bloco = fs_buscar_bloco_livre(fs);
    if (bloco == INODE_INVALID) {
        printf("Erro: sem blocos livres para criar diretorio.\n");
        fs_bitmap_set(fs->bitmap_inodes, novo_inode, 0);
        fs->sb.inodes_livres++;
        fs_salvar_superbloco(fs);
        fs_salvar_bitmaps(fs);
        return;
    }

    fs_bitmap_set(fs->bitmap_blocos, bloco, 1);
    fs->sb.blocos_livres--;
    fs_salvar_superbloco(fs);
    fs_salvar_bitmaps(fs);

    dir.blocos_diretos[0] = bloco;
    dir.data_criacao = time(NULL);
    dir.data_modificacao = dir.data_criacao;
    dir.data_ultimo_acesso = dir.data_criacao;
    fs_escrever_inode(fs, &dir);

    fs_inicializar_bloco_diretorio(fs, bloco, novo_inode, pai_id);
    fs_atualizar_tamanho_diretorio(fs, &dir);
    fs_adicionar_entrada_diretorio(fs, &pai, novo_inode, nome);
    fs_escrever_inode(fs, &pai);
    printf("Diretorio criado: %s\n", caminho);
}

void fs_renomear_diretorio(SistemaArquivos *fs, const char *caminho_antigo, const char *novo_nome) {
    if (strchr(novo_nome, '/')) {
        printf("Erro: novo nome de diretorio nao pode conter '/'.\n");
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
        printf("Erro ao renomear diretorio: %s\n", caminho_antigo);
        return;
    }
    printf("Diretorio renomeado para %s\n", caminho_novo);
}

void fs_apagar_diretorio(SistemaArquivos *fs, const char *caminho) {
    if (strcmp(caminho, "/") == 0) {
        printf("Erro: nao e possivel apagar o diretorio raiz.\n");
        return;
    }

    uint32_t id = fs_resolver_caminho(fs, caminho);
    if (id == INODE_INVALID) {
        printf("Erro: diretorio nao encontrado: %s\n", caminho);
        return;
    }

    Inode dir;
    fs_ler_inode(fs, id, &dir);
    if (!dir.eh_diretorio) {
        printf("Erro: caminho nao e um diretorio: %s\n", caminho);
        return;
    }

    uint32_t entradas_por_bloco = fs->sb.tamanho_bloco / sizeof(EntradaDiretorio);
    EntradaDiretorio entrada;
    int conteudo = 0;
    for (int i = 0; i < BLOCOS_DIRETOS; ++i) {
        uint32_t bloco = dir.blocos_diretos[i];
        if (bloco == 0) {
            continue;
        }
        fseek(fs->arquivo_disco, fs_offset_bloco(fs, bloco), SEEK_SET);
        for (uint32_t j = 0; j < entradas_por_bloco; ++j) {
            fread(&entrada, sizeof(EntradaDiretorio), 1, fs->arquivo_disco);
            if (entrada.id_inode != INODE_INVALID && strcmp(entrada.nome, ".") != 0 && strcmp(entrada.nome, "..") != 0) {
                conteudo++;
            }
        }
    }
    if (conteudo > 0) {
        printf("Erro: diretorio nao esta vazio: %s\n", caminho);
        return;
    }

    uint32_t pai_id;
    char nome[MAX_TAMANHO_NOME + 1];
    if (!fs_resolver_caminho_pai(fs, caminho, &pai_id, nome)) {
        printf("Erro: pai do diretorio nao encontrado.\n");
        return;
    }

    Inode pai;
    fs_ler_inode(fs, pai_id, &pai);
    EntradaDiretorio entrada_pai;
    uint32_t bloco_id, indice;
    if (!fs_buscar_entrada(fs, &pai, nome, &entrada_pai, &bloco_id, &indice)) {
        printf("Erro: entrada do diretorio nao encontrada no pai.\n");
        return;
    }

    fs_remover_entrada_diretorio(fs, &pai, bloco_id, indice);
    fs_escrever_inode(fs, &pai);
    fs_liberar_inode(fs, id);
    printf("Diretorio apagado: %s\n", caminho);
}

void fs_listar_conteudo(SistemaArquivos *fs, const char *caminho) {
    uint32_t id = caminho ? fs_resolver_caminho(fs, caminho) : fs->inode_diretorio_atual;
    if (id == INODE_INVALID) {
        printf("Erro: caminho invalido para listar: %s\n", caminho ? caminho : "");
        return;
    }

    Inode inode;
    fs_ler_inode(fs, id, &inode);
    if (inode.eh_diretorio) {
        EntradaDiretorio entradas[256];
        int total = fs_ler_entradas_diretorio(fs, &inode, entradas, 256);
        printf("Conteudo do diretorio %s:\n", caminho ? caminho : "/");
        for (int i = 0; i < total; ++i) {
            Inode item;
            fs_ler_inode(fs, entradas[i].id_inode, &item);
            printf("  %s%s\n", entradas[i].nome, item.eh_diretorio ? "/" : "");
        }
        if (total == 0) {
            printf("  (vazio)\n");
        }
    } else {
        printf("Arquivo: %s, tamanho: %u bytes\n", caminho ? caminho : "<arquivo>", inode.tamanho);
    }
}

static void fs_imprimir_arvore_recursiva(SistemaArquivos *fs, uint32_t inode_id, int profundidade) {
    Inode inode;
    fs_ler_inode(fs, inode_id, &inode);
    if (!inode.eh_diretorio) {
        return;
    }

    EntradaDiretorio entradas[256];
    int total = fs_ler_entradas_diretorio(fs, &inode, entradas, 256);

    for (int i = 0; i < total; ++i) {
        for (int j = 0; j < profundidade; ++j) {
            printf("    ");
        }
        printf("%s", entradas[i].nome);

        Inode item;
        fs_ler_inode(fs, entradas[i].id_inode, &item);
        if (item.eh_diretorio) {
            printf("/");
        }
        printf("\n");

        if (item.eh_diretorio) {
            fs_imprimir_arvore_recursiva(fs, entradas[i].id_inode, profundidade + 1);
        }
    }
}

void fs_tree(SistemaArquivos *fs, const char *caminho) {
    uint32_t id = caminho ? fs_resolver_caminho(fs, caminho) : fs->inode_diretorio_atual;
    if (id == INODE_INVALID) {
        printf("Erro: caminho invalido para tree: %s\n", caminho ? caminho : "");
        return;
    }

    Inode inode;
    fs_ler_inode(fs, id, &inode);
    if (!inode.eh_diretorio) {
        printf("Erro: caminho nao e um diretorio para tree: %s\n", caminho ? caminho : "");
        return;
    }

    printf("Estrutura de diretorios %s:\n", caminho ? caminho : "/");
    fs_imprimir_arvore_recursiva(fs, id, 0);
}

void fs_status(SistemaArquivos *fs) {
    uint32_t blocos_usados = 0;
    uint32_t inodes_usados = 0;

    for (uint32_t i = 0; i < fs->sb.quantidade_blocos; ++i) {
        if (fs_bitmap_get(fs->bitmap_blocos, i)) {
            blocos_usados++;
        }
    }

    for (uint32_t i = 0; i < fs->sb.quantidade_inodes; ++i) {
        if (fs_bitmap_get(fs->bitmap_inodes, i)) {
            inodes_usados++;
        }
    }

    uint32_t blocos_livres = fs->sb.quantidade_blocos - blocos_usados;
    uint32_t inodes_livres = fs->sb.quantidade_inodes - inodes_usados;

    printf("=== Relatorio de Status do Sistema ===\n");
    printf("Capacidade Total do Disco: %u bytes\n", fs->sb.tamanho_particao);
    printf("Blocos em uso: %u / %u", blocos_usados, fs->sb.quantidade_blocos);
    if (fs->sb.quantidade_blocos > 0) {
        printf(" (%.0f%% ocupado)\n", (double)blocos_usados * 100.0 / fs->sb.quantidade_blocos);
    } else {
        printf(" (0%% ocupado)\n");
    }
    printf("I-nodes disponiveis: %u / %u\n", inodes_livres, fs->sb.quantidade_inodes);
    printf("Blocos livres: %u\n", blocos_livres);
    printf("I-nodes usados: %u\n", inodes_usados);
}

void fs_limpar_disco(SistemaArquivos *fs) {
    printf("Formatando disco... (apagando todos os dados)\n");

    uint32_t size_bitmap_inodes = fs_tamanho_bitmap(fs->sb.quantidade_inodes);
    uint32_t size_bitmap_blocos = fs_tamanho_bitmap(fs->sb.quantidade_blocos);

    memset(fs->bitmap_inodes, 0, size_bitmap_inodes);
    memset(fs->bitmap_blocos, 0, size_bitmap_blocos);

    fs->sb.blocos_livres = fs->sb.quantidade_blocos;
    fs->sb.inodes_livres = fs->sb.quantidade_inodes;

    fs_salvar_superbloco(fs);
    fs_salvar_bitmaps(fs);

    uint32_t raiz_id = fs_criar_inode(fs, 1);
    if (raiz_id == INODE_INVALID) {
        fprintf(stderr, "Erro ao criar inode raiz.\n");
        return;
    }

    Inode raiz;
    fs_ler_inode(fs, raiz_id, &raiz);
    uint32_t bloco_raiz = fs_buscar_bloco_livre(fs);
    if (bloco_raiz == INODE_INVALID) {
        fprintf(stderr, "Erro: sem blocos livres para criar diretorio raiz.\n");
        return;
    }

    fs_bitmap_set(fs->bitmap_blocos, bloco_raiz, 1);
    fs->sb.blocos_livres--;
    fs_salvar_superbloco(fs);
    fs_salvar_bitmaps(fs);

    raiz.blocos_diretos[0] = bloco_raiz;
    raiz.data_criacao = time(NULL);
    raiz.data_modificacao = raiz.data_criacao;
    raiz.data_ultimo_acesso = raiz.data_criacao;
    fs_escrever_inode(fs, &raiz);

    fs_inicializar_bloco_diretorio(fs, bloco_raiz, raiz_id, raiz_id);
    fs_atualizar_tamanho_diretorio(fs, &raiz);
    fs_escrever_inode(fs, &raiz);

    fs->inode_diretorio_atual = 0;
    fflush(fs->arquivo_disco);
    printf("Disco formatado com sucesso!\n");
}
