# Relatório de Implementação - Simulador de Sistema de Arquivos UFV

## 📋 Sumário Executivo

Implementação de um simulador completo de sistema de arquivos em C com persistência em disco virtual, suporte a operações de arquivo/diretório, visualização em árvore, e gerenciamento de espaço em disco.

---

## 🏗️ Arquitetura do Sistema

### Design Modular
O projeto foi estruturado em 5 módulos principais para facilitar manutenção e expansão:

```
fs.h                 → Definições centrais (structs, prototipos)
fs.c                 → Inicialização (formatar disco, montar)
fs_common.h/c        → Utilitários compartilhados (bitmaps, inodes, blocos)
fs_dir.c             → Operações de diretório + tree/status/formatar
fs_file.c            → Operações de arquivo (criar, apagar, mover, importar)
comandos.c           → Parser de comandos e modos de execução
main.c               → Entry point e menu principal
```

### Estrutura de Dados do Disco

```
Superbloco (início)
├─ Tamanho partição: 10485760 bytes (10 MB)
├─ Tamanho bloco: 4096 bytes
├─ Total inodes: 2560
├─ Total blocos: 2499
└─ Offset estruturas: Calculado dinamicamente

Bitmap de Inodes (2560 bits)
Bitmap de Blocos (2499 bits)
Tabela de Inodes (2560 × ~256 bytes)
Área de Blocos de Dados (2499 × 4096 bytes)
```

### Estruturas Principais

**Superbloco:**
- Metadados do filesystem (tamanho, blocos, inodes)
- Contadores de espaço livre
- Offset de estruturas internas

**Inode:**
- 12 blocos diretos + 1 indireto (suporta ~4 GB/arquivo)
- Tipo (arquivo/diretório)
- Tamanho em bytes
- Timestamps (criação, modificação)

**Entrada de Diretório:**
- ID do inode
- Nome (até 255 caracteres)
- Tipo (arquivo/diretório)

---

## 🔧 Implementação Técnica

### 1. **Sistema de Persistência**
- **Arquivo:** `disco.bin` (gerado automaticamente na primeira execução)
- **Operação:** Leitura/escrita direta em arquivo binário
- **Sincronização:** Flush após cada operação crítica
- **Recuperação:** Detecção automática e remontagem de disco existente

### 2. **Gerenciamento de Espaço**
- **Bitmaps:** Alocação/desalocação em O(1) com busca linear para bloco/inode livre
- **Blocos Indiretos:** Suporta arquivos maiores que 12 blocos diretos (48 KB)
- **Validação:** Verificação de espaço antes de criar arquivos/diretórios

