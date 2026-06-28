#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs_common.h"

void fs_formatar_disco(SistemaArquivos *fs, const char *caminho_arquivo_fisico) {
    fs->arquivo_disco = fopen(caminho_arquivo_fisico, "wb+");
    if (!fs->arquivo_disco) {
        perror("Erro ao criar o arquivo fisico do disco");
        exit(EXIT_FAILURE);
    }

    if (fs->sb.quantidade_inodes == 0) {
        fs->sb.quantidade_inodes = fs->sb.tamanho_particao / fs->sb.tamanho_bloco;
    }

    uint32_t size_bitmap_inodes = fs_tamanho_bitmap(fs->sb.quantidade_inodes);
    uint32_t quantidade_blocos = (fs->sb.tamanho_particao - sizeof(Superbloco) - size_bitmap_inodes - fs->sb.quantidade_inodes * sizeof(Inode)) / fs->sb.tamanho_bloco;
    uint32_t size_bitmap_blocos = fs_tamanho_bitmap(quantidade_blocos);
    while (1) {
        uint32_t nova_quantidade = (fs->sb.tamanho_particao - sizeof(Superbloco) - size_bitmap_inodes - fs->sb.quantidade_inodes * sizeof(Inode) - size_bitmap_blocos) / fs->sb.tamanho_bloco;
        if (nova_quantidade == quantidade_blocos) {
            break;
        }
        quantidade_blocos = nova_quantidade;
        size_bitmap_blocos = fs_tamanho_bitmap(quantidade_blocos);
    }

    fs->sb.quantidade_blocos = quantidade_blocos;
    fs->sb.blocos_livres = fs->sb.quantidade_blocos;
    fs->sb.inodes_livres = fs->sb.quantidade_inodes;

    fs->sb.inicio_bitmap_inodes = sizeof(Superbloco);
    fs->sb.inicio_bitmap_blocos = fs->sb.inicio_bitmap_inodes + size_bitmap_inodes;
    fs->sb.inicio_tabela_inodes = fs->sb.inicio_bitmap_blocos + size_bitmap_blocos;
    fs->sb.inicio_blocos_dados = fs->sb.inicio_tabela_inodes + fs->sb.quantidade_inodes * sizeof(Inode);

    fseek(fs->arquivo_disco, 0, SEEK_SET);
    fwrite(&fs->sb, sizeof(Superbloco), 1, fs->arquivo_disco);

    fs->bitmap_inodes = calloc(size_bitmap_inodes, 1);
    fs->bitmap_blocos = calloc(size_bitmap_blocos, 1);
    fwrite(fs->bitmap_inodes, 1, size_bitmap_inodes, fs->arquivo_disco);
    fwrite(fs->bitmap_blocos, 1, size_bitmap_blocos, fs->arquivo_disco);

    fseek(fs->arquivo_disco, fs->sb.tamanho_particao - 1, SEEK_SET);
    fputc(0, fs->arquivo_disco);

    fs->inode_diretorio_atual = 0;
    fs_salvar_superbloco(fs);
    fs_salvar_bitmaps(fs);

    uint32_t raiz_id = fs_criar_inode(fs, 1);
    if (raiz_id == INODE_INVALID) {
        fprintf(stderr, "Erro ao criar inode raiz.\n");
        exit(EXIT_FAILURE);
    }

    Inode raiz;
    fs_ler_inode(fs, raiz_id, &raiz);
    uint32_t bloco_raiz = fs_buscar_bloco_livre(fs);
    if (bloco_raiz == INODE_INVALID) {
        fprintf(stderr, "Erro: sem blocos livres para criar diretorio raiz.\n");
        exit(EXIT_FAILURE);
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

    fflush(fs->arquivo_disco);
    printf("[FS] Disco formatado com sucesso! Arquivo: %s\n", caminho_arquivo_fisico);
}

void fs_montar_disco(SistemaArquivos *fs, const char *caminho_arquivo_fisico) {
    fs->arquivo_disco = fopen(caminho_arquivo_fisico, "rb+");
    if (!fs->arquivo_disco) {
        perror("Erro ao abrir o disco (arquivo fisico nao encontrado)");
        exit(EXIT_FAILURE);
    }

    fseek(fs->arquivo_disco, 0, SEEK_SET);
    fread(&fs->sb, sizeof(Superbloco), 1, fs->arquivo_disco);

    uint32_t size_bitmap_inodes = fs_tamanho_bitmap(fs->sb.quantidade_inodes);
    uint32_t size_bitmap_blocos = fs_tamanho_bitmap(fs->sb.quantidade_blocos);

    fs->bitmap_inodes = malloc(size_bitmap_inodes);
    fs->bitmap_blocos = malloc(size_bitmap_blocos);

    fseek(fs->arquivo_disco, fs->sb.inicio_bitmap_inodes, SEEK_SET);
    fread(fs->bitmap_inodes, 1, size_bitmap_inodes, fs->arquivo_disco);

    fseek(fs->arquivo_disco, fs->sb.inicio_bitmap_blocos, SEEK_SET);
    fread(fs->bitmap_blocos, 1, size_bitmap_blocos, fs->arquivo_disco);

    fs->inode_diretorio_atual = 0;
    printf("[FS] Disco montado com sucesso! Arquivo: %s\n", caminho_arquivo_fisico);
}
