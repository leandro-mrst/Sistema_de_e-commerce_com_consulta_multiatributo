#!/usr/bin/env python3
"""
Analise de localidade de referencia com Valgrind Cachegrind.

Simula caches I1/D1/LL e mede taxas de miss de dados — proxy quantitativo
de localidade espacial/temporal conforme secao 10.5 do enunciado.

Requisito: valgrind instalado (sudo apt install valgrind)

Saida:
  bench/resultados_localidade.csv
  bench/relatorio_localidade.md
  bench/cachegrind/*.out  (arquivos brutos para cg_annotate)
"""

import argparse
import csv
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BIN_VETOR = ROOT / "bin" / "tp3.out"
BIN_HASH = ROOT / "bin" / "tp3_hash.out"
GERADOR = ROOT / "bench" / "gerar_entrada.py"
CSV_PADRAO = ROOT / "bench" / "resultados_localidade.csv"
RELATORIO = ROOT / "bench" / "relatorio_localidade.md"
CG_DIR = ROOT / "bench" / "cachegrind"

# Cargas por perfil (total-linhas). Modo completo usa cargas maiores.
CARGAS_RAPIDO = [
    ("cadastros", 2000),
    ("transacoes", 2000),
    ("consultas", 2000),
]

CARGAS_COMPLETO = [
    ("cadastros", 8000, "misto"),
    ("transacoes", 8000, "misto"),
    ("consultas", 8000, "misto"),
    ("consultas", 6000, "faixa"),
    ("consultas", 6000, "booleano"),
]

CAMPOS_CSV = [
    "experimento", "perfil", "variante", "total_linhas", "tipo_consulta",
    "i_refs", "i1_misses", "i1_miss_rate_pct",
    "d_refs", "d_reads", "d_writes",
    "d1_misses", "d1_miss_rate_pct",
    "lld_misses", "lld_miss_rate_pct",
    "ll_misses", "ll_miss_rate_pct",
    "branches", "branch_mispredicts", "branch_mispredict_rate_pct",
    "cachegrind_out",
]

# Funcoes-chave para localidade (relatorio)
FUNCOES_LOCALIDADE = [
    "ConjuntoIds::",
    "MapaVetor",
    "MapaHash",
    "IndiceOrdenado",
    "Vetor<",
    "intersect",
    "uniao",
    "complemento",
    "faixa_rec",
    "buscar_no",
]


