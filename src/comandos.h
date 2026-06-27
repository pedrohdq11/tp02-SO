#ifndef COMANDOS_H
#define COMANDOS_H

#include "fs.h"

void processar_comando(SistemaArquivos *fs, char *comando);
void modo_interativo(SistemaArquivos *fs);
void modo_arquivo(SistemaArquivos *fs, const char *nome_arquivo);
void modo_arquivo(SistemaArquivos *fs, const char *nome_arquivo);

#endif
