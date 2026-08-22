#!/usr/bin/env python3
"""Executa benchmark comparando variantes MapaVetor e Hash."""

import argparse
import csv
import re
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BIN_VETOR = ROOT / "bin" / "tp3.out"
BIN_HASH = ROOT / "bin" / "tp3_hash.out"
GERADOR = ROOT / "bench" / "gerar_entrada.py"


def gerar_entrada(args):
    cmd = [
        sys.executable,
        str(GERADOR),
        "-u", str(args.usuarios),
        "-m", str(args.produtos),
        "--compras", str(args.compras),
        "--reposicoes", str(args.reposicoes),
        "--consultas", str(args.consultas),
        "--filtros", str(args.filtros),
        "--itens-transacao", str(args.itens_transacao),
        "--seed", str(args.seed),
    ]
    return subprocess.check_output(cmd, text=True)


def parse_mem(output):
    dados = {}
    for linha in output.splitlines():
        if linha.startswith("MEM "):
            partes = linha.split()
            if len(partes) >= 3:
                dados[partes[1]] = partes[2]
    return dados


def rodar(binario, entrada):
    inicio = time.perf_counter()
    proc = subprocess.run(
        [str(binario)],
        input=entrada,
        capture_output=True,
        text=True,
        check=True,
    )
    fim = time.perf_counter()
    mem = parse_mem(proc.stdout)
    return {
        "tempo_s": fim - inicio,
        "variante": mem.get("variante", "?"),
        "total_estimado": int(mem.get("total_estimado", 0)),
        "rss_kb": int(mem.get("rss_kb", 0)),
        "usuarios_bytes": int(mem.get("usuarios", 0)),
        "produtos_bytes": int(mem.get("produtos", 0)),
        "indices_bytes": int(mem.get("indices", 0)),
        "historico_bytes": int(mem.get("historico", 0)),
    }


def main():
    parser = argparse.ArgumentParser(description="Roda benchmark MapaVetor vs Hash")
    parser.add_argument("-u", "--usuarios", type=int, default=2000)
    parser.add_argument("-m", "--produtos", type=int, default=2000)
    parser.add_argument("--compras", type=int, default=1000)
    parser.add_argument("--reposicoes", type=int, default=400)
    parser.add_argument("--consultas", type=int, default=500)
    parser.add_argument("--filtros", type=int, default=2)
    parser.add_argument("--itens-transacao", type=int, default=3)
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("-o", "--csv", type=str, default="bench/resultados.csv")
    parser.add_argument("--skip-build", action="store_true")
    args = parser.parse_args()

    if not args.skip_build:
        subprocess.check_call(["make", "all"], cwd=ROOT)
        subprocess.check_call(["make", "hash-build"], cwd=ROOT)

    entrada = gerar_entrada(args)
    linhas = [l for l in entrada.splitlines() if l.strip()]
    print(f"Carga: {len(linhas)} linhas "
          f"(U={args.usuarios}, P={args.produtos}, C={args.compras}, "
          f"R={args.reposicoes}, consultas={args.consultas})")

    resultados = []
    for nome, binario in [("vetor", BIN_VETOR), ("hash", BIN_HASH)]:
        if not binario.exists():
            print(f"Erro: {binario} nao encontrado", file=sys.stderr)
            sys.exit(1)
        r = rodar(binario, entrada)
        r["config"] = (
            f"u{args.usuarios}_m{args.produtos}_c{args.compras}_"
            f"r{args.reposicoes}_q{args.consultas}"
        )
        r["usuarios_n"] = args.usuarios
        r["produtos_n"] = args.produtos
        resultados.append(r)
        print(f"\n[{nome}] tempo={r['tempo_s']:.3f}s  "
              f"estimado={r['total_estimado']} bytes  "
              f"rss={r['rss_kb']} KB  indices={r['indices_bytes']} bytes")

    csv_path = ROOT / args.csv
    csv_path.parent.mkdir(parents=True, exist_ok=True)
    campos = [
        "config", "variante", "usuarios_n", "produtos_n",
        "tempo_s", "total_estimado", "rss_kb",
        "usuarios_bytes", "produtos_bytes", "indices_bytes", "historico_bytes",
    ]
    escrever_header = not csv_path.exists() or csv_path.stat().st_size == 0
    with open(csv_path, "a", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=campos)
        if escrever_header:
            w.writeheader()
        for r in resultados:
            w.writerow({k: r.get(k, "") for k in campos})

    print(f"\nResultados append em {csv_path}")


if __name__ == "__main__":
    main()