def verificar_valgrind():
    try:
        subprocess.run(
            ["valgrind", "--tool=cachegrind", "--help"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=True,
        )
    except (FileNotFoundError, subprocess.CalledProcessError):
        print("Erro: valgrind nao encontrado. Instale: sudo apt install valgrind",
              file=sys.stderr)
        sys.exit(1)


def gerar_entrada(perfil, total_linhas, tipo_consulta="misto", seed=42):
    cmd = [
        sys.executable, str(GERADOR),
        "--perfil", perfil,
        "--total-linhas", str(total_linhas),
        "--tipo-consulta", tipo_consulta,
        "--seed", str(seed),
        "--sem-metricas",
        "--sem-memoria",
    ]
    return subprocess.check_output(cmd, text=True)


def parse_cachegrind_stderr(stderr):
    """Extrai sumario do stderr do Cachegrind."""
    nums = lambda s: int(s.replace(",", ""))

    def buscar(padrao, texto, default=0):
        m = re.search(padrao, texto)
        return m.group(1) if m else default

    def buscar_num(padrao, texto, default=0):
        v = buscar(padrao, texto, None)
        return nums(v) if v else default

    def buscar_float(padrao, texto, default=0.0):
        v = buscar(padrao, texto, None)
        return float(v) if v else default

    d_refs_line = re.search(
        r"D refs:\s+([\d,]+)\s+\(([\d,]+) rd\s+\+\s+([\d,]+) wr\)", stderr
    )

    return {
        "i_refs": buscar_num(r"I refs:\s+([\d,]+)", stderr),
        "i1_misses": buscar_num(r"I1  misses:\s+([\d,]+)", stderr),
        "i1_miss_rate_pct": buscar_float(r"I1  miss rate:\s+([\d.]+)%", stderr),
        "d_refs": nums(d_refs_line.group(1)) if d_refs_line else 0,
        "d_reads": nums(d_refs_line.group(2)) if d_refs_line else 0,
        "d_writes": nums(d_refs_line.group(3)) if d_refs_line else 0,
        "d1_misses": buscar_num(r"D1  misses:\s+([\d,]+)", stderr),
        "d1_miss_rate_pct": buscar_float(r"D1  miss rate:\s+([\d.]+)%", stderr),
        "lld_misses": buscar_num(r"LLd misses:\s+([\d,]+)", stderr),
        "lld_miss_rate_pct": buscar_float(r"LLd miss rate:\s+([\d.]+)%", stderr),
        "ll_misses": buscar_num(r"LL misses:\s+([\d,]+)", stderr),
        "ll_miss_rate_pct": buscar_float(r"LL miss rate:\s+([\d.]+)%", stderr),
        "branches": buscar_num(r"Branches:\s+([\d,]+)", stderr),
        "branch_mispredicts": buscar_num(r"Mispredicts:\s+([\d,]+)", stderr),
        "branch_mispredict_rate_pct": buscar_float(r"Mispred rate:\s+([\d.]+)%", stderr),
    }


def top_funcoes_cachegrind(cg_out, limite=15):
    """Usa cg_annotate para listar funcoes com mais D1 read misses."""
    try:
        proc = subprocess.run(
            ["cg_annotate", "--auto=yes", "--threshold=0.1", "--sort=D1mr", str(cg_out)],
            capture_output=True, text=True, check=False,
        )
    except FileNotFoundError:
        return []

    funcoes = []
    for linha in proc.stdout.splitlines():
        if "PROGRAM TOTALS" in linha or "file:function" in linha:
            continue
        if "ConjuntoIds" in linha or "MapaVetor" in linha or "MapaHash" in linha:
            funcoes.append(linha.strip())
        elif "Vetor" in linha or "IndiceOrdenado" in linha or "faixa_rec" in linha:
            funcoes.append(linha.strip())
        if len(funcoes) >= limite:
            break
    return funcoes


def rodar_cachegrind(binario, entrada, cg_out):
    CG_DIR.mkdir(parents=True, exist_ok=True)
    with cg_out.open("w") as f:
        f.write("")

    proc = subprocess.run(
        [
            "valgrind", "--tool=cachegrind",
            "--cache-sim=yes",
            "--branch-sim=yes",
            f"--cachegrind-out-file={cg_out}",
            str(binario),
        ],
        input=entrada,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
        text=True,
        check=True,
    )
    return parse_cachegrind_stderr(proc.stderr)


def montar_experimentos(modo):
    if modo == "rapido":
        return [
            (f"CG_{p}", p, n, "misto")
            for p, n in CARGAS_RAPIDO
        ]
    return [
        (f"CG_{p}" if t == "misto" else f"CG_{p}_{t}", p, n, t)
        for p, n, t in CARGAS_COMPLETO
    ]


def executar(modo, seed):
    verificar_valgrind()
    experimentos = montar_experimentos(modo)
    resultados = []

    for exp_id, perfil, total_linhas, tipo_consulta in experimentos:
        entrada = gerar_entrada(perfil, total_linhas, tipo_consulta, seed)

        for variante, binario in [("vetor", BIN_VETOR), ("hash", BIN_HASH)]:
            if not binario.exists():
                print(f"Erro: {binario} nao encontrado. Rode make all / make hash-build.",
                      file=sys.stderr)
                sys.exit(1)

            cg_out = CG_DIR / f"{exp_id}_{variante}.out"
            print(f"== {exp_id} [{variante}] cachegrind (linhas~{total_linhas}) ==")
            stats = rodar_cachegrind(binario, entrada, cg_out)

            row = {
                "experimento": exp_id,
                "perfil": perfil,
                "variante": variante,
                "total_linhas": total_linhas,
                "tipo_consulta": tipo_consulta,
                "cachegrind_out": str(cg_out.relative_to(ROOT)),
                **stats,
            }
            resultados.append(row)
            print(f"  D1 miss rate: {stats['d1_miss_rate_pct']:.2f}%  "
                  f"LLd miss rate: {stats['lld_miss_rate_pct']:.2f}%  "
                  f"LL miss rate: {stats['ll_miss_rate_pct']:.2f}%")

    return resultados


def salvar_csv(resultados, csv_path):
    csv_path.parent.mkdir(parents=True, exist_ok=True)
    escrever_header = not csv_path.exists() or csv_path.stat().st_size == 0
    with open(csv_path, "a", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=CAMPOS_CSV)
        if escrever_header:
            w.writeheader()
        w.writerows(resultados)


def gerar_relatorio(resultados, relatorio_path):
    """Gera relatorio markdown para copiar ao PDF do TP."""
    linhas = [
        "# Analise de Localidade de Referencia (Valgrind Cachegrind)",
        "",
        "Medicao via simulacao de caches I1/D1/LL. Quanto **menor** a taxa de miss",
        "de dados (D1/LLd), **melhor** a localidade de referencia.",
        "",
        "## 1. Resumo por perfil e variante",
        "",
        "| Perfil | Variante | D1 miss rate | LLd miss rate | LL miss rate | D refs |",
        "|--------|----------|--------------|---------------|--------------|--------|",
    ]

    for r in resultados:
        linhas.append(
            f"| {r['perfil']} | {r['variante']} | "
            f"{r['d1_miss_rate_pct']:.2f}% | {r['lld_miss_rate_pct']:.2f}% | "
            f"{r['ll_miss_rate_pct']:.2f}% | {r['d_refs']:,} |"
        )

    linhas.extend([
        "",
        "## 2. Interpretacao (secao 10.5 do enunciado)",
        "",
        "### 2.1 ConjuntoIds — vetor ordenado de ids",
        "",
        "Os ids ficam em array contiguo (`Vetor<int>`). Operacoes de intersecao,",
        "uniao e complemento percorrem dois vetores com two-pointer, acessando",
        "indices consecutivos (`i`, `i+1`). Isso favorece **localidade espacial**:",
        "acessos sequenciais caem na mesma linha de cache.",
        "",
        "No Cachegrind, funcoes como `ConjuntoIds::intersect` e `operator[]` tendem",
        "a apresentar **baixa taxa de D1 miss** por acesso, pois cada elemento",
        "ocupa 4 bytes contiguos.",
        "",
        "### 2.2 Indices invertidos — MapaVetor vs Hash",
        "",
        "**MapaVetor:** pares (chave, ConjuntoIds) em array contiguo; busca binaria",
        "acessa regiao compacta da memoria → **boa localidade espacial**.",
        "Insercao desloca elementos (O(n)), mas sem ponteiros dispersos.",
        "",
        "**Hash:** bucket array contiguo (boa localidade no vetor de ponteiros),",
        "mas colisoes percorrem **lista encadeada** — cada no em endereco distinto",
        "→ misses adicionais. Lookups O(1) medio com menos comparacoes que busca",
        "binaria quando a tabela e grande.",
        "",
        "Compare `MapaVetor*::buscar` vs `MapaHash*::*` no cg_annotate:",
        "",
        "```bash",
        "cg_annotate --auto=yes --sort=D1mr bench/cachegrind/CG_consultas_vetor.out",
        "cg_annotate --auto=yes --sort=D1mr bench/cachegrind/CG_consultas_hash.out",
        "```",
        "",
        "### 2.3 Vetor de entidades — recuperacao pos-consulta",
        "",
        "Apos obter ids do resultado, `_usuarios[id]`, `_produtos[id]` acessa",
        "array contiguo de structs. Se ids consecutivos (comum quando cadastro e",
        "sequencial e filtro e amplo), acessos sao **adjacentes** → prefetch eficaz.",
        "Consultas muito seletivas (poucos ids dispersos) geram saltos maiores.",
        "",
        "### 2.4 Correlacao perfil × localidade",
        "",
    ])

    # Comparacao MapaVetor vs Hash por perfil
    perfis = sorted(set(r["perfil"] for r in resultados))
    for perfil in perfis:
        vetor = next((r for r in resultados if r["perfil"] == perfil and r["variante"] == "vetor"), None)
        hash_r = next((r for r in resultados if r["perfil"] == perfil and r["variante"] == "hash"), None)
        if vetor and hash_r:
            diff = hash_r["d1_miss_rate_pct"] - vetor["d1_miss_rate_pct"]
            vencedor = "Hash" if hash_r["d1_miss_rate_pct"] < vetor["d1_miss_rate_pct"] else "MapaVetor"
            linhas.append(
                f"- **{perfil}:** MapaVetor D1={vetor['d1_miss_rate_pct']:.2f}% vs "
                f"Hash D1={hash_r['d1_miss_rate_pct']:.2f}% "
                f"(delta {diff:+.2f} p.p.). Melhor localidade de dados: **{vencedor}**."
            )

    linhas.extend([
        "",
        "## 3. Funcoes relevantes (cg_annotate)",
        "",
    ])

    for r in resultados:
        cg_path = ROOT / r["cachegrind_out"]
        if not cg_path.exists():
            continue
        funcoes = top_funcoes_cachegrind(cg_path)
        if not funcoes:
            continue
        linhas.append(f"### {r['experimento']} ({r['variante']})")
        linhas.append("")
        linhas.append("```")
        linhas.extend(funcoes[:8])
        linhas.append("```")
        linhas.append("")

    linhas.extend([
        "## 4. Como reproduzir",
        "",
        "```bash",
        "make localidade-rapido   # ~2-5 min",
        "make localidade          # cargas maiores + faixa/booleano",
        "python3 bench/plot_resultados.py --localidade",
        "```",
        "",
        "Inspecao manual detalhada:",
        "",
        "```bash",
        "cg_annotate --auto=yes --sort=D1mr bench/cachegrind/CG_consultas_vetor.out | less",
        "cg_annotate --auto=yes --sort=Dr bench/cachegrind/CG_cadastros_hash.out | less",
        "```",
    ])

    relatorio_path.write_text("\n".join(linhas) + "\n", encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(
        description="Analise de localidade com Valgrind Cachegrind"
    )
    parser.add_argument(
        "--modo", choices=["rapido", "completo"], default="rapido",
    )
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("-o", "--csv", default=str(CSV_PADRAO))
    parser.add_argument("--relatorio", default=str(RELATORIO))
    parser.add_argument("--skip-build", action="store_true")
    args = parser.parse_args()

    if not args.skip_build:
        subprocess.check_call(["make", "all"], cwd=ROOT)
        subprocess.check_call(["make", "hash-build"], cwd=ROOT)

    resultados = executar(args.modo, args.seed)
    salvar_csv(resultados, Path(args.csv))
    gerar_relatorio(resultados, Path(args.relatorio))

    print(f"\n{len(resultados)} medicoes gravadas em {args.csv}")
    print(f"Relatorio: {args.relatorio}")
    print("Graficos: python3 bench/plot_resultados.py --localidade")


if __name__ == "__main__":
    main()
