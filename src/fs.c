#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

// ----------------------------------------------------------------------------
// Inicialização e Sistema
// ----------------------------------------------------------------------------

void fs_formatar_disco(SistemaArquivos *fs, const char *caminho_arquivo_fisico) {
    fs->arquivo_disco = fopen(caminho_arquivo_fisico, "wb+");
    if (!fs->arquivo_disco) {
        perror("Erro ao criar o arquivo fisico do disco");
        exit(EXIT_FAILURE);
    }
    
    if (fs->sb.quantidade_inodes == 0) {
        fs->sb.quantidade_inodes = fs->sb.tamanho_particao / fs->sb.tamanho_bloco;
    }
    
    uint32_t size_bitmap_inodes = (fs->sb.quantidade_inodes + 7) / 8;
    uint32_t espaco_metadados = sizeof(Superbloco) + size_bitmap_inodes + (fs->sb.quantidade_inodes * sizeof(Inode));
    
    if (espaco_metadados >= fs->sb.tamanho_particao) {
        printf("Erro: O tamanho da particao é muito pequeno para comportar os metadados.\n");
        exit(EXIT_FAILURE);
    }
    
    uint32_t espaco_livre = fs->sb.tamanho_particao - espaco_metadados;
    fs->sb.quantidade_blocos = (espaco_livre * 8) / (fs->sb.tamanho_bloco * 8 + 1);
    
    fs->sb.blocos_livres = fs->sb.quantidade_blocos;
    fs->sb.inodes_livres = fs->sb.quantidade_inodes;
    
    uint32_t size_bitmap_blocos = (fs->sb.quantidade_blocos + 7) / 8;
    
    fs->sb.inicio_bitmap_inodes = sizeof(Superbloco);
    fs->sb.inicio_bitmap_blocos = fs->sb.inicio_bitmap_inodes + size_bitmap_inodes;
    fs->sb.inicio_tabela_inodes = fs->sb.inicio_bitmap_blocos + size_bitmap_blocos;
    fs->sb.inicio_blocos_dados = fs->sb.inicio_tabela_inodes + (fs->sb.quantidade_inodes * sizeof(Inode));
    
    fseek(fs->arquivo_disco, 0, SEEK_SET);
    fwrite(&fs->sb, sizeof(Superbloco), 1, fs->arquivo_disco);
    
    fs->bitmap_inodes = calloc(size_bitmap_inodes, 1);
    fs->bitmap_blocos = calloc(size_bitmap_blocos, 1);
    
    fwrite(fs->bitmap_inodes, 1, size_bitmap_inodes, fs->arquivo_disco);
    fwrite(fs->bitmap_blocos, 1, size_bitmap_blocos, fs->arquivo_disco);
    
    fseek(fs->arquivo_disco, fs->sb.tamanho_particao - 1, SEEK_SET);
    fputc(0, fs->arquivo_disco);
    
    fs->inode_diretorio_atual = 0; 
    
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
    
    uint32_t size_bitmap_inodes = (fs->sb.quantidade_inodes + 7) / 8;
    uint32_t size_bitmap_blocos = (fs->sb.quantidade_blocos + 7) / 8;
    
    fs->bitmap_inodes = malloc(size_bitmap_inodes);
    fs->bitmap_blocos = malloc(size_bitmap_blocos);
    
    fseek(fs->arquivo_disco, fs->sb.inicio_bitmap_inodes, SEEK_SET);
    fread(fs->bitmap_inodes, 1, size_bitmap_inodes, fs->arquivo_disco);
    
    fseek(fs->arquivo_disco, fs->sb.inicio_bitmap_blocos, SEEK_SET);
    fread(fs->bitmap_blocos, 1, size_bitmap_blocos, fs->arquivo_disco);
    
    fs->inode_diretorio_atual = 0;
    
    printf("[FS] Disco montado com sucesso! Arquivo: %s\n", caminho_arquivo_fisico);
}

void fs_criar_diretorio(SistemaArquivos *fs, const char *caminho) {
}

void fs_renomear_diretorio(SistemaArquivos *fs, const char *caminho_antigo, const char *novo_nome) {
}

void fs_apagar_diretorio(SistemaArquivos *fs, const char *caminho) {
}

void fs_listar_conteudo(SistemaArquivos *fs, const char *caminho) {
}
void fs_criar_arquivo(SistemaArquivos *fs, const char *caminho) {
}

void fs_renomear_arquivo(SistemaArquivos *fs, const char *caminho_antigo, const char *novo_nome) {
}
void fs_mover_arquivo(SistemaArquivos *fs, const char *caminho_antigo, const char *novo_caminho) {
}
void fs_apagar_arquivo(SistemaArquivos *fs, const char *caminho) {
}
void fs_importar_arquivo(SistemaArquivos *fs, const char *caminho_simulado, const char *caminho_real) {
}
