# Roteiro de Benchmark — TP3 MercadoDCC

Roteiro experimental conforme **seção 10** do enunciado (`_ED_2026_1__TP3.pdf`) e `GUIA_IMPLEMENTACAO.md`.

---

## 1. Objetivos

| Requisito (enunciado) | Como atendemos |
|----------------------|----------------|
| 3 perfis de carga | `cadastros`, `transacoes`, `consultas` em `gerar_entrada.py` |
| 2 variantes de estrutura | `make all` (MapaVetor) vs `make hash-build` (Hash) |
| Variar n, m, compras, reposições | Perfis + experimentos de escala |
| Produtos por transação (1, 5, 20) | Experimento `I*_itens_*` |
| Filtros por consulta (1, 2, 4) | Experimento `F*_filtros_*` |
| Faixa vs exata vs booleano | `--tipo-consulta` |
| Tempo médio por operação | Comando `MET` + colunas `tempo_medio_*_ns` |
| Tempo vs nº de filtros | Experimento `F*_filtros_*` |
| Comparações interseção/união/complemento | `MET comparacoes_conjunto` |
| Memória aproximada | Comando `MEM` |
| Localidade de referência | Seção 5 deste documento (texto para o PDF) |

---

## 2. Variantes comparadas

| Variante | Binário | Mapa valor→ids | Conjunto de ids |
|----------|---------|----------------|-----------------|
| **A (MapaVetor)** | `bin/tp3.out` | `MapaVetor` (vetor ordenado) | `Vetor` ordenado (`ConjuntoIds`) |
| **B (Hash)** | `bin/tp3_hash.out` | `MapaHash` | `Vetor` ordenado (`ConjuntoIds`) |

Compilação:

```bash
make all          # variante MapaVetor (padrao)
make hash-build   # variante Hash (sem apagar tp3.out)
```

---

## 3. Perfis de carga (obrigatório)

### 3.1 Perfil **cadastros** (~70% U+P)

**Foco:** custo de inserção nos índices invertidos.

```bash
python3 bench/gerar_entrada.py --perfil cadastros --total-linhas 15000 -o bench/carga_cadastros.in
./bin/tp3.out < bench/carga_cadastros.in > /dev/null
```

### 3.2 Perfil **transacoes** (~50% C+R)

**Foco:** validação de estoque, atualização de `qtd` nos índices, históricos.

```bash
python3 bench/gerar_entrada.py --perfil transacoes --total-linhas 12000 \
  --itens-transacao 5 -o bench/carga_transacoes.in
```

### 3.3 Perfil **consultas** (~70% LU/LP/LC/LR)

**Foco:** interseção, faixas numéricas, operadores booleanos.

```bash
python3 bench/gerar_entrada.py --perfil consultas --total-linhas 15000 \
  --tipo-consulta misto --filtros 2 -o bench/carga_consultas.in
```

---

## 4. Matriz de parâmetros (seção 10.3)

| Parâmetro | Valores testados | Experimento |
|-----------|------------------|-------------|
| n usuários = m produtos | 500, 2000, 5000 | `E*_escala_*` |
| Produtos por C/R | 1, 5, 20 | `I*_itens_*` |
| Filtros por consulta | 1, 2, 4 | `F*_filtros_*` |
| Tipo de consulta | exata, faixa, booleano, misto | `C*_tipo_*` |

---

## 5. Localidade de referência (Valgrind Cachegrind)

A seção **10.5** do enunciado exige documentar localidade de referência. Usamos
**Valgrind Cachegrind** para quantificar misses de cache (proxy de localidade).

### 5.1 O que mede o Cachegrind

| Métrica | Significado |
|---------|-------------|
| **D1 miss rate** | Falhas na cache L1 de dados — acessos dispersos (listas, ponteiros) aumentam |
| **LLd miss rate** | Falhas na cache LL de dados — penalidade maior (RAM) |
| **LL miss rate** | Total instrução + dados na cache LL |
| **Branch mispredict** | Desvios imprevisíveis (ex.: comparações em AVL) |

**Menor taxa de miss → melhor localidade de referência.**

### 5.2 Execução automatizada

```bash
# Rapido (~2–5 min): 3 perfis × AVL + Hash
make localidade-rapido

# Completo: inclui consultas faixa/booleano
make localidade
```

