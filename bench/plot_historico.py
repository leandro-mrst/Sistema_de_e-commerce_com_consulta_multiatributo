#!/usr/bin/env python3
"""Graficos HistoricoLista vs HistoricoMatriz a partir de resultados_historico.csv."""

import argparse
import csv
import sys
from pathlib import Path

try:
    import matplotlib.pyplot as plt
except ImportError:
    print("pip install matplotlib", file=sys.stderr)
    sys.exit(1)

ROOT = Path(__file__).resolve().parent.parent
CSV = ROOT / "bench" / "resultados_historico.csv"
SAIDA = ROOT / "bench" / "graficos"


def carregar(path):
    with open(path, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def serie(rows, tipo, campo):
    sub = [r for r in rows if r["historico_tipo"] == tipo]
    sub.sort(key=lambda r: int(r["compras"]))
    xs = [int(r["compras"]) for r in sub]
    ys = [float(r[campo]) for r in sub]
    return xs, ys


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--csv", default=str(CSV))
    parser.add_argument("-o", "--output", default=str(SAIDA))
    args = parser.parse_args()

    rows = carregar(args.csv)
    if not rows:
        print(f"CSV vazio: {args.csv}. Rode: make historico-bench", file=sys.stderr)
        sys.exit(1)

    out = Path(args.output)
    out.mkdir(parents=True, exist_ok=True)

    fig, axes = plt.subplots(1, 2, figsize=(12, 5))
    for tipo, label, marker in [("lista", "HistoricoLista", "o"), ("matriz", "HistoricoMatriz", "s")]:
        xs, ys = serie(rows, tipo, "tempo_total_s")
        axes[0].plot(xs, ys, marker=marker, linewidth=2, label=label)
        xs, ys = serie(rows, tipo, "historico_bytes")
        axes[1].plot(xs, [y / 1024 for y in ys], marker=marker, linewidth=2, label=label)

    axes[0].set_xlabel("Numero de compras (C)")
    axes[0].set_ylabel("Tempo total (s)")
    axes[0].set_title("Tempo — lista vs matriz")
    axes[0].legend()
    axes[0].grid(True, alpha=0.3)

    axes[1].set_xlabel("Numero de compras (C)")
    axes[1].set_ylabel("Memoria do historico (KB)")
    axes[1].set_title("Memoria — lista vs matriz")
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)

    fig.suptitle("Historico auxiliar de compras (U=P=1000)", fontsize=12)
    fig.tight_layout()
    fig.savefig(out / "historico_lista_vs_matriz.png", dpi=150)
    plt.close(fig)
    print(f"Grafico: {out / 'historico_lista_vs_matriz.png'}")


if __name__ == "__main__":
    main()
