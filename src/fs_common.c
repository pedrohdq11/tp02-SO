#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <limits.h>
#include "fs_common.h"

uint32_t fs_tamanho_bitmap(uint32_t quantidade) {
    return (quantidade + 7) / 8;
}

int fs_bitmap_get(const uint8_t *bitmap, uint32_t index) {
    return (bitmap[index / 8] >> (index % 8)) & 1;
}

void fs_bitmap_set(uint8_t *bitmap, uint32_t index, int value) {
    uint8_t mask = 1 << (index % 8);
    if (value) {
        bitmap[index / 8] |= mask;
    } else {
        bitmap[index / 8] &= ~mask;
    }
}

uint32_t fs_offset_inode(SistemaArquivos *fs, uint32_t inode_id) {
    return fs->sb.inicio_tabela_inodes + inode_id * sizeof(Inode);
}

uint32_t fs_offset_bloco(SistemaArquivos *fs, uint32_t bloco_id) {
    return fs->sb.inicio_blocos_dados + bloco_id * fs->sb.tamanho_bloco;
}

void fs_salvar_superbloco(SistemaArquivos *fs) {
    fseek(fs->arquivo_disco, 0, SEEK_SET);
    fwrite(&fs->sb, sizeof(Superbloco), 1, fs->arquivo_disco);
    fflush(fs->arquivo_disco);
}

void fs_salvar_bitmaps(SistemaArquivos *fs) {
    uint32_t size_bitmap_inodes = fs_tamanho_bitmap(fs->sb.quantidade_inodes);
    uint32_t size_bitmap_blocos = fs_tamanho_bitmap(fs->sb.quantidade_blocos);

    fseek(fs->arquivo_disco, fs->sb.inicio_bitmap_inodes, SEEK_SET);
    fwrite(fs->bitmap_inodes, 1, size_bitmap_inodes, fs->arquivo_disco);

    fseek(fs->arquivo_disco, fs->sb.inicio_bitmap_blocos, SEEK_SET);
    fwrite(fs->bitmap_blocos, 1, size_bitmap_blocos, fs->arquivo_disco);
    fflush(fs->arquivo_disco);
}

void fs_escrever_inode(SistemaArquivos *fs, const Inode *inode) {
    fseek(fs->arquivo_disco, fs_offset_inode(fs, inode->id), SEEK_SET);
    fwrite(inode, sizeof(Inode), 1, fs->arquivo_disco);
    fflush(fs->arquivo_disco);
}

void fs_ler_inode(SistemaArquivos *fs, uint32_t inode_id, Inode *inode) {
    fseek(fs->arquivo_disco, fs_offset_inode(fs, inode_id), SEEK_SET);
    fread(inode, sizeof(Inode), 1, fs->arquivo_disco);
}

uint32_t fs_buscar_inode_livre(SistemaArquivos *fs) {
    for (uint32_t i = 0; i < fs->sb.quantidade_inodes; ++i) {
        if (!fs_bitmap_get(fs->bitmap_inodes, i)) {
            return i;
        }
    }
    return INODE_INVALID;
}

uint32_t fs_buscar_bloco_livre(SistemaArquivos *fs) {
    for (uint32_t i = 0; i < fs->sb.quantidade_blocos; ++i) {
        if (!fs_bitmap_get(fs->bitmap_blocos, i)) {
            return i;
        }
    }
    return INODE_INVALID;
}

int fs_ler_bloco(SistemaArquivos *fs, uint32_t bloco_id, void *dados) {
    fseek(fs->arquivo_disco, fs_offset_bloco(fs, bloco_id), SEEK_SET);
    return fread(dados, 1, fs->sb.tamanho_bloco, fs->arquivo_disco) == fs->sb.tamanho_bloco;
}

int fs_escrever_bloco(SistemaArquivos *fs, uint32_t bloco_id, const void *dados) {
    fseek(fs->arquivo_disco, fs_offset_bloco(fs, bloco_id), SEEK_SET);
    return fwrite(dados, 1, fs->sb.tamanho_bloco, fs->arquivo_disco) == fs->sb.tamanho_bloco;
}