Saídas:
- `bench/resultados_localidade.csv` — dados numéricos
- `bench/relatorio_localidade.md` — texto pronto para o PDF
- `bench/cachegrind/*.out` — arquivos brutos
- `bench/graficos/localidade_*.png` — gráficos

### 5.3 Traço endereço × ciclo (Valgrind Lackey)

Gráfico scatter no estilo da seção 10.5: eixo **X = ciclo** (ordem dos loads),
eixo **Y = endereço** normalizado. Faixas **diagonais** indicam localidade
**espacial** (varredura sequencial); faixas **horizontais** indicam localidade
**temporal** (reutilização do mesmo endereço).

```bash
make localidade-traco    # ~1–2 min por variante
```

Saídas:
- `bench/lackey/traco_vetor.csv`, `traco_hash.csv`
- `bench/graficos/localidade_traco_vetor.png`
- `bench/graficos/localidade_traco_hash.png`
- `bench/graficos/localidade_traco_comparacao.png`

### 5.4 Inspeção manual (cg_annotate)

```bash
# Funcoes com mais D1 read misses
cg_annotate --auto=yes --sort=D1mr bench/cachegrind/CG_consultas_avl.out | less

# Funcoes com mais leituras de dados
cg_annotate --auto=yes --sort=Dr bench/cachegrind/CG_cadastros_hash.out | less
```

### 5.5 Interpretação teórica (para o relatório)

1. **`ConjuntoIds` (vetor ordenado):** ids contíguos → interseção/união percorre
   indices sequenciais (`i`, `i+1`) com boa **localidade espacial** e prefetch.

2. **AVL (`MapaAVL`):** nos com `new` → endereços dispersos → buscas seguem
   ponteiros esq/dir → **mais D1 misses** em `buscar_no` (ver cg_annotate).

3. **Hash (`MapaHash`):** array de buckets contiguo (boa localidade no vetor de
   ponteiros), mas colisoes percorrem **lista encadeada** → nos dispersos.

4. **`Vetor<Entidade>` por id:** acesso O(1) pos-consulta; ids consecutivos no
   resultado implicam acessos adjacentes no array de entidades.

5. **MapaVetor vs Hash:** compare colunas `d1_miss_rate_pct` no CSV — perfil
   **cadastros** tende a favorecer MapaVetor (acessos sequenciais); **consultas**
   podem favorecer Hash (lookup O(1) medio).

### 5.5 Comando Valgrind manual

```bash
python3 bench/gerar_entrada.py --perfil consultas --total-linhas 3000 \
  --sem-metricas --sem-memoria -o bench/carga_cg.in

valgrind --tool=cachegrind --cache-sim=yes --branch-sim=yes \
  --cachegrind-out-file=bench/cachegrind/manual_avl.out \
  ./bin/tp3.out < bench/carga_cg.in
```

O sumário aparece no **stderr** (linhas `I refs`, `D1 misses`, etc.).

---

## 6. Métricas coletadas

### 6.1 Comandos especiais na entrada

| Comando | Saída |
|---------|-------|
| `MET` | Tempo total/médio por operação (ns), comparações, inserções nos índices |
| `MEM` | Memória estimada por componente + RSS (KB) |

Exemplo:

```
MET op U count 3500 tempo_ns_total 1234567 tempo_ns_medio 352
MET comparacoes_conjunto 890123
MET insercoes_indice 42000
MEM variante vetor
MEM total_estimado 743925
MEM rss_kb 4424
```

### 6.2 CSV do roteiro

Arquivo: `bench/resultados_roteiro.csv`

Colunas principais: `experimento`, `perfil`, `variante`, `tempo_medio_*_ns`, `comparacoes_conjunto`, `total_estimado_bytes`, `rss_kb`.

---

## 7. Execução automatizada

### 7.1 Teste rápido (~1 min)

Valida os 3 perfis com carga reduzida:

```bash
make roteiro-rapido
```

Equivalente a:

```bash
make all && make hash-build
python3 bench/roteiro_bench.py --modo rapido
```

### 7.2 Roteiro completo (relatório)

Todos os experimentos da matriz (pode levar vários minutos):

```bash
make roteiro
```

Equivalente a:

```bash
make all && make hash-build
python3 bench/roteiro_bench.py --modo completo --repeticoes 3
```

