#!/usr/bin/env python3
"""
Roteiro completo de benchmark — TP3 MercadoDCC (secao 10 do enunciado).

Executa:
  1. Tres perfis de carga (cadastros, transacoes, consultas)
  2. Duas variantes de estrutura (MapaVetor e Hash)
  3. Variacao de parametros: n/m, itens por transacao, filtros, tipo de consulta
  4. Coleta: tempo por operacao, comparacoes, memoria estimada e RSS

Saida: bench/resultados_roteiro.csv
"""

import argparse
import csv
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BIN_VETOR = ROOT / "bin" / "tp3.out"
BIN_HASH = ROOT / "bin" / "tp3_hash.out"
GERADOR = ROOT / "bench" / "gerar_entrada.py"
CSV_PADRAO = ROOT / "bench" / "resultados_roteiro.csv"

# --- Matriz de experimentos (secao 10.1–10.3) ---

PERFIS = [
    ("cadastros", 15000),
    ("transacoes", 12000),
    ("consultas", 15000),
]

ESCALAS = [500, 2000, 5000]

ITENS_TRANSACAO = [1, 5, 20]

FILTROS = [1, 2, 4]

TIPOS_CONSULTA = ["exata", "faixa", "booleano", "misto"]

# Frequencia de consultas LU/LP/LC/LR (cada tipo), baseline fixa secao 10.
CONSULTAS_POR_TIPO = [50, 100, 200, 300, 400, 500]

CONSULTAS_BASELINE = {
    "usuarios": 1000,
    "produtos": 1000,
    "compras": 200,
    "reposicoes": 200,
    "filtros": 2,
    "itens_transacao": 3,
    "tipo_consulta": "exata",
    "seed": 42,
}

CAMPOS_CSV = [
    "experimento", "perfil", "variante", "seed",
    "usuarios", "produtos", "compras", "reposicoes", "consultas",
    "itens_transacao", "filtros", "tipo_consulta", "total_linhas",
    "tempo_total_s",
    "tempo_medio_U_ns", "tempo_medio_P_ns", "tempo_medio_C_ns", "tempo_medio_R_ns",
    "tempo_medio_LU_ns", "tempo_medio_LP_ns", "tempo_medio_LC_ns", "tempo_medio_LR_ns",
    "count_U", "count_P", "count_C", "count_R",
    "count_LU", "count_LP", "count_LC", "count_LR",
    "comparacoes_conjunto", "insercoes_indice",
    "total_estimado_bytes", "rss_kb",
    "indices_bytes", "historico_bytes",
]


def gerar_entrada(**kwargs):
    cmd = [sys.executable, str(GERADOR), "--seed", str(kwargs.get("seed", 42))]
    for flag, key in [
        ("--perfil", "perfil"),
        ("--total-linhas", "total_linhas"),
        ("-u", "usuarios"),
        ("-m", "produtos"),
        ("--compras", "compras"),
        ("--reposicoes", "reposicoes"),
        ("--consultas", "consultas"),
        ("--filtros", "filtros"),
        ("--itens-transacao", "itens_transacao"),
        ("--tipo-consulta", "tipo_consulta"),
        ("--consultas-lu", "consultas_lu"),
        ("--consultas-lp", "consultas_lp"),
        ("--consultas-lc", "consultas_lc"),
        ("--consultas-lr", "consultas_lr"),
    ]:
        if key in kwargs and kwargs[key] is not None and kwargs[key] != "":
            cmd.extend([flag, str(kwargs[key])])
    return subprocess.check_output(cmd, text=True)


def parse_saida(output):
    dados = {"mem": {}, "met": {}}
    for linha in output.splitlines():
        if linha.startswith("MEM "):
            p = linha.split()
            if len(p) >= 3:
                dados["mem"][p[1]] = p[2]
        elif linha.startswith("MET op "):
            p = linha.split()
            # MET op U count N tempo_ns_total T tempo_ns_medio M
            if len(p) >= 8:
                op = p[2]
                dados["met"][op] = {
                    "count": int(p[4]),
                    "tempo_ns_total": int(p[6]),
                    "tempo_ns_medio": int(p[8]),
                }
        elif linha.startswith("MET comparacoes_conjunto "):
            dados["comparacoes_conjunto"] = int(linha.split()[2])
        elif linha.startswith("MET insercoes_indice "):
            dados["insercoes_indice"] = int(linha.split()[2])
    return dados