void fs_atualizar_tamanho_diretorio(SistemaArquivos *fs, Inode *dir) {
    uint32_t entradas_por_bloco = fs->sb.tamanho_bloco / sizeof(EntradaDiretorio);
    EntradaDiretorio entrada;
    uint32_t total = 0;

    for (int i = 0; i < BLOCOS_DIRETOS; ++i) {
        uint32_t bloco = dir->blocos_diretos[i];
        if (bloco == 0) {
            continue;
        }

        fseek(fs->arquivo_disco, fs_offset_bloco(fs, bloco), SEEK_SET);
        for (uint32_t j = 0; j < entradas_por_bloco; ++j) {
            fread(&entrada, sizeof(EntradaDiretorio), 1, fs->arquivo_disco);
            if (entrada.id_inode != INODE_INVALID) {
                total += sizeof(EntradaDiretorio);
            }
        }
    }

    dir->tamanho = total;
    fs_escrever_inode(fs, dir);
}

void fs_inicializar_bloco_vazio(SistemaArquivos *fs, uint32_t bloco_id) {
    uint32_t entradas_por_bloco = fs->sb.tamanho_bloco / sizeof(EntradaDiretorio);
    EntradaDiretorio *entradas = calloc(entradas_por_bloco, sizeof(EntradaDiretorio));
    if (!entradas) {
        return;
    }
    for (uint32_t i = 0; i < entradas_por_bloco; ++i) {
        entradas[i].id_inode = INODE_INVALID;
        entradas[i].nome[0] = '\0';
    }
    fs_escrever_bloco(fs, bloco_id, entradas);
    free(entradas);
}

void fs_inicializar_bloco_diretorio(SistemaArquivos *fs, uint32_t bloco_id, uint32_t self_id, uint32_t parent_id) {
    uint32_t entradas_por_bloco = fs->sb.tamanho_bloco / sizeof(EntradaDiretorio);
    EntradaDiretorio *entradas = calloc(entradas_por_bloco, sizeof(EntradaDiretorio));
    if (!entradas) {
        return;
    }
    for (uint32_t i = 0; i < entradas_por_bloco; ++i) {
        entradas[i].id_inode = INODE_INVALID;
        entradas[i].nome[0] = '\0';
    }
    entradas[0].id_inode = self_id;
    strncpy(entradas[0].nome, ".", MAX_TAMANHO_NOME);
    entradas[0].nome[MAX_TAMANHO_NOME] = '\0';
    entradas[1].id_inode = parent_id;
    strncpy(entradas[1].nome, "..", MAX_TAMANHO_NOME);
    entradas[1].nome[MAX_TAMANHO_NOME] = '\0';
    fs_escrever_bloco(fs, bloco_id, entradas);
    free(entradas);
}

int fs_buscar_entrada(SistemaArquivos *fs, const Inode *dir, const char *nome, EntradaDiretorio *saida, uint32_t *bloco_id, uint32_t *indice) {
    if (!dir->eh_diretorio) {
        return 0;
    }

    uint32_t entradas_por_bloco = fs->sb.tamanho_bloco / sizeof(EntradaDiretorio);
    EntradaDiretorio entrada;

    for (int i = 0; i < BLOCOS_DIRETOS; ++i) {
        uint32_t bloco = dir->blocos_diretos[i];
        if (bloco == 0) {
            continue;
        }
        fseek(fs->arquivo_disco, fs_offset_bloco(fs, bloco), SEEK_SET);
        for (uint32_t j = 0; j < entradas_por_bloco; ++j) {
            fread(&entrada, sizeof(EntradaDiretorio), 1, fs->arquivo_disco);
            if (entrada.id_inode == INODE_INVALID) {
                continue;
            }
            if (strcmp(entrada.nome, nome) == 0) {
                if (saida) {
                    *saida = entrada;
                }
                if (bloco_id) {
                    *bloco_id = bloco;
                }
                if (indice) {
                    *indice = j;
                }
                return 1;
            }
        }
    }
    return 0;
}

