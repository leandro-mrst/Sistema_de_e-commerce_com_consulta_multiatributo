#!/usr/bin/env python3
"""
Experimentos isolados — analise experimental TP3 MercadoDCC.

Varia UM parametro por vez (demais fixos na baseline), executa MapaVetor e Hash,
coleta tempo e memoria. Saida: bench/resultados_experimentos.csv

Experimentos:
  E01  numero de usuarios
  E02  numero de produtos
  E03  frequencia de C e R
  E04  atributos (filtros) por consulta
  E05  frequencia de consultas LU/LP/LC/LR
  E06  percentual de consultas booleanas
  E07  percentual de faixas preco/qtd em LP
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
CSV_PADRAO = ROOT / "bench" / "resultados_experimentos.csv"

# Baseline fixa (demais parametros constantes em cada experimento)
BASELINE = {
    "usuarios": 1000,
    "produtos": 1000,
    "compras": 200,
    "reposicoes": 200,
    "consultas_lu": 100,
    "consultas_lp": 100,
    "consultas_lc": 100,
    "consultas_lr": 100,
    "filtros": 2,
    "itens_transacao": 3,
    "tipo_consulta": "exata",
    "pct_booleano": 0,
    "pct_faixa_preco_qtd": 0,
    "seed": 42,
}

CAMPOS_CSV = [
    "experimento", "parametro", "valor",
    "variante", "seed",
    "usuarios", "produtos", "compras", "reposicoes",
    "consultas_lu", "consultas_lp", "consultas_lc", "consultas_lr",
    "filtros", "pct_booleano", "pct_faixa_preco_qtd",
    "tempo_total_s",
    "total_estimado_bytes", "rss_kb", "indices_bytes",
    "comparacoes_conjunto", "insercoes_indice",
]


def montar_matriz():
    b = dict(BASELINE)
    q = b["consultas_lu"]

    return [
        ("E01_usuarios", "usuarios", [200, 500, 1000, 2000, 4000], {}),
        ("E02_produtos", "produtos", [200, 500, 1000, 2000, 4000], {}),
        ("E03_transacoes", "compras_reposicoes", [50, 100, 200, 400, 800], {}),
        ("E04_filtros", "filtros", [1, 2, 3, 4, 5], {}),
        ("E05_consultas", "consultas_por_tipo", [50, 100, 200, 300, 400, 500], {}),
        ("E06_booleanos", "pct_booleano", [0, 25, 50, 75, 100], {}),
        ("E07_faixas_preco_qtd", "pct_faixa_preco_qtd", [0, 25, 50, 75, 100], {}),
    ]


def params_para_valor(exp_id, parametro, valor, baseline):
    p = dict(baseline)
    p["seed"] = baseline["seed"]

    if parametro == "usuarios":
        p["usuarios"] = valor
    elif parametro == "produtos":
        p["produtos"] = valor
    elif parametro == "compras_reposicoes":
        p["compras"] = valor
        p["reposicoes"] = valor
    elif parametro == "filtros":
        p["filtros"] = valor
    elif parametro == "consultas_por_tipo":
        p["consultas_lu"] = valor
        p["consultas_lp"] = valor
        p["consultas_lc"] = valor
        p["consultas_lr"] = valor
    elif parametro == "pct_booleano":
        p["pct_booleano"] = valor
        p["tipo_consulta"] = "exata"
    elif parametro == "pct_faixa_preco_qtd":
        p["pct_faixa_preco_qtd"] = valor
        p["tipo_consulta"] = "exata"

    return p


def gerar_entrada(params):
    cmd = [sys.executable, str(GERADOR), "--seed", str(params["seed"])]
    flags = [
        ("-u", "usuarios"),
        ("-m", "produtos"),
        ("--compras", "compras"),
        ("--reposicoes", "reposicoes"),
        ("--filtros", "filtros"),
        ("--itens-transacao", "itens_transacao"),
        ("--tipo-consulta", "tipo_consulta"),
        ("--pct-booleano", "pct_booleano"),
        ("--pct-faixa-preco-qtd", "pct_faixa_preco_qtd"),
        ("--consultas-lu", "consultas_lu"),
        ("--consultas-lp", "consultas_lp"),
        ("--consultas-lc", "consultas_lc"),
        ("--consultas-lr", "consultas_lr"),
    ]
    for flag, key in flags:
        if key in params and params[key] is not None:
            cmd.extend([flag, str(params[key])])
    return subprocess.check_output(cmd, text=True)


def parse_saida(output):
    dados = {}
    for linha in output.splitlines():
        if linha.startswith("MEM "):
            partes = linha.split()
            if len(partes) >= 3:
                dados[partes[1]] = partes[2]
        elif linha.startswith("MET comparacoes_conjunto "):
            dados["comparacoes_conjunto"] = linha.split()[2]
        elif linha.startswith("MET insercoes_indice "):
            dados["insercoes_indice"] = linha.split()[2]
    return dados


def rodar(binario, entrada):
    t0 = time.perf_counter()
    proc = subprocess.run(
        [str(binario)], input=entrada,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True,
    )
    t1 = time.perf_counter()
    return t1 - t0, parse_saida(proc.stdout)


def executar_experimento(exp_id, parametro, valores, baseline):
    linhas = []
    for valor in valores:
        params = params_para_valor(exp_id, parametro, valor, baseline)
        entrada = gerar_entrada(params)
        for variante, binario in [("vetor", BIN_VETOR), ("hash", BIN_HASH)]:
            tempo, parsed = rodar(binario, entrada)
            linhas.append({
                "experimento": exp_id,
                "parametro": parametro,
                "valor": valor,
                "variante": variante,
                "seed": params["seed"],
                "usuarios": params["usuarios"],
                "produtos": params["produtos"],
                "compras": params["compras"],
                "reposicoes": params["reposicoes"],
                "consultas_lu": params["consultas_lu"],
                "consultas_lp": params["consultas_lp"],
                "consultas_lc": params["consultas_lc"],
                "consultas_lr": params["consultas_lr"],
                "filtros": params["filtros"],
                "pct_booleano": params.get("pct_booleano", 0),
                "pct_faixa_preco_qtd": params.get("pct_faixa_preco_qtd", 0),
                "tempo_total_s": f"{tempo:.6f}",
                "total_estimado_bytes": parsed.get("total_estimado", 0),
                "rss_kb": parsed.get("rss_kb", 0),
                "indices_bytes": parsed.get("indices", 0),
                "comparacoes_conjunto": parsed.get("comparacoes_conjunto", 0),
                "insercoes_indice": parsed.get("insercoes_indice", 0),
            })
            print(f"  {exp_id} {parametro}={valor} [{variante}] "
                  f"t={tempo:.3f}s mem={parsed.get('total_estimado', '?')} "
                  f"rss={parsed.get('rss_kb', '?')}KB")
    return linhas


def main():
    parser = argparse.ArgumentParser(description="Experimentos isolados TP3")
    parser.add_argument("-o", "--csv", default=str(CSV_PADRAO))
    parser.add_argument("--skip-build", action="store_true")
    parser.add_argument(
        "--experimento", default="",
        help="Rodar apenas um experimento (ex: E01_usuarios)",
    )
    args = parser.parse_args()

    if not args.skip_build:
        subprocess.check_call(["make", "all"], cwd=ROOT)
        subprocess.check_call(["make", "hash-build"], cwd=ROOT)

    for binario in (BIN_VETOR, BIN_HASH):
        if not binario.exists():
            print(f"Erro: {binario} nao encontrado", file=sys.stderr)
            sys.exit(1)

    matriz = montar_matriz()
    if args.experimento:
        matriz = [m for m in matriz if m[0] == args.experimento]
        if not matriz:
            print(f"Experimento '{args.experimento}' nao encontrado", file=sys.stderr)
            sys.exit(1)

    resultados = []
    print("Baseline fixa:", BASELINE)
    print()

    for exp_id, parametro, valores, _extra in matriz:
        print(f"== {exp_id} (varia {parametro}) ==")
        resultados.extend(executar_experimento(exp_id, parametro, valores, BASELINE))
        print()

    csv_path = Path(args.csv)
    csv_path.parent.mkdir(parents=True, exist_ok=True)
    with open(csv_path, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=CAMPOS_CSV)
        w.writeheader()
        w.writerows(resultados)

    print(f"{len(resultados)} registros gravados em {csv_path}")
    print("Graficos: python3 bench/plot_experimentos.py")


if __name__ == "__main__":
    main()