Subconjuntos:

```bash
python3 bench/roteiro_bench.py --modo perfis      # só 3 perfis
python3 bench/roteiro_bench.py --modo escala      # n = 500, 2000, 5000
python3 bench/roteiro_bench.py --modo filtros     # 1, 2, 4 filtros
python3 bench/roteiro_bench.py --modo consultas   # exata, faixa, booleano
```

### 7.4 Experimentos isolados (tempo e memória)

Varia **um parâmetro por vez** (baseline fixa), compara **MapaVetor** vs **Hash**:

| Exp | Parâmetro variado | Valores testados |
|-----|-------------------|------------------|
| E01 | Usuários (U) | 200, 500, 1000, 2000, 4000 |
| E02 | Produtos (P) | 200, 500, 1000, 2000, 4000 |
| E03 | Compras + Reposições (C/R) | 50, 100, 200, 400, 800 |
| E04 | Atributos (filtros) por consulta | 1, 2, 3, 4, 5 |
| E05 | Consultas LU/LP/LC/LR (cada tipo) | 50, 100, 200, 300, 400, 500 |
| E06 | % consultas booleanas | 0, 25, 50, 75, 100 |
| E07 | % faixas preço/qtd em LP | 0, 25, 50, 75, 100 |

Baseline fixa: 1000 U, 1000 P, 200 C, 200 R, 100 consultas de cada tipo, 2 filtros.

```bash
make experimentos          # todos os experimentos + gráficos
make experimentos-rapido   # só E01 (teste rápido)
```

Saídas:
- `bench/resultados_experimentos.csv`
- `bench/graficos/experimentos/E0*_tempo.png`, `*_memoria.png`, `*_rss.png`
- `bench/graficos/experimentos/resumo_tempo.png`

---

### 7.3 Gráficos para o PDF

```bash
pip install matplotlib   # se necessário
python3 bench/plot_resultados.py
# Gera: bench/graficos/*.png
```

Gráficos sugeridos no relatório:

- Tempo total vs escala (AVL vs Hash)
- Tempo médio de consulta vs número de filtros
- Comparações em `ConjuntoIds` por perfil
- Memória estimada vs escala

---

## 8. Análise esperada no relatório

Para cada gráfico/tabela, discutir:

1. **Cadastros:** Hash vs AVL em `insercoes_indice` e tempo médio de `U`/`P`.
2. **Transações:** impacto de `itens_transacao` (1 vs 5 vs 20) em `C`/`R`.
3. **Consultas:** crescimento do tempo com filtros; diferença faixa vs exata vs booleano.
4. **Memória:** `total_estimado` vs `rss_kb`; overhead de ponteiros (AVL/Hash).
5. **Localidade:** correlacionar comparações com perfil de consultas intensivas.

---

## 9. Regressão funcional (antes do benchmark)

Garantir que o binário de entrega continua correto:

```bash
make all
./bin/tp3.out < tests/exemplo.in | diff - tests/exemplo.out
```

---

## 10. Estrutura de arquivos

```
bench/
├── ROTEIRO.md                  ← protocolo geral
├── valgrind_localidade.py      ← Cachegrind (localidade)
├── relatorio_localidade.md     ← gerado: texto para PDF
├── resultados_localidade.csv   ← gerado: metricas Valgrind
├── cachegrind/                 ← arquivos .out brutos
├── gerar_entrada.py
├── roteiro_bench.py
├── plot_resultados.py
├── resultados_roteiro.csv
└── graficos/
    ├── localidade_cache_miss.png
    └── localidade_vetor_vs_hash.png
```

---

## 11. Checklist para entrega

- [ ] `make all` produz `bin/tp3.out`
- [ ] `make hash-build` produz `bin/tp3_hash.out`
- [ ] 3 perfis executados (cadastros, transacoes, consultas)
- [ ] Ambas variantes comparadas em cada experimento
- [ ] Parâmetros 1/5/20 itens, 1/2/4 filtros, tipos de consulta
- [ ] Gráficos no PDF (≤ 10 páginas total)
- [ ] Seção de localidade de referência preenchida (`make localidade-rapido`)
- [ ] Gráficos `localidade_*.png` no PDF
- [ ] Teste `exemplo.in` passa sem diff
