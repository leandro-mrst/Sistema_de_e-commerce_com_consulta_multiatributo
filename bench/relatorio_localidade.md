# Analise de Localidade de Referencia (Valgrind Cachegrind)

Medicao via simulacao de caches I1/D1/LL. Quanto **menor** a taxa de miss
de dados (D1/LLd), **melhor** a localidade de referencia.

## 1. Resumo por perfil e variante

| Perfil | Variante | D1 miss rate | LLd miss rate | LL miss rate | D refs |
|--------|----------|--------------|---------------|--------------|--------|
| cadastros | vetor | 0.20% | 0.00% | 0.00% | 204,551,527 |
| cadastros | hash | 0.10% | 0.00% | 0.00% | 168,637,519 |
| transacoes | vetor | 0.10% | 0.00% | 0.00% | 591,632,917 |
| transacoes | hash | 0.10% | 0.00% | 0.00% | 588,951,380 |
| consultas | vetor | 0.10% | 0.00% | 0.00% | 398,063,773 |
| consultas | hash | 0.10% | 0.00% | 0.00% | 391,993,091 |

## 2. Interpretacao (secao 10.5 do enunciado)

### 2.1 ConjuntoIds — vetor ordenado de ids

Os ids ficam em array contiguo (`Vetor<int>`). Operacoes de intersecao,
uniao e complemento percorrem dois vetores com two-pointer, acessando
indices consecutivos (`i`, `i+1`). Isso favorece **localidade espacial**:
acessos sequenciais caem na mesma linha de cache.

No Cachegrind, funcoes como `ConjuntoIds::intersect` e `operator[]` tendem
a apresentar **baixa taxa de D1 miss** por acesso, pois cada elemento
ocupa 4 bytes contiguos.

### 2.2 Indices invertidos — MapaVetor vs Hash

**MapaVetor:** pares (chave, ConjuntoIds) em array contiguo; busca binaria
acessa regiao compacta da memoria → **boa localidade espacial**.
Insercao desloca elementos (O(n)), mas sem ponteiros dispersos.

**Hash:** bucket array contiguo (boa localidade no vetor de ponteiros),
mas colisoes percorrem **lista encadeada** — cada no em endereco distinto
→ misses adicionais. Lookups O(1) medio com menos comparacoes que busca
binaria quando a tabela e grande.

Compare `MapaVetor*::buscar` vs `MapaHash*::*` no cg_annotate:

```bash
cg_annotate --auto=yes --sort=D1mr bench/cachegrind/CG_consultas_vetor.out
cg_annotate --auto=yes --sort=D1mr bench/cachegrind/CG_consultas_hash.out
```

### 2.3 Vetor de entidades — recuperacao pos-consulta

Apos obter ids do resultado, `_usuarios[id]`, `_produtos[id]` acessa
array contiguo de structs. Se ids consecutivos (comum quando cadastro e
sequencial e filtro e amplo), acessos sao **adjacentes** → prefetch eficaz.
Consultas muito seletivas (poucos ids dispersos) geram saltos maiores.

### 2.4 Correlacao perfil × localidade

- **cadastros:** MapaVetor D1=0.20% vs Hash D1=0.10% (delta -0.10 p.p.). Melhor localidade de dados: **Hash**.
- **consultas:** MapaVetor D1=0.10% vs Hash D1=0.10% (delta +0.00 p.p.). Melhor localidade de dados: **MapaVetor**.
- **transacoes:** MapaVetor D1=0.10% vs Hash D1=0.10% (delta +0.00 p.p.). Melhor localidade de dados: **MapaVetor**.

## 3. Funcoes relevantes (cg_annotate)

### CG_cadastros (vetor)