def rodar_binario(binario, entrada):
    t0 = time.perf_counter()
    proc = subprocess.run(
        [str(binario)], input=entrada, capture_output=True, text=True, check=True,
    )
    t1 = time.perf_counter()
    parsed = parse_saida(proc.stdout)
    return t1 - t0, parsed


def linha_resultado(exp_id, meta, variante, seed, parsed, tempo_total):
    mem = parsed.get("mem", {})
    met = parsed.get("met", {})

    def med(op):
        return met.get(op, {}).get("tempo_ns_medio", 0)

    def cnt(op):
        return met.get(op, {}).get("count", 0)

    return {
        "experimento": exp_id,
        "perfil": meta.get("perfil", ""),
        "variante": variante,
        "seed": seed,
        "usuarios": meta.get("usuarios", ""),
        "produtos": meta.get("produtos", ""),
        "compras": meta.get("compras", ""),
        "reposicoes": meta.get("reposicoes", ""),
        "consultas": meta.get("consultas_lu", meta.get("consultas", "")),
        "itens_transacao": meta.get("itens_transacao", ""),
        "filtros": meta.get("filtros", ""),
        "tipo_consulta": meta.get("tipo_consulta", ""),
        "total_linhas": meta.get("total_linhas", ""),
        "tempo_total_s": f"{tempo_total:.6f}",
        "tempo_medio_U_ns": med("U"),
        "tempo_medio_P_ns": med("P"),
        "tempo_medio_C_ns": med("C"),
        "tempo_medio_R_ns": med("R"),
        "tempo_medio_LU_ns": med("LU"),
        "tempo_medio_LP_ns": med("LP"),
        "tempo_medio_LC_ns": med("LC"),
        "tempo_medio_LR_ns": med("LR"),
        "count_U": cnt("U"),
        "count_P": cnt("P"),
        "count_C": cnt("C"),
        "count_R": cnt("R"),
        "count_LU": cnt("LU"),
        "count_LP": cnt("LP"),
        "count_LC": cnt("LC"),
        "count_LR": cnt("LR"),
        "comparacoes_conjunto": parsed.get("comparacoes_conjunto", 0),
        "insercoes_indice": parsed.get("insercoes_indice", 0),
        "total_estimado_bytes": mem.get("total_estimado", 0),
        "rss_kb": mem.get("rss_kb", 0),
        "indices_bytes": mem.get("indices", 0),
        "historico_bytes": mem.get("historico", 0),
    }


def executar_experimento(exp_id, params, seeds, resultados):
    entrada = gerar_entrada(**params)
    n_linhas = len([l for l in entrada.splitlines() if l.strip()])
    meta = dict(params)
    meta["total_linhas"] = n_linhas

    for seed in seeds:
        if seed != params.get("seed"):
            p = dict(params)
            p["seed"] = seed
            entrada = gerar_entrada(**p)

        for nome, binario in [("vetor", BIN_VETOR), ("hash", BIN_HASH)]:
            tempo, parsed = rodar_binario(binario, entrada)
            row = linha_resultado(exp_id, meta, nome, params.get("seed", 42), parsed, tempo)
            resultados.append(row)
            print(
                f"  [{exp_id}] {nome} seed={params.get('seed')} "
                f"t={tempo:.3f}s comp={row['comparacoes_conjunto']} "
                f"mem={row['total_estimado_bytes']} rss={row['rss_kb']}KB"
            )


