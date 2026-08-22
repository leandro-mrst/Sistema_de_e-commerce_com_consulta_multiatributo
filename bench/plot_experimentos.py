#!/usr/bin/env python3
"""
Gera graficos de tempo e memoria por experimento isolado.

Le bench/resultados_experimentos.csv e produz, para cada experimento E01..E07:
  - {exp}_tempo.png   — tempo total vs parametro (MapaVetor vs Hash)
  - {exp}_memoria.png — memoria estimada vs parametro
"""

import argparse
import csv
import sys
from pathlib import Path

try:
    import matplotlib.pyplot as plt
except ImportError:
    print("Instale matplotlib: pip install matplotlib", file=sys.stderr)
    sys.exit(1)

ROOT = Path(__file__).resolve().parent.parent
CSV_PADRAO = ROOT / "bench" / "resultados_experimentos.csv"
SAIDA = ROOT / "bench" / "graficos" / "experimentos"

TITULOS = {
    "E01_usuarios": ("Numero de usuarios", "Usuarios cadastrados (U)"),
    "E02_produtos": ("Numero de produtos", "Produtos cadastrados (P)"),
    "E03_transacoes": ("Frequencia de C e R", "Compras = Reposicoes"),
    "E04_filtros": ("Atributos por consulta", "Numero de filtros"),
    "E05_consultas": ("Frequencia LU/LP/LC/LR", "Consultas de cada tipo"),
    "E06_booleanos": ("Consultas booleanas", "Percentual booleano (%)"),
    "E07_faixas_preco_qtd": ("Faixas preco/qtd em LP", "Percentual faixa (%)"),
}


def carregar(csv_path):
    with open(csv_path, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def serie(rows, exp_id, variante, campo_y):
    sub = [r for r in rows if r["experimento"] == exp_id and r["variante"] == variante]
    sub.sort(key=lambda r: float(r["valor"]))
    xs = [float(r["valor"]) for r in sub]
    ys = [float(r[campo_y]) for r in sub]
    return xs, ys


def plot_experimento(rows, exp_id, out_dir):
    titulo, xlab = TITULOS.get(exp_id, (exp_id, "valor"))

    fig, ax = plt.subplots(figsize=(8, 5))
    for variante, label, marker in [
        ("vetor", "MapaVetor", "o"),
        ("hash", "Tabela Hash", "s"),
    ]:
        xs, ys = serie(rows, exp_id, variante, "tempo_total_s")
        if xs:
            ax.plot(xs, ys, marker=marker, linewidth=2, label=label)
    ax.set_xlabel(xlab)
    ax.set_ylabel("Tempo total (s)")
    ax.set_title(f"Impacto no tempo — {titulo}")
    ax.legend()
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(out_dir / f"{exp_id}_tempo.png", dpi=150)
    plt.close(fig)

    fig, ax = plt.subplots(figsize=(8, 5))
    for variante, label, marker in [
        ("vetor", "MapaVetor", "o"),
        ("hash", "Tabela Hash", "s"),
    ]:
        xs, ys = serie(rows, exp_id, variante, "total_estimado_bytes")
        if xs:
            ax.plot(xs, [y / 1024 for y in ys], marker=marker, linewidth=2, label=label)
    ax.set_xlabel(xlab)
    ax.set_ylabel("Memoria estimada (KB)")
    ax.set_title(f"Impacto na memoria — {titulo}")
    ax.legend()
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(out_dir / f"{exp_id}_memoria.png", dpi=150)
    plt.close(fig)

    # RSS (memoria real do processo)
    fig, ax = plt.subplots(figsize=(8, 5))
    for variante, label, marker in [
        ("vetor", "MapaVetor", "o"),
        ("hash", "Tabela Hash", "s"),
    ]:
        xs, ys = serie(rows, exp_id, variante, "rss_kb")
        if xs:
            ax.plot(xs, ys, marker=marker, linewidth=2, label=label)
    ax.set_xlabel(xlab)
    ax.set_ylabel("RSS (KB)")
    ax.set_title(f"Memoria residente (RSS) — {titulo}")
    ax.legend()
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(out_dir / f"{exp_id}_rss.png", dpi=150)
    plt.close(fig)


def plot_resumo(rows, out_dir):
    """Painel 2x2 com tempo e memoria dos experimentos principais."""
    exps = sorted(set(r["experimento"] for r in rows))
    n = len(exps)
    cols = 2
    rows_n = (n + 1) // 2

    fig, axes = plt.subplots(rows_n, cols, figsize=(12, 4 * rows_n))
    if rows_n == 1:
        axes = [axes]
    axes_flat = [ax for row in axes for ax in (row if hasattr(row, "__len__") else [row])]

    for i, exp_id in enumerate(exps):
        ax = axes_flat[i]
        for variante, label in [("vetor", "MapaVetor"), ("hash", "Hash")]:
            xs, ys = serie(rows, exp_id, variante, "tempo_total_s")
            if xs:
                ax.plot(xs, ys, marker="o", label=label)
        ax.set_title(TITULOS.get(exp_id, (exp_id,))[0])
        ax.set_xlabel("valor")
        ax.set_ylabel("tempo (s)")
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.3)

    for j in range(i + 1, len(axes_flat)):
        axes_flat[j].set_visible(False)

    fig.suptitle("Resumo — tempo por experimento (MapaVetor vs Hash)", fontsize=13)
    fig.tight_layout()
    fig.savefig(out_dir / "resumo_tempo.png", dpi=150)
    plt.close(fig)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--csv", default=str(CSV_PADRAO))
    parser.add_argument("-o", "--output", default=str(SAIDA))
    args = parser.parse_args()

    rows = carregar(args.csv)
    if not rows:
        print(f"CSV vazio: {args.csv}. Rode: make experimentos", file=sys.stderr)
        sys.exit(1)

    out_dir = Path(args.output)
    out_dir.mkdir(parents=True, exist_ok=True)

    exps = sorted(set(r["experimento"] for r in rows))
    for exp_id in exps:
        plot_experimento(rows, exp_id, out_dir)

    plot_resumo(rows, out_dir)
    print(f"{len(exps)} experimentos -> {len(exps) * 3 + 1} graficos em {out_dir}/")


if __name__ == "__main__":
    main()