```
13,634,363  (3.5%)            14  (0.0%)            4  (0.1%)          6,637,201  (4.8%)         29,592 (10.2%)            0                 2,695,071  (4.1%)           141  (0.3%)            0                   732,986  (2.1%)         26,364  (3.7%)                0                      0                    Vetor<int>::operator=(Vetor<int> const&)
78,396,730 (20.2%)           887  (1.8%)            5  (0.1%)         34,159,462 (24.6%)         12,908  (4.4%)            0                 6,691,643 (10.2%)           342  (0.8%)            0                12,782,682 (37.4%)         57,072  (8.1%)                0                      0                    ConjuntoIds::inserir(int)
1,268,662  (0.3%)            27  (0.1%)            5  (0.1%)            511,101  (0.4%)         11,784  (4.0%)            0                   232,500  (0.4%)             0                    0                    96,193  (0.3%)         13,045  (1.9%)                0                      0                    IndiceOrdenado<int>::faixa_rec(IndiceOrdenado<int>::No*, int const&, int const&, ConjuntoIds&) const
1,882,500  (0.5%)             0                    0                    753,000  (0.5%)         11,342  (3.9%)            0                   470,625  (0.7%)             0                    0                         0                      0                        0                      0                    EntradaVetorInt::operator=(EntradaVetorInt const&)
1,237,668  (0.3%)             1  (0.0%)            1  (0.0%)            508,408  (0.4%)          8,465  (2.9%)            0                   312,540  (0.5%)            13  (0.0%)            0                   104,180  (0.3%)          3,730  (0.5%)                0                      0                    IndiceOrdenado<int>::altura(IndiceOrdenado<int>::No*) const
1,001,908  (0.3%)            28  (0.1%)            3  (0.1%)            367,742  (0.3%)          8,387  (2.9%)            0                   127,353  (0.2%)             0                    0                    73,655  (0.2%)         13,694  (1.9%)                0                      0                    MapaVetorInt::buscar_pos(int) const
2,144,464  (0.6%)             1  (0.0%)            1  (0.0%)            765,880  (0.6%)          8,380  (2.9%)            0                   612,704  (0.9%)             0                    0                         0                      0                        0                      0                    ConjuntoIds::operator[](int) const
885,621  (0.2%)             5  (0.0%)            5  (0.1%)            391,147  (0.3%)          7,631  (2.6%)            0                   214,783  (0.3%)           599  (1.3%)            0                    51,653  (0.2%)         11,043  (1.6%)                0                      0                    IndiceOrdenado<int>::inserir_rec(IndiceOrdenado<int>::No*, int const&, int)
```

### CG_cadastros (hash)

```
1,268,662  (0.4%)             5  (0.0%)            5  (0.1%)           511,101  (0.4%)         13,255  (7.4%)            0                   232,500  (0.4%)             0                    0                    96,193  (0.3%)         13,771  (2.2%)                0                      0                   IndiceOrdenado<int>::faixa_rec(IndiceOrdenado<int>::No*, int const&, int const&, ConjuntoIds&) const
2,236,150  (0.7%)             2  (0.0%)            1  (0.0%)           798,625  (0.7%)         10,884  (6.1%)            0                   638,900  (1.2%)             0                    0                         0                      0                        0                      0                   ConjuntoIds::operator[](int) const
78,753,704 (24.6%)            66  (0.2%)            5  (0.1%)        34,278,800 (29.6%)          7,306  (4.1%)            0                 6,776,988 (12.8%)            17  (0.0%)            0                12,822,600 (44.6%)         53,211  (8.7%)                0                      0                   ConjuntoIds::inserir(int)
179,264  (0.1%)             1  (0.0%)            1  (0.0%)            89,632  (0.1%)          5,750  (3.2%)            0                    44,816  (0.1%)             0                    0                         0                      0                        0                      0                   Vetor<ParIdQtd>::tamanho() const
1,237,668  (0.4%)             1  (0.0%)            1  (0.0%)           508,408  (0.4%)          4,699  (2.6%)            0                   312,540  (0.6%)             0                    0                   104,180  (0.4%)          3,677  (0.6%)                0                      0                   IndiceOrdenado<int>::altura(IndiceOrdenado<int>::No*) const
536,052  (0.2%)             6  (0.0%)            6  (0.2%)           249,842  (0.2%)          4,418  (2.5%)            0                    66,285  (0.1%)         1,757  (4.2%)          815  (4.6%)            52,483  (0.2%)          1,845  (0.3%)                0                      0                   MapaHashInt::redimensionar()
885,621  (0.3%)             5  (0.0%)            4  (0.1%)           391,147  (0.3%)          4,260  (2.4%)            0                   214,783  (0.4%)            45  (0.1%)            0                    51,653  (0.2%)         10,955  (1.8%)                0                      0                   IndiceOrdenado<int>::inserir_rec(IndiceOrdenado<int>::No*, int const&, int)
187,112  (0.1%)             3  (0.0%)            2  (0.1%)            95,868  (0.1%)          3,959  (2.2%)            0                    21,912  (0.0%)             0                    0                    24,184  (0.1%)            908  (0.1%)                0                      0                   MapaHashInt::destruir()
```