int fs_adicionar_entrada_diretorio(SistemaArquivos *fs, Inode *dir, uint32_t inode_id, const char *nome) {
    if (!dir->eh_diretorio || strlen(nome) > MAX_TAMANHO_NOME) {
        return 0;
    }

    EntradaDiretorio entrada;
    entrada.id_inode = inode_id;
    strncpy(entrada.nome, nome, MAX_TAMANHO_NOME);
    entrada.nome[MAX_TAMANHO_NOME] = '\0';

    uint32_t entradas_por_bloco = fs->sb.tamanho_bloco / sizeof(EntradaDiretorio);
    EntradaDiretorio *bloco_entradas = malloc(entradas_por_bloco * sizeof(EntradaDiretorio));
    if (!bloco_entradas) {
        return 0;
    }

    for (int i = 0; i < BLOCOS_DIRETOS; ++i) {
        uint32_t bloco = dir->blocos_diretos[i];
        if (bloco == 0) {
            continue;
        }
        fseek(fs->arquivo_disco, fs_offset_bloco(fs, bloco), SEEK_SET);
        fread(bloco_entradas, sizeof(EntradaDiretorio), entradas_por_bloco, fs->arquivo_disco);
        for (uint32_t j = 0; j < entradas_por_bloco; ++j) {
            if (bloco_entradas[j].id_inode == INODE_INVALID) {
                bloco_entradas[j] = entrada;
                fseek(fs->arquivo_disco, fs_offset_bloco(fs, bloco) + j * sizeof(EntradaDiretorio), SEEK_SET);
                fwrite(&entrada, sizeof(EntradaDiretorio), 1, fs->arquivo_disco);
                fs_atualizar_tamanho_diretorio(fs, dir);
                free(bloco_entradas);
                return 1;
            }
        }
    }

    for (int i = 0; i < BLOCOS_DIRETOS; ++i) {
        if (dir->blocos_diretos[i] == 0) {
            uint32_t novo_bloco = fs_buscar_bloco_livre(fs);
            if (novo_bloco == INODE_INVALID) {
                free(bloco_entradas);
                return 0;
            }
            fs_bitmap_set(fs->bitmap_blocos, novo_bloco, 1);
            fs->sb.blocos_livres--;
            fs_salvar_superbloco(fs);
            fs_salvar_bitmaps(fs);

            dir->blocos_diretos[i] = novo_bloco;
            fs_inicializar_bloco_vazio(fs, novo_bloco);
            fseek(fs->arquivo_disco, fs_offset_bloco(fs, novo_bloco), SEEK_SET);
            fwrite(&entrada, sizeof(EntradaDiretorio), 1, fs->arquivo_disco);
            fs_atualizar_tamanho_diretorio(fs, dir);
            fs_escrever_inode(fs, dir);
            free(bloco_entradas);
            return 1;
        }
    }

    free(bloco_entradas);
    return 0;
}

int fs_remover_entrada_diretorio(SistemaArquivos *fs, Inode *dir, uint32_t bloco_id, uint32_t indice) {
    if (!dir->eh_diretorio) {
        return 0;
    }

    EntradaDiretorio entrada;
    fseek(fs->arquivo_disco, fs_offset_bloco(fs, bloco_id) + indice * sizeof(EntradaDiretorio), SEEK_SET);
    fread(&entrada, sizeof(EntradaDiretorio), 1, fs->arquivo_disco);
    if (entrada.id_inode == INODE_INVALID) {
        return 0;
    }

    entrada.id_inode = INODE_INVALID;
    entrada.nome[0] = '\0';
    fseek(fs->arquivo_disco, fs_offset_bloco(fs, bloco_id) + indice * sizeof(EntradaDiretorio), SEEK_SET);
    fwrite(&entrada, sizeof(EntradaDiretorio), 1, fs->arquivo_disco);
    fs_atualizar_tamanho_diretorio(fs, dir);
    return 1;
}

