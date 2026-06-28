# Simulador de Sistema de Arquivos UFV

> Um simulador completo de filesystem em C com persistência em disco virtual

## 🚀 Quick Start

### Compilar e Executar
```bash
make clean && make
./simulador
```

### Modos de Execução
- **Opção 1:** Modo interativo (digite comandos no terminal)
- **Opção 2:** Modo arquivo/lote (processa arquivo de comandos)

## 📋 Comandos Disponíveis

### Diretórios
```bash
criar_dir /pasta              # Criar diretório
apagar_dir /pasta             # Remover (deve estar vazio)
renomear_dir /pasta novo_nome # Renomear diretório
listar /pasta                 # Listar conteúdo
```

### Arquivos
```bash
criar_arquivo /caminho/arquivo.txt        # Criar arquivo vazio
apagar_arquivo /caminho/arquivo.txt       # Remover arquivo
renomear_arquivo /arquivo.txt novo.txt    # Renomear
mover_arquivo /origem /destino/arquivo.txt # Mover arquivo
importar_arquivo /destino/arquivo origem  # Importar do SO real
```

### Sistema
```bash
tree                 # Visualizar estrutura em árvore
status               # Relatório de uso do disco
formatar             # Limpar todo o disco
sair                 # Encerrar simulador
```

## 📂 Testes

Executar suite completa de testes:
```bash
./simulador testes/teste_completo.txt
```

Testes inclusos:
- `teste_completo.txt` - Teste robusto de todas funcionalidades ✅
- `criar.txt` - Teste de criação
- `limpar.txt` - Teste de limpeza
- `formatar.txt` - Teste de formatação

## 🏗️ Arquitetura

```
main.c           → Menu principal
comandos.c       → Parser de comandos
fs.c             → Inicialização do disco
fs_common.c      → Utilitários compartilhados
fs_dir.c         → Operações de diretório
fs_file.c        → Operações de arquivo
```

## 💾 Armazenamento

- **Disco Virtual:** `disco.bin` (10 MB)
- **Tamanho Bloco:** 4096 bytes
- **Total I-nodes:** 2560
- **Persistência:** Automática após cada operação

## ✨ Funcionalidades

- ✅ Criação/remoção de diretórios e arquivos
- ✅ Renomeação e movimento de arquivos
- ✅ Importação de arquivos externos
- ✅ Visualização em árvore recursiva
- ✅ Relatório de status do disco
- ✅ Formatação completa do disco
- ✅ Persistência em disco virtual
- ✅ Parser robusto (suporta espaços em caminhos)
- ✅ Modo interativo e batch

## 📊 Exemplo de Uso

```bash
> criar_dir /documentos
Diretorio criado: /documentos

> criar_arquivo /documentos/relatorio.txt
Arquivo criado: /documentos/relatorio.txt

> listar /documentos
Conteudo do diretorio /documentos:
  relatorio.txt

> tree
documentos/
    relatorio.txt

> status
=== Relatorio de Status do Sistema ===
Capacidade Total do Disco: 10485760 bytes
Blocos em uso: 2 / 2499 (0% ocupado)
```

## 📖 Documentação Completa

Veja [RELATORIO.md](RELATORIO.md) para documentação técnica detalhada.

---

**Status:** ✅ Completo | **Versão:** 1.0 | **Data:** 28/06/2026
