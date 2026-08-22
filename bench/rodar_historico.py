#!/usr/bin/env python3
"""
Compara HistoricoLista vs HistoricoMatriz (tempo e memoria).

Varia numero de compras (C) com baseline fixa; executa bin/tp3.out e
bin/tp3_hist_matriz.out. Saida: bench/resultados_historico.csv
"""

import argparse
import csv
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BIN_LISTA = ROOT / "bin" / "tp3.out"
BIN_MATRIZ = ROOT / "bin" / "tp3_hist_matriz.out"
GERADOR = ROOT / "bench" / "gerar_entrada.py"
CSV_PADRAO = ROOT / "bench" / "resultados_historico.csv"

BASELINE = {
    "usuarios": 1000,
    "produtos": 1000,
    "reposicoes": 200,
    "consultas_lu": 100,
    "consultas_lp": 100,
    "consultas_lc": 100,
    "consultas_lr": 100,
    "filtros": 2,
    "itens_transacao": 3,
    "tipo_consulta": "exata",
    "seed": 42,
}

COMPRAS_VALORES = [50, 100, 200, 400, 800]

CAMPOS = [
    "experimento", "compras", "historico_tipo", "seed",
    "usuarios", "produtos", "reposicoes",
    "tempo_total_s", "total_estimado_bytes", "historico_bytes", "rss_kb",
    "tempo_medio_C_ns", "tempo_medio_LU_ns", "tempo_medio_LP_ns",
    "count_C", "count_LU", "count_LP",
]


def gerar_entrada(compras):
    p = dict(BASELINE)
    p["compras"] = compras
    cmd = [
        sys.executable, str(GERADOR),
        "-u", str(p["usuarios"]),
        "-m", str(p["produtos"]),
        "--compras", str(p["compras"]),
        "--reposicoes", str(p["reposicoes"]),
        "--consultas-lu", str(p["consultas_lu"]),
        "--consultas-lp", str(p["consultas_lp"]),
        "--consultas-lc", str(p["consultas_lc"]),
        "--consultas-lr", str(p["consultas_lr"]),
        "--filtros", str(p["filtros"]),
        "--itens-transacao", str(p["itens_transacao"]),
        "--tipo-consulta", p["tipo_consulta"],
        "--seed", str(p["seed"]),
    ]
    return subprocess.check_output(cmd, text=True)


def parse_saida(output):
    mem = {}
    met = {}
    for linha in output.splitlines():
        if linha.startswith("MEM "):
            partes = linha.split()
            if len(partes) >= 3:
                mem[partes[1]] = partes[2]
        elif linha.startswith("MET op "):
            partes = linha.split()
            if len(partes) >= 9:
                op = partes[2]
                met[op] = int(partes[8])
    return mem, met


def rodar(binario, entrada):
    t0 = time.perf_counter()
    proc = subprocess.run(
        [str(binario)], input=entrada,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True,
    )
    t1 = time.perf_counter()
    mem, met = parse_saida(proc.stdout)
    return t1 - t0, mem, met


def main():
    parser = argparse.ArgumentParser(description="Benchmark HistoricoLista vs HistoricoMatriz")
    parser.add_argument("-o", "--csv", default=str(CSV_PADRAO))
    parser.add_argument("--skip-build", action="store_true")
    args = parser.parse_args()

    if not args.skip_build:
        subprocess.check_call(["make", "all"], cwd=ROOT)
        subprocess.check_call(["make", "hist-matriz-build"], cwd=ROOT)

    for binario in (BIN_LISTA, BIN_MATRIZ):
        if not binario.exists():
            print(f"Erro: {binario} nao encontrado", file=sys.stderr)
            sys.exit(1)

    resultados = []
    for compras in COMPRAS_VALORES:
        entrada = gerar_entrada(compras)
        for historico_tipo, binario in [("lista", BIN_LISTA), ("matriz", BIN_MATRIZ)]:
            tempo, mem, met = rodar(binario, entrada)
            row = {
                "experimento": "H01_compras",
                "compras": compras,
                "historico_tipo": historico_tipo,
                "seed": BASELINE["seed"],
                "usuarios": BASELINE["usuarios"],
                "produtos": BASELINE["produtos"],
                "reposicoes": BASELINE["reposicoes"],
                "tempo_total_s": f"{tempo:.6f}",
                "total_estimado_bytes": mem.get("total_estimado", 0),
                "historico_bytes": mem.get("historico", 0),
                "rss_kb": mem.get("rss_kb", 0),
                "tempo_medio_C_ns": met.get("C", 0),
                "tempo_medio_LU_ns": met.get("LU", 0),
                "tempo_medio_LP_ns": met.get("LP", 0),
                "count_C": compras,
                "count_LU": BASELINE["consultas_lu"],
                "count_LP": BASELINE["consultas_lp"],
            }
            resultados.append(row)
            print(f"  C={compras} [{historico_tipo}] t={tempo:.3f}s "
                  f"hist={mem.get('historico', '?')}B")

    csv_path = Path(args.csv)
    with open(csv_path, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=CAMPOS)
        w.writeheader()
        w.writerows(resultados)
    print(f"\n{len(resultados)} registros em {csv_path}")


if __name__ == "__main__":
    main()