int fs_ler_entradas_diretorio(SistemaArquivos *fs, const Inode *dir, EntradaDiretorio *resultado, uint32_t max) {
    if (!dir->eh_diretorio) {
        return 0;
    }

    uint32_t entradas_por_bloco = fs->sb.tamanho_bloco / sizeof(EntradaDiretorio);
    EntradaDiretorio entrada;
    uint32_t gravados = 0;

    for (int i = 0; i < BLOCOS_DIRETOS && gravados < max; ++i) {
        uint32_t bloco = dir->blocos_diretos[i];
        if (bloco == 0) {
            continue;
        }
        fseek(fs->arquivo_disco, fs_offset_bloco(fs, bloco), SEEK_SET);
        for (uint32_t j = 0; j < entradas_por_bloco && gravados < max; ++j) {
            fread(&entrada, sizeof(EntradaDiretorio), 1, fs->arquivo_disco);
            if (entrada.id_inode != INODE_INVALID && strcmp(entrada.nome, ".") != 0 && strcmp(entrada.nome, "..") != 0) {
                resultado[gravados++] = entrada;
            }
        }
    }
    return gravados;
}

uint32_t fs_criar_inode(SistemaArquivos *fs, int eh_diretorio) {
    uint32_t inode_id = fs_buscar_inode_livre(fs);
    if (inode_id == INODE_INVALID) {
        return INODE_INVALID;
    }

    fs_bitmap_set(fs->bitmap_inodes, inode_id, 1);
    fs->sb.inodes_livres--;
    fs_salvar_superbloco(fs);
    fs_salvar_bitmaps(fs);

    Inode inode;
    memset(&inode, 0, sizeof(Inode));
    inode.id = inode_id;
    inode.eh_diretorio = eh_diretorio ? 1 : 0;
    inode.tamanho = 0;
    inode.data_criacao = time(NULL);
    inode.data_modificacao = inode.data_criacao;
    inode.data_ultimo_acesso = inode.data_criacao;
    fs_escrever_inode(fs, &inode);
    return inode_id;
}

void fs_liberar_bloco(SistemaArquivos *fs, uint32_t bloco_id) {
    if (bloco_id == 0 || bloco_id >= fs->sb.quantidade_blocos) {
        return;
    }
    fs_bitmap_set(fs->bitmap_blocos, bloco_id, 0);
    fs->sb.blocos_livres++;
    fs_salvar_superbloco(fs);
    fs_salvar_bitmaps(fs);
}

void fs_liberar_inodes_e_blocos(SistemaArquivos *fs, Inode *inode) {
    for (int i = 0; i < BLOCOS_DIRETOS; ++i) {
        if (inode->blocos_diretos[i] != 0) {
            fs_liberar_bloco(fs, inode->blocos_diretos[i]);
            inode->blocos_diretos[i] = 0;
        }
    }

    if (!inode->eh_diretorio && inode->bloco_indireto != 0) {
        uint32_t ponteiros_count = fs->sb.tamanho_bloco / sizeof(uint32_t);
        uint32_t *ponteiros = malloc(ponteiros_count * sizeof(uint32_t));
        if (ponteiros) {
            fs_ler_bloco(fs, inode->bloco_indireto, ponteiros);
            for (uint32_t i = 0; i < ponteiros_count; ++i) {
                if (ponteiros[i] != 0) {
                    fs_liberar_bloco(fs, ponteiros[i]);
                }
            }
            free(ponteiros);
        }
        fs_liberar_bloco(fs, inode->bloco_indireto);
        inode->bloco_indireto = 0;
    }
}

void fs_liberar_inode(SistemaArquivos *fs, uint32_t inode_id) {
    Inode inode;
    fs_ler_inode(fs, inode_id, &inode);
    fs_liberar_inodes_e_blocos(fs, &inode);
    fs_bitmap_set(fs->bitmap_inodes, inode_id, 0);
    fs->sb.inodes_livres++;
    fs_salvar_superbloco(fs);
    fs_salvar_bitmaps(fs);
}