def montar_experimentos(modo):
    exps = []
    n = 0

    if modo in ("completo", "perfis"):
        for perfil, total in PERFIS:
            n += 1
            exps.append((f"P{n:02d}_perfil_{perfil}", {
                "perfil": perfil,
                "total_linhas": total,
                "filtros": 2,
                "itens_transacao": 5,
                "tipo_consulta": "misto",
                "seed": 42,
            }))

    if modo in ("completo", "escala"):
        for escala in ESCALAS:
            n += 1
            exps.append((f"E{n:02d}_escala_{escala}", {
                "usuarios": escala,
                "produtos": escala,
                "compras": escala // 2,
                "reposicoes": escala // 4,
                "consultas": escala,
                "filtros": 2,
                "itens_transacao": 5,
                "tipo_consulta": "misto",
                "seed": 42,
            }))

    if modo in ("completo", "itens"):
        for itens in ITENS_TRANSACAO:
            n += 1
            exps.append((f"I{n:02d}_itens_{itens}", {
                "perfil": "transacoes",
                "total_linhas": 8000,
                "itens_transacao": itens,
                "filtros": 2,
                "tipo_consulta": "exata",
                "seed": 42,
            }))

    if modo in ("completo", "filtros"):
        for nf in FILTROS:
            n += 1
            exps.append((f"F{n:02d}_filtros_{nf}", {
                "perfil": "consultas",
                "total_linhas": 10000,
                "filtros": nf,
                "itens_transacao": 3,
                "tipo_consulta": "exata",
                "seed": 42,
            }))

    if modo in ("completo", "consultas_freq"):
        for q in CONSULTAS_POR_TIPO:
            n += 1
            p = dict(CONSULTAS_BASELINE)
            p["consultas_lu"] = q
            p["consultas_lp"] = q
            p["consultas_lc"] = q
            p["consultas_lr"] = q
            exps.append((f"Q{n:02d}_consultas_tipo_{q}", p))

    if modo in ("completo", "consultas"):
        for tipo in TIPOS_CONSULTA:
            n += 1
            exps.append((f"C{n:02d}_tipo_{tipo}", {
                "perfil": "consultas",
                "total_linhas": 10000,
                "filtros": 2,
                "itens_transacao": 3,
                "tipo_consulta": tipo,
                "seed": 42,
            }))

    if modo == "rapido":
        exps = [
            ("RAPIDO_cadastros", {
                "perfil": "cadastros", "total_linhas": 3000,
                "filtros": 2, "itens_transacao": 3, "tipo_consulta": "misto", "seed": 42,
            }),
            ("RAPIDO_transacoes", {
                "perfil": "transacoes", "total_linhas": 3000,
                "filtros": 2, "itens_transacao": 5, "tipo_consulta": "misto", "seed": 42,
            }),
            ("RAPIDO_consultas", {
                "perfil": "consultas", "total_linhas": 3000,
                "filtros": 2, "itens_transacao": 3, "tipo_consulta": "misto", "seed": 42,
            }),
        ]

    return exps


def main():
    parser = argparse.ArgumentParser(description="Roteiro completo de benchmark TP3")
    parser.add_argument(
        "--modo",
        choices=["completo", "rapido", "perfis", "escala", "itens", "filtros",
                 "consultas", "consultas_freq"],
        default="rapido",
        help="Subconjunto de experimentos (completo = todos)",
    )
    parser.add_argument(
        "--repeticoes", type=int, default=1,
        help="Seeds 42..42+repeticoes-1 para media no relatorio",
    )
    parser.add_argument("-o", "--csv", type=str, default=str(CSV_PADRAO))
    parser.add_argument("--skip-build", action="store_true")
    args = parser.parse_args()

    if not args.skip_build:
        subprocess.check_call(["make", "all"], cwd=ROOT)
        subprocess.check_call(["make", "hash-build"], cwd=ROOT)

    experimentos = montar_experimentos(args.modo)
    seeds = [42 + i for i in range(args.repeticoes)]

    print(f"Roteiro: {len(experimentos)} experimentos x 2 variantes x {len(seeds)} seed(s)")
    print(f"Modo: {args.modo}\n")

    resultados = []
    for exp_id, params in experimentos:
        print(f"== {exp_id} ==")
        for i, seed in enumerate(seeds):
            p = dict(params)
            p["seed"] = seed
            executar_experimento(exp_id, p, [seed], resultados)

    csv_path = Path(args.csv)
    csv_path.parent.mkdir(parents=True, exist_ok=True)
    escrever_header = not csv_path.exists() or csv_path.stat().st_size == 0
    with open(csv_path, "a", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=CAMPOS_CSV)
        if escrever_header:
            w.writeheader()
        w.writerows(resultados)

    print(f"\n{len(resultados)} registros gravados em {csv_path}")
    print("Use bench/plot_resultados.py para gerar graficos do relatorio.")


if __name__ == "__main__":
    main()