### CG_transacoes (vetor)

```
7,566,491  (0.7%)            94  (0.2%)            5  (0.1%)          3,190,233  (0.8%)        106,091 (28.9%)            0                  1,435,150  (0.8%)            38  (0.1%)            0                   608,798  (0.5%)           82,683  (4.1%)                 0                       0                    IndiceOrdenado<int>::faixa_rec(IndiceOrdenado<int>::No*, int const&, int const&, ConjuntoIds&) const
5,027,652  (0.4%)             1  (0.0%)            1  (0.0%)          1,795,590  (0.4%)         42,825 (11.7%)            0                  1,436,472  (0.8%)             0                    0                         0                        0                         0                       0                    ConjuntoIds::operator[](int) const
1,243,390  (0.1%)            94  (0.2%)            3  (0.1%)            455,474  (0.1%)          8,850  (2.4%)            0                    158,438  (0.1%)             0                    0                    91,036  (0.1%)           18,584  (0.9%)                 0                       0                    MapaVetorInt::buscar_pos(int) const
2,351,660  (0.2%)             1  (0.0%)            1  (0.0%)            964,582  (0.2%)          8,161  (2.2%)            0                    594,462  (0.3%)             3  (0.0%)            0                   198,154  (0.2%)            6,042  (0.3%)                 0                       0                    IndiceOrdenado<int>::altura(IndiceOrdenado<int>::No*) const
777,468  (0.1%)             2  (0.0%)            2  (0.1%)            354,406  (0.1%)          6,439  (1.8%)            0                    164,583  (0.1%)             5  (0.0%)            0                    54,706  (0.0%)           12,775  (0.6%)                 0                       0                    IndiceOrdenado<int>::remover_rec(IndiceOrdenado<int>::No*, int const&, int)
5,048,493  (0.4%)           131  (0.2%)            4  (0.1%)          2,498,391  (0.6%)          5,872  (1.6%)            0                    529,899  (0.3%)         8,122 (14.2%)          761  (8.2%)           230,758  (0.2%)            4,557  (0.2%)                 0                       0                    Vetor<int>::Vetor(Vetor<int> const&)
1,379,691  (0.1%)             5  (0.0%)            5  (0.1%)            610,435  (0.1%)          4,031  (1.1%)            0                    333,331  (0.2%)            66  (0.1%)            0                    80,484  (0.1%)           14,124  (0.7%)                 0                       0                    IndiceOrdenado<int>::inserir_rec(IndiceOrdenado<int>::No*, int const&, int)
906,320  (0.1%)             1  (0.0%)            1  (0.0%)            453,160  (0.1%)          3,875  (1.1%)            0                    226,580  (0.1%)             0                    0                         0                        0                         0                       0                    Vetor<ParIdQtd>::tamanho() const
```

### CG_transacoes (hash)

