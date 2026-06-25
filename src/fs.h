#ifndef FS_H
#define FS_H

#include <time.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_TAMANHO_NOME 255
#define BLOCOS_DIRETOS 12

/* ============================================================================
 * 1. ESTRUTURAS DE DISCO (Persistidas no arquivo do simulador)
 * ========================================================================= */

/* 
 * Estrutura para representar o Superbloco
 * Fica geralmente no primeiro bloco do "disco" e contém o estado global.
 */
typedef struct {
    uint32_t tamanho_particao;     // Tamanho total da partição em bytes
    uint32_t tamanho_bloco;        // Tamanho do bloco escolhido pelo usuário
    uint32_t quantidade_inodes;    // Quantidade total de i-nodes criados
    uint32_t inodes_livres;        // Quantidade de i-nodes sobrando
    uint32_t quantidade_blocos;    // Quantidade total de blocos na partição
    uint32_t blocos_livres;        // Quantidade de blocos sobrando
    
    // Posições no disco (em bytes) onde começam as outras estruturas
    uint32_t inicio_bitmap_inodes; 
    uint32_t inicio_bitmap_blocos;
    uint32_t inicio_tabela_inodes;
    uint32_t inicio_blocos_dados;
} Superbloco;

/* 
 * Estrutura para representar um i-node
 */
typedef struct {
    uint32_t id;                   // Identificador único numérico do i-node
    uint8_t eh_diretorio;          // 1 se for pasta, 0 se for arquivo
    uint32_t tamanho;              // Tamanho lógico em bytes
    
    time_t data_criacao;           // Data e hora de criação
    time_t data_modificacao;       // Data e hora da última modificação
    time_t data_ultimo_acesso;     // Data e hora da última leitura
    
    // Estruturação dos endereços de disco (IDs dos blocos de dados)
    uint32_t blocos_diretos[BLOCOS_DIRETOS]; // Blocos salvos de forma direta
    uint32_t bloco_indireto;                 // Ponteiro de indireção simples
    uint32_t bloco_indireto_duplo;           // Ponteiro de indireção dupla
} Inode;

/* 
 * Estrutura para representar o que tem dentro de uma pasta
 * Os blocos de dados de um diretório são sequências desta estrutura.
 */
typedef struct {
    uint32_t id_inode;                       // Código do i-node vinculado
    char nome[MAX_TAMANHO_NOME + 1];         // Nome do arquivo / subpasta
} EntradaDiretorio;


/* ============================================================================
 * 2. ESTRUTURAS EM MEMÓRIA (Para controle durante a execução do simulador)
 * ========================================================================= */

/* 
 * Mantém o contexto de variáveis globais do disco enquanto o programa roda.
 */
typedef struct {
    FILE *arquivo_disco;           // O arquivo no Linux que finge ser o HD
    Superbloco sb;                 // Os dados globais cacheados em RAM
    
    uint8_t *bitmap_inodes;        // O mapa de i-nodes livres/ocupados na RAM
    uint8_t *bitmap_blocos;        // O mapa de blocos livres/ocupados na RAM
    
    uint32_t inode_diretorio_atual;// Onde o terminal está agora (o "pwd")
} SistemaArquivos;


/* ============================================================================
 * 3. ESTRUTURAS AUXILIARES DE BLOCO
 * ========================================================================= */

#define MAX_TAMANHO_BLOCO 8192 // Um limite generoso para silenciar o compilador

/*
 * Usada para pular pelos blocos indiretos (blocos que guardam IDs de blocos).
 */
typedef struct {
    uint32_t ponteiros[MAX_TAMANHO_BLOCO / 4]; // Array com tamanho limite
} BlocoIndireto;

/*
 * Estrutura padrão para carregar dados reais do disco para a memória.
 */
typedef struct {
    uint8_t dados[MAX_TAMANHO_BLOCO];          // Array com tamanho limite
} BlocoDados;


/* ============================================================================
 * 4. ASSINATURAS DAS FUNÇÕES DO SISTEMA DE ARQUIVOS (API)
 * ========================================================================= */

// Inicialização e Sistema
void fs_formatar_disco(SistemaArquivos *fs, const char *caminho_arquivo_fisico);
void fs_montar_disco(SistemaArquivos *fs, const char *caminho_arquivo_fisico);

// Operações de Diretórios
void fs_criar_diretorio(SistemaArquivos *fs, const char *caminho);
void fs_renomear_diretorio(SistemaArquivos *fs, const char *caminho_antigo, const char *novo_nome);
void fs_apagar_diretorio(SistemaArquivos *fs, const char *caminho);
void fs_listar_conteudo(SistemaArquivos *fs, const char *caminho);

// Operações de Arquivos
void fs_criar_arquivo(SistemaArquivos *fs, const char *caminho);
void fs_renomear_arquivo(SistemaArquivos *fs, const char *caminho_antigo, const char *novo_nome);
void fs_mover_arquivo(SistemaArquivos *fs, const char *caminho_antigo, const char *novo_caminho);
void fs_apagar_arquivo(SistemaArquivos *fs, const char *caminho);

// Importação Externa
void fs_importar_arquivo(SistemaArquivos *fs, const char *caminho_simulado, const char *caminho_real);

#endif // FS_H