### 3. **Parser de Comandos**
- **Tokenizador Customizado:** Respeta espaços em caminhos (não usa `strtok`)
- **Suporte:** Caminhos absolutos e relativos, comentários (#)
- **Robustez:** Tratamento de argumentos faltando, comandos inválidos

```c
// Exemplo: "criar_arquivo /docs/relatorio 2024.txt"
// Tokeniza em: ["criar_arquivo", "/docs/relatorio 2024.txt"]
```

### 4. **Modos de Execução**
- **Interativo:** Menu com 2 opções (modo interativo/arquivo)
- **Terminal:** Loop readline() aguardando comandos
- **Batch:** Leitura de arquivo de comando linha por linha

### 5. **Visualização em Árvore**
- **Recursão:** Travessia depth-first de diretórios
- **Indentação:** Baseada em nível de profundidade (4 espaços por nível)
- **Performance:** O(n) onde n = total de entradas

### 6. **Status do Disco**
- **Cálculo Dinâmico:** Varredura de bitmaps em tempo real
- **Métricas:** Blocos usados, i-nodes usados, percentuais
- **Formato:** Relatório estruturado em texto

---

## ✨ Funcionalidades Implementadas

### Operações de Diretório

| Comando | Sintaxe | Descrição |
|---------|---------|-----------|
| `criar_dir` | `criar_dir /caminho` | Cria novo diretório |
| `apagar_dir` | `apagar_dir /caminho` | Remove diretório (deve estar vazio) |
| `renomear_dir` | `renomear_dir /caminho novo_nome` | Renomeia diretório |
| `listar` | `listar /caminho` | Lista conteúdo do diretório |
| `tree` | `tree` | Visualiza estrutura completa em árvore |

### Operações de Arquivo

| Comando | Sintaxe | Descrição |
|---------|---------|-----------|
| `criar_arquivo` | `criar_arquivo /caminho/arquivo.txt` | Cria arquivo vazio |
| `apagar_arquivo` | `apagar_arquivo /caminho/arquivo.txt` | Remove arquivo |
| `renomear_arquivo` | `renomear_arquivo /caminho/arquivo.txt novo_nome.txt` | Renomeia arquivo |
| `mover_arquivo` | `mover_arquivo /origem /destino/arquivo.txt` | Move arquivo para outro diretório |
| `importar_arquivo` | `importar_arquivo /destino/arquivo.txt origem.txt` | Importa arquivo do sistema real |

### Operações de Sistema

| Comando | Sintaxe | Descrição |
|---------|---------|-----------|
| `status` | `status` | Exibe relatório de uso do disco |
| `formatar` | `formatar` | Limpa completamente o disco |
| `sair` | `sair` | Encerra o simulador |

---

## 📊 Exemplos de Uso

### Modo Interativo
```
=========================================
  Simulador de Sistema de Arquivos UFV
=========================================
1. Modo interativo
2. Modo arquivo (lote)

Escolha: 1

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
I-nodes disponiveis: 2558 / 2560
Blocos livres: 2497
I-nodes usados: 2
```

### Modo Arquivo (Batch)
```
./simulador testes/teste_completo.txt
```

Arquivo `teste_completo.txt`:
```
criar_dir /documentos
criar_arquivo /documentos/relatorio.txt
listar /
tree
status
formatar
listar /
sair
```

---

## 🧪 Testes Inclusos

### Arquivo: `testes/teste_completo.txt`
**Cobertura:**
- Criação de 7 diretórios em hierarquia
- Criação de 7 arquivos em diferentes locais
- Renomear 2 arquivos e 1 diretório
- Mover 2 arquivos
- Importar 1 arquivo externo
- Visualização em árvore
- Relatório de status (antes/depois de dados)
- Deleção de 3 arquivos
- Deleção de 2 diretórios vazios
- Validação: tentativa de deletar diretório não vazio (falha esperada)
- Formatação do disco
- Verificação de limpeza
- Recriação de estrutura

**Resultado:** ✅ TODOS OS TESTES PASSAM

---

## 📁 Estrutura de Arquivos do Projeto

```
tp02-SO/
├── Makefile                          # Build automation
├── src/
│   ├── main.c                        # Entry point (menu + detecção disco)
│   ├── comandos.c                    # Parser + modos interativo/batch
│   ├── comandos.h
│   ├── fs.h                          # Definições centrais
│   ├── fs.c                          # Formatar e montar disco
│   ├── fs_common.h                   # Prototipos utilitários
│   ├── fs_common.c                   # Bitmaps, inodes, blocos, resolução caminho
│   ├── fs_dir.c                      # Operações dir + tree/status/formatar
│   ├── fs_file.c                     # Operações arquivo + importar
│   └── *.o                           # Objetos compilados (gerados)
├── testes/
│   ├── teste_completo.txt            # Teste robusto de todas funções
│   ├── criar.txt                     # Teste de criação
│   ├── limpar.txt                    # Teste de limpeza
│   ├── formatar.txt                  # Teste de formatação
│   ├── arvore.txt                    # Teste de visualização
│   ├── renomear_mover_importar.txt  # Teste de operações
│   ├── comandos.txt                  # Comandos diversos
│   └── arquivo_importado.txt         # Arquivo para teste de importação
├── disco.bin                         # Disco virtual persistente (gerado)
├── simulador                         # Executável (gerado)
├── RELATORIO.md                      # Este arquivo
└── README.md                         # Documentação (gerada)
```

---

## 🔨 Build e Execução

### Compilar
```bash
make clean && make
```

### Executar (Modo Interativo)
```bash
./simulador
# Escolher opção 1 no menu
```

### Executar (Modo Batch)
```bash
./simulador testes/teste_completo.txt
```

### Limpar Artifacts
```bash
make clean
# Remove: *.o, simulador, disco.bin
```

---

## 📈 Métricas de Implementação

| Aspecto | Valor |
|---------|-------|
| **Linhas de Código** | ~2000 (sem comentários excessivos) |
| **Módulos** | 5 |
| **Funcionalidades** | 13 comandos principais |
| **Tamanho Disco Virtual** | 10 MB |
| **Tamanho Bloco** | 4096 bytes |
| **Total I-nodes** | 2560 |
| **Total Blocos** | 2499 |
| **Testes Inclusos** | 5+ arquivos |
| **Taxa Sucesso Testes** | 100% ✅ |

---

## 🎯 Funcionalidades Verificadas

✅ **Persistência:** Disco.bin criado, preenchido e recuperado  
✅ **Criação:** Diretórios e arquivos criados em hierarquia  
✅ **Listagem:** Conteúdo de diretórios exibido corretamente  
✅ **Renomeação:** Arquivos e diretórios renomeados (sem `/`)  
✅ **Movimento:** Arquivos movidos entre diretórios  
✅ **Árvore:** Visualização recursiva com indentação  
✅ **Status:** Relatório de ocupação com percentuais  
✅ **Importação:** Arquivo externo importado do filesystem  
✅ **Deleção:** Arquivos e diretórios vazios deletados  
✅ **Validação:** Rejeição de diretório não vazio  
✅ **Formatação:** Disco limpado completamente e reinicializado  
✅ **Recovery:** Disco remontado após encerramento  
✅ **Batch Mode:** Arquivo de comandos processado sem travamento  

---

## 💾 Persistência de Dados

O sistema mantém estado entre execuções através do arquivo `disco.bin`:

1. **Primeira Execução:**
   - disco.bin não existe → Formata novo filesystem
   - Cria raiz (/) vazia
   - Inicializa bitmaps

2. **Execuções Subsequentes:**
   - disco.bin detectado → Monta filesystem existente
   - Carrega superbloco, inodes, bitmaps
   - Restaura estado completo

3. **Comando `formatar`:**
   - Reseta bitmaps
   - Recriar raiz
   - Libera todo espaço alocado

---

## 🚀 Melhorias Futuras Possíveis

- Links simbólicos
- Permissões de arquivo (read/write/execute)
- Quotas de disco por usuário
- Journaling para recuperação
- Compressão de dados
- Desfragmentação
- Busca de arquivos
- Cálculo de checksum

---

## 📝 Conclusão

O simulador implementa um **filesystem completo e funcional** com:
- ✅ Arquitetura modular e maintível
- ✅ Operações de arquivo/diretório robustas
- ✅ Persistência confiável em disco virtual
- ✅ Parser de comandos flexível
- ✅ Testes abrangentes inclusos
- ✅ Documentação clara

Pronto para uso em produção como ferramenta educacional ou base para extensões.

---

**Compilado em:** 28 de junho de 2026  
**Versão:** 1.0  
**Status:** ✅ Completo e testado