```
7,566,491  (0.7%)             5  (0.0%)            5  (0.2%)          3,190,233  (0.8%)        112,153 (30.7%)            0                  1,435,150  (0.8%)            35  (0.1%)            0                   608,798  (0.5%)           84,662  (4.3%)                 0                      0                    IndiceOrdenado<int>::faixa_rec(IndiceOrdenado<int>::No*, int const&, int const&, ConjuntoIds&) const
5,070,912  (0.4%)             2  (0.0%)            1  (0.0%)          1,811,040  (0.4%)         45,029 (12.3%)            0                  1,448,832  (0.8%)             0                    0                         0                        0                         0                      0                    ConjuntoIds::operator[](int) const
2,351,660  (0.2%)             1  (0.0%)            1  (0.0%)            964,582  (0.2%)          7,323  (2.0%)            0                    594,462  (0.3%)             0                    0                   198,154  (0.2%)            6,043  (0.3%)                 0                      0                    IndiceOrdenado<int>::altura(IndiceOrdenado<int>::No*) const
777,468  (0.1%)             2  (0.0%)            2  (0.1%)            354,406  (0.1%)          5,812  (1.6%)            0                    164,583  (0.1%)             4  (0.0%)            0                    54,706  (0.0%)           12,117  (0.6%)                 0                      0                    IndiceOrdenado<int>::remover_rec(IndiceOrdenado<int>::No*, int const&, int)
906,320  (0.1%)             1  (0.0%)            1  (0.0%)            453,160  (0.1%)          3,947  (1.1%)            0                    226,580  (0.1%)             0                    0                         0                        0                         0                      0                    Vetor<ParIdQtd>::tamanho() const
4,804,639  (0.4%)            11  (0.0%)            4  (0.1%)          2,388,836  (0.6%)          3,916  (1.1%)            0                    475,385  (0.3%)         6,653 (11.4%)           92  (0.9%)           219,053  (0.2%)            4,224  (0.2%)                 0                      0                    Vetor<int>::Vetor(Vetor<int> const&)
530,499  (0.0%)             7  (0.0%)            7  (0.2%)            220,373  (0.1%)          3,879  (1.1%)            0                    119,196  (0.1%)             7  (0.0%)            0                    19,343  (0.0%)              558  (0.0%)                 0                      0                    MapaHashInt::inserir(int, int)
1,379,691  (0.1%)             5  (0.0%)            4  (0.1%)            610,435  (0.1%)          3,829  (1.0%)            0                    333,331  (0.2%)            59  (0.1%)            0                    80,484  (0.1%)           13,806  (0.7%)                 0                      0                    IndiceOrdenado<int>::inserir_rec(IndiceOrdenado<int>::No*, int const&, int)
```

### CG_consultas (vetor)

```
6,236,880  (0.8%)           133  (0.2%)            5  (0.1%)          2,627,185  (1.0%)         79,726 (23.5%)            0                 1,179,703  (0.9%)             0                    0                   501,439  (0.7%)           70,538  (3.7%)                 0                      0                    IndiceOrdenado<int>::faixa_rec(IndiceOrdenado<int>::No*, int const&, int const&, ConjuntoIds&) const
4,931,836  (0.6%)             0                    0                  1,761,370  (0.7%)         37,142 (11.0%)            0                 1,409,096  (1.1%)             0                    0                         0                        0                         0                      0                    ConjuntoIds::operator[](int) const
845,260  (0.1%)           147  (0.2%)            3  (0.1%)            309,950  (0.1%)          6,957  (2.1%)            0                   107,834  (0.1%)             0                    0                    62,152  (0.1%)           13,325  (0.7%)                 0                      0                    MapaVetorInt::buscar_pos(int) const
876,664  (0.1%)             1  (0.0%)            1  (0.0%)            438,332  (0.2%)          6,832  (2.0%)            0                   219,166  (0.2%)             0                    0                         0                        0                         0                      0                    Vetor<ParIdQtd>::tamanho() const
5,561,429  (0.7%)           211  (0.3%)            4  (0.1%)          2,752,257  (1.0%)          6,136  (1.8%)            0                   586,607  (0.5%)         9,085 (15.8%)          760  (9.9%)           254,188  (0.3%)            6,465  (0.3%)                 0                      0                    Vetor<int>::Vetor(Vetor<int> const&)
3,003,755  (0.4%)            82  (0.1%)            4  (0.1%)          1,471,653  (0.5%)          4,766  (1.4%)            0                   505,332  (0.4%)           215  (0.4%)            1  (0.0%)           155,080  (0.2%)           11,516  (0.6%)                 0                      0                    Vetor<int>::operator=(Vetor<int> const&)
394,798  (0.1%)             5  (0.0%)            5  (0.1%)            169,290  (0.1%)          4,522  (1.3%)            0                    74,306  (0.1%)             0                    0                    32,411  (0.0%)            5,189  (0.3%)                 0                      0                    IndiceOrdenado<double>::faixa_rec(IndiceOrdenado<double>::No*, double const&, double const&, ConjuntoIds&) const
1,317,556  (0.2%)             1  (0.0%)            1  (0.0%)            539,912  (0.2%)          4,345  (1.3%)            0                   333,276  (0.3%)             0                    0                   111,092  (0.1%)            4,193  (0.2%)                 0                      0                    IndiceOrdenado<int>::altura(IndiceOrdenado<int>::No*) const
```

