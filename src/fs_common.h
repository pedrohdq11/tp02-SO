#ifndef FS_COMMON_H
#define FS_COMMON_H

#include "fs.h"

#define INODE_INVALID UINT32_MAX

uint32_t fs_tamanho_bitmap(uint32_t quantidade);
int fs_bitmap_get(const uint8_t *bitmap, uint32_t index);
void fs_bitmap_set(uint8_t *bitmap, uint32_t index, int value);

uint32_t fs_offset_inode(SistemaArquivos *fs, uint32_t inode_id);
uint32_t fs_offset_bloco(SistemaArquivos *fs, uint32_t bloco_id);

void fs_salvar_superbloco(SistemaArquivos *fs);
void fs_salvar_bitmaps(SistemaArquivos *fs);
void fs_escrever_inode(SistemaArquivos *fs, const Inode *inode);
void fs_ler_inode(SistemaArquivos *fs, uint32_t inode_id, Inode *inode);

uint32_t fs_buscar_inode_livre(SistemaArquivos *fs);
uint32_t fs_buscar_bloco_livre(SistemaArquivos *fs);
int fs_ler_bloco(SistemaArquivos *fs, uint32_t bloco_id, void *dados);
int fs_escrever_bloco(SistemaArquivos *fs, uint32_t bloco_id, const void *dados);

void fs_atualizar_tamanho_diretorio(SistemaArquivos *fs, Inode *dir);
void fs_inicializar_bloco_vazio(SistemaArquivos *fs, uint32_t bloco_id);
void fs_inicializar_bloco_diretorio(SistemaArquivos *fs, uint32_t bloco_id, uint32_t self_id, uint32_t parent_id);

int fs_buscar_entrada(SistemaArquivos *fs, const Inode *dir, const char *nome, EntradaDiretorio *saida, uint32_t *bloco_id, uint32_t *indice);
int fs_adicionar_entrada_diretorio(SistemaArquivos *fs, Inode *dir, uint32_t inode_id, const char *nome);
int fs_remover_entrada_diretorio(SistemaArquivos *fs, Inode *dir, uint32_t bloco_id, uint32_t indice);
int fs_ler_entradas_diretorio(SistemaArquivos *fs, const Inode *dir, EntradaDiretorio *resultado, uint32_t max);

uint32_t fs_criar_inode(SistemaArquivos *fs, int eh_diretorio);
void fs_liberar_bloco(SistemaArquivos *fs, uint32_t bloco_id);
void fs_liberar_inodes_e_blocos(SistemaArquivos *fs, Inode *inode);
void fs_liberar_inode(SistemaArquivos *fs, uint32_t inode_id);

uint32_t fs_resolver_caminho(SistemaArquivos *fs, const char *caminho);
int fs_resolver_caminho_pai(SistemaArquivos *fs, const char *caminho, uint32_t *pai_id, char *nome_final);
int fs_mover_entrada(SistemaArquivos *fs, const char *origem, const char *destino);

#endif // FS_COMMON_H
