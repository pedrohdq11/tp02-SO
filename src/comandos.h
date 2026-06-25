#ifndef COMANDOS_H
#define COMANDOS_H

// Processa a string de um comando lido e o executa
void processar_comando(char *comando);

// Inicia o terminal iterativo aguardando o usuário digitar os comandos
void modo_interativo(void);

// Lê um arquivo de lote (.txt) linha por linha e envia para execução
void modo_arquivo(const char *nome_arquivo);

#endif // COMANDOS_H
