#ifndef FS_H
#define FS_H

#include <time.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_TAMANHO_NOME 255
#define BLOCOS_DIRETOS 12

typedef struct {
    uint32_t tamanho_particao;
    uint32_t tamanho_bloco;
    uint32_t quantidade_inodes;
    uint32_t inodes_livres;
    uint32_t quantidade_blocos;
    uint32_t blocos_livres;
    uint32_t inicio_bitmap_inodes;
    uint32_t inicio_bitmap_blocos;
    uint32_t inicio_tabela_inodes;
    uint32_t inicio_blocos_dados;
} Superbloco;

// Representa um i-node
typedef struct {
    uint32_t id;
    uint8_t eh_diretorio;
    uint32_t tamanho;
    time_t data_criacao;
    time_t data_modificacao;
    time_t data_ultimo_acesso;
    uint32_t blocos_diretos[BLOCOS_DIRETOS];
    uint32_t bloco_indireto;
    uint32_t bloco_indireto_duplo;
} Inode;

// Entrada de diretório
typedef struct {
    uint32_t id_inode;
    char nome[MAX_TAMANHO_NOME + 1];
} EntradaDiretorio;

// Controle do sistema de arquivos
typedef struct {
    FILE *arquivo_disco;
    Superbloco sb;
    uint8_t *bitmap_inodes;
    uint8_t *bitmap_blocos;
    uint32_t inode_diretorio_atual;
} SistemaArquivos;


#define MAX_TAMANHO_BLOCO 8192

typedef struct {
    uint32_t ponteiros[MAX_TAMANHO_BLOCO / 4];
} BlocoIndireto;

typedef struct {
    uint8_t dados[MAX_TAMANHO_BLOCO];
} BlocoDados;

// Inicialização e Sistema
void fs_formatar_disco(SistemaArquivos *fs, const char *caminho_arquivo_fisico);
void fs_montar_disco(SistemaArquivos *fs, const char *caminho_arquivo_fisico);

// Operações de Diretórios
void fs_criar_diretorio(SistemaArquivos *fs, const char *caminho);
void fs_renomear_diretorio(SistemaArquivos *fs, const char *caminho_antigo, const char *novo_nome);
void fs_apagar_diretorio(SistemaArquivos *fs, const char *caminho);
void fs_listar_conteudo(SistemaArquivos *fs, const char *caminho);
void fs_tree(SistemaArquivos *fs, const char *caminho);
void fs_status(SistemaArquivos *fs);
void fs_limpar_disco(SistemaArquivos *fs);

// Operações de Arquivos
void fs_criar_arquivo(SistemaArquivos *fs, const char *caminho);
void fs_renomear_arquivo(SistemaArquivos *fs, const char *caminho_antigo, const char *novo_nome);
void fs_mover_arquivo(SistemaArquivos *fs, const char *caminho_antigo, const char *novo_caminho);
void fs_apagar_arquivo(SistemaArquivos *fs, const char *caminho);

// Importação Externa
void fs_importar_arquivo(SistemaArquivos *fs, const char *caminho_simulado, const char *caminho_real);

#endif