### CG_consultas (hash)

```
6,236,880  (0.8%)             5  (0.0%)            5  (0.2%)          2,627,185  (1.0%)         79,634 (24.8%)            0                 1,179,703  (0.9%)             0                    0                   501,439  (0.7%)           73,252  (3.9%)                 0                      0                    IndiceOrdenado<int>::faixa_rec(IndiceOrdenado<int>::No*, int const&, int const&, ConjuntoIds&) const
4,974,368  (0.6%)             2  (0.0%)            1  (0.0%)          1,776,560  (0.7%)         34,501 (10.8%)            0                 1,421,248  (1.1%)             0                    0                         0                        0                         0                      0                    ConjuntoIds::operator[](int) const
876,664  (0.1%)             1  (0.0%)            1  (0.0%)            438,332  (0.2%)          6,823  (2.1%)            0                   219,166  (0.2%)             0                    0                         0                        0                         0                      0                    Vetor<ParIdQtd>::tamanho() const
394,798  (0.1%)           201  (0.3%)            4  (0.1%)            169,290  (0.1%)          4,866  (1.5%)            0                    74,306  (0.1%)             0                    0                    32,411  (0.0%)            5,483  (0.3%)                 0                      0                    IndiceOrdenado<double>::faixa_rec(IndiceOrdenado<double>::No*, double const&, double const&, ConjuntoIds&) const
5,337,701  (0.7%)             4  (0.0%)            4  (0.1%)          2,651,118  (1.0%)          4,381  (1.4%)            0                   537,701  (0.4%)         7,844 (13.7%)          138  (1.6%)           243,483  (0.3%)            5,614  (0.3%)                 0                      0                    Vetor<int>::Vetor(Vetor<int> const&)
1,317,556  (0.2%)             1  (0.0%)            1  (0.0%)            539,912  (0.2%)          3,380  (1.1%)            0                   333,276  (0.3%)             0                    0                   111,092  (0.1%)            4,214  (0.2%)                 0                      0                    IndiceOrdenado<int>::altura(IndiceOrdenado<int>::No*) const
424,648  (0.1%)             2  (0.0%)            2  (0.1%)            193,490  (0.1%)          2,728  (0.9%)            0                    89,846  (0.1%)             0                    0                    29,914  (0.0%)            6,571  (0.4%)                 0                      0                    IndiceOrdenado<int>::remover_rec(IndiceOrdenado<int>::No*, int const&, int)
97,012  (0.0%)             3  (0.0%)            2  (0.1%)             49,390  (0.0%)          2,119  (0.7%)            0                    11,712  (0.0%)             0                    0                    12,556  (0.0%)               99  (0.0%)                 0                      0                    MapaHashInt::destruir()
```

## 4. Como reproduzir

```bash
make localidade-rapido   # ~2-5 min
make localidade          # cargas maiores + faixa/booleano
python3 bench/plot_resultados.py --localidade
```

Inspecao manual detalhada:

```bash
cg_annotate --auto=yes --sort=D1mr bench/cachegrind/CG_consultas_vetor.out | less
cg_annotate --auto=yes --sort=Dr bench/cachegrind/CG_cadastros_hash.out | less
```