uint32_t fs_resolver_caminho(SistemaArquivos *fs, const char *caminho) {
    if (!caminho || caminho[0] == '\0') {
        return fs->inode_diretorio_atual;
    }

    uint32_t atual = (caminho[0] == '/') ? 0 : fs->inode_diretorio_atual;
    if (strcmp(caminho, "/") == 0) {
        return 0;
    }

    char copia[512];
    strncpy(copia, caminho, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    char *token = strtok(copia, "/");
    while (token != NULL) {
        if (strcmp(token, ".") == 0) {
            token = strtok(NULL, "/");
            continue;
        }

        Inode dir;
        fs_ler_inode(fs, atual, &dir);
        if (!dir.eh_diretorio) {
            return INODE_INVALID;
        }

        if (strcmp(token, "..") == 0) {
            EntradaDiretorio entrada;
            if (!fs_buscar_entrada(fs, &dir, "..", &entrada, NULL, NULL)) {
                return INODE_INVALID;
            }
            atual = entrada.id_inode;
        } else {
            EntradaDiretorio entrada;
            if (!fs_buscar_entrada(fs, &dir, token, &entrada, NULL, NULL)) {
                return INODE_INVALID;
            }
            atual = entrada.id_inode;
        }
        token = strtok(NULL, "/");
    }
    return atual;
}

int fs_resolver_caminho_pai(SistemaArquivos *fs, const char *caminho, uint32_t *pai_id, char *nome_final) {
    if (!caminho || caminho[0] == '\0') {
        return 0;
    }

    char copia[512];
    strncpy(copia, caminho, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    size_t len = strlen(copia);
    while (len > 1 && copia[len - 1] == '/') {
        copia[len - 1] = '\0';
        len--;
    }

    char *ultimo_slash = strrchr(copia, '/');
    if (!ultimo_slash) {
        *pai_id = fs->inode_diretorio_atual;
        strncpy(nome_final, copia, MAX_TAMANHO_NOME);
        nome_final[MAX_TAMANHO_NOME] = '\0';
        return 1;
    }

    if (ultimo_slash == copia) {
        *pai_id = 0;
        strncpy(nome_final, copia + 1, MAX_TAMANHO_NOME);
        nome_final[MAX_TAMANHO_NOME] = '\0';
        return 1;
    }

    *ultimo_slash = '\0';
    *pai_id = fs_resolver_caminho(fs, copia);
    if (*pai_id == INODE_INVALID) {
        return 0;
    }
    strncpy(nome_final, ultimo_slash + 1, MAX_TAMANHO_NOME);
    nome_final[MAX_TAMANHO_NOME] = '\0';
    return 1;
}

int fs_mover_entrada(SistemaArquivos *fs, const char *origem, const char *destino) {
    uint32_t origem_pai;
    char nome_origem[MAX_TAMANHO_NOME + 1];
    if (!fs_resolver_caminho_pai(fs, origem, &origem_pai, nome_origem)) {
        return 0;
    }

    uint32_t destino_pai;
    char nome_destino[MAX_TAMANHO_NOME + 1];
    if (!fs_resolver_caminho_pai(fs, destino, &destino_pai, nome_destino)) {
        return 0;
    }

    Inode pai_origem;
    fs_ler_inode(fs, origem_pai, &pai_origem);
    EntradaDiretorio entrada_origem;
    uint32_t bloco_origem, indice_origem;
    if (!fs_buscar_entrada(fs, &pai_origem, nome_origem, &entrada_origem, &bloco_origem, &indice_origem)) {
        return 0;
    }

    Inode pai_destino;
    fs_ler_inode(fs, destino_pai, &pai_destino);
    EntradaDiretorio existente;
    if (fs_buscar_entrada(fs, &pai_destino, nome_destino, &existente, NULL, NULL)) {
        return 0;
    }

    if (!fs_remover_entrada_diretorio(fs, &pai_origem, bloco_origem, indice_origem)) {
        return 0;
    }
    if (!fs_adicionar_entrada_diretorio(fs, &pai_destino, entrada_origem.id_inode, nome_destino)) {
        fs_adicionar_entrada_diretorio(fs, &pai_origem, entrada_origem.id_inode, nome_origem);
        return 0;
    }

    fs_escrever_inode(fs, &pai_origem);
    fs_escrever_inode(fs, &pai_destino);
    return 1;
}
