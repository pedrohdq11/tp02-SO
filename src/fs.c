#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

// ----------------------------------------------------------------------------
// Inicialização e Sistema
// ----------------------------------------------------------------------------

void fs_formatar_disco(SistemaArquivos *fs, const char *caminho_arquivo_fisico) {
    // TODO: Implementar a criacao do arquivo binario principal.
    // Inicializar o Superbloco, preencher bitmaps com zero e gravar no disco.
}

void fs_montar_disco(SistemaArquivos *fs, const char *caminho_arquivo_fisico) {
    // TODO: Implementar a abertura do arquivo existente (.bin).
    // Ler o Superbloco do disco e carregar os mapas de bits para a memoria RAM.
}

// ----------------------------------------------------------------------------
// Operações de Diretórios
// ----------------------------------------------------------------------------

void fs_criar_diretorio(SistemaArquivos *fs, const char *caminho) {
    // TODO: Implementar a busca de um i-node livre e setar eh_diretorio = 1.
    // Alocar um bloco de dados vazio para iniciar o array de EntradaDiretorio.
}

void fs_renomear_diretorio(SistemaArquivos *fs, const char *caminho_antigo, const char *novo_nome) {
    // TODO: Caminhar ate o diretorio pai, encontrar a EntradaDiretorio com o caminho_antigo
    // e alterar o texto dela para o novo_nome.
}

void fs_apagar_diretorio(SistemaArquivos *fs, const char *caminho) {
    // TODO: Verificar se esta vazio. Remover as entradas do diretorio pai.
    // Liberar todos os blocos de dados associados no block_bitmap e libertar o i-node.
}

void fs_listar_conteudo(SistemaArquivos *fs, const char *caminho) {
    // TODO: Acessar os blocos de dados do i-node alvo. 
    // Fazer um loop lendo e imprimindo a estrutura EntradaDiretorio ate acabar os dados.
}

// ----------------------------------------------------------------------------
// Operações de Arquivos
// ----------------------------------------------------------------------------

void fs_criar_arquivo(SistemaArquivos *fs, const char *caminho) {
    // TODO: Alocar um i-node livre (eh_diretorio = 0).
    // Adicionar uma nova EntradaDiretorio no diretorio pai mapeando para este i-node.
}

void fs_renomear_arquivo(SistemaArquivos *fs, const char *caminho_antigo, const char *novo_nome) {
    // TODO: Igual ao de diretorio. Buscar EntradaDiretorio no pai e sobrescrever o nome.
}

void fs_mover_arquivo(SistemaArquivos *fs, const char *caminho_antigo, const char *novo_caminho) {
    // TODO: Copiar a EntradaDiretorio antiga para o bloco de dados do novo diretorio pai.
    // Apagar a EntradaDiretorio antiga do diretorio de origem. O i-node fica intocado!
}

void fs_apagar_arquivo(SistemaArquivos *fs, const char *caminho) {
    // TODO: Buscar os ponteiros no i-node (blocos_diretos, bloco_indireto, etc).
    // Limpar os bits correspondentes no block_bitmap e no inode_bitmap.
    // Remover a entrada do diretorio pai.
}

// ----------------------------------------------------------------------------
// Importação Externa
// ----------------------------------------------------------------------------

void fs_importar_arquivo(SistemaArquivos *fs, const char *caminho_simulado, const char *caminho_real) {
    // TODO: Ler sequencialmente os blocos fisicos do PC local (caminho_real).
    // Alocar blocos iterativamente usando 'fs_encontrar_bloco_livre'
    // Gravar os chunks de tamanho MAX_TAMANHO_BLOCO e assinar nos indices diretos ou indiretos do i-node.
}
