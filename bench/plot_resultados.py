#!/usr/bin/env python3
"""Gera graficos a partir de bench/resultados_roteiro.csv para o relatorio PDF."""

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
CSV_PADRAO = ROOT / "bench" / "resultados_roteiro.csv"
CSV_LOCALIDADE = ROOT / "bench" / "resultados_localidade.csv"
SAIDA = ROOT / "bench" / "graficos"


def carregar(csv_path):
    with open(csv_path, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def filtrar(rows, **kwargs):
    out = rows
    for k, v in kwargs.items():
        out = [r for r in out if r.get(k) == str(v)]
    return out


def plot_escala(rows, out_dir):
    fig, axes = plt.subplots(1, 2, figsize=(12, 5))
    for variante, ax, metrica, ylabel in [
        ("vetor", axes[0], "tempo_total_s", "Tempo total (s)"),
        ("hash", axes[1], "total_estimado_bytes", "Memoria estimada (bytes)"),
    ]:
        sub = [r for r in rows if r["variante"] == variante and r["experimento"].startswith("E")]
        if not sub:
            continue
        xs = sorted(set(int(r["usuarios"]) for r in sub))
        ys = []
        for x in xs:
            m = [float(r[metrica]) for r in sub if int(r["usuarios"]) == x]
            ys.append(sum(m) / len(m) if m else 0)
        ax.plot(xs, ys, marker="o", label=variante)
        ax.set_xlabel("n usuarios = m produtos")
        ax.set_ylabel(ylabel)
        ax.set_title(f"{variante.upper()} — escala")
        ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(out_dir / "escala_tempo_memoria.png", dpi=150)
    plt.close(fig)


def plot_filtros(rows, out_dir):
    fig, ax = plt.subplots(figsize=(8, 5))
    for variante in ("vetor", "hash"):
        sub = [r for r in rows if r["variante"] == variante and r["experimento"].startswith("F")]
        if not sub:
            continue
        xs = sorted(set(int(r["filtros"]) for r in sub))
        ys = []
        for x in xs:
            m = [float(r["tempo_medio_LU_ns"]) + float(r["tempo_medio_LP_ns"])
                 for r in sub if int(r["filtros"]) == x]
            ys.append(sum(m) / len(m) if m else 0)
        ax.plot(xs, ys, marker="s", label=variante)
    ax.set_xlabel("Numero de filtros por consulta")
    ax.set_ylabel("Tempo medio LU+LP (ns)")
    ax.set_title("Consulta vs numero de filtros")
    ax.legend()
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(out_dir / "filtros_vs_tempo.png", dpi=150)
    plt.close(fig)


def plot_comparacoes(rows, out_dir):
    fig, ax = plt.subplots(figsize=(8, 5))
    perfis = ["cadastros", "transacoes", "consultas"]
    x = range(len(perfis))
    w = 0.35
    for i, variante in enumerate(("vetor", "hash")):
        vals = []
        for perfil in perfis:
            sub = [int(r["comparacoes_conjunto"]) for r in rows
                   if r["variante"] == variante and perfil in r.get("perfil", "")]
            vals.append(sum(sub) / len(sub) if sub else 0)
        ax.bar([xi + i * w for xi in x], vals, width=w, label=variante)
    ax.set_xticks([xi + w / 2 for xi in x])
    ax.set_xticklabels(perfis)
    ax.set_ylabel("Comparacoes em ConjuntoIds")
    ax.set_title("Comparacoes por perfil de carga")
    ax.legend()
    fig.tight_layout()
    fig.savefig(out_dir / "comparacoes_por_perfil.png", dpi=150)
    plt.close(fig)


def plot_localidade(rows, out_dir):
    """Graficos a partir de bench/resultados_localidade.csv (Valgrind)."""
    if not rows:
        return

    # D1 miss rate por perfil
    fig, axes = plt.subplots(1, 2, figsize=(12, 5))

    perfis = sorted(set(r["perfil"] for r in rows))
    x = range(len(perfis))
    w = 0.35
    for i, (variante, metrica, titulo) in enumerate([
        ("vetor", "d1_miss_rate_pct", "D1 data cache miss rate"),
        ("hash", "lld_miss_rate_pct", "LLd data cache miss rate"),
    ]):
        ax = axes[i]
        for j, var in enumerate(("vetor", "hash")):
            vals = []
            for perfil in perfis:
                sub = [float(r[metrica]) for r in rows
                       if r["variante"] == var and r["perfil"] == perfil]
                vals.append(sum(sub) / len(sub) if sub else 0)
            ax.bar([xi + j * w for xi in x], vals, width=w, label=var)
        ax.set_xticks([xi + w / 2 for xi in x])
        ax.set_xticklabels(perfis, rotation=15)
        ax.set_ylabel("Taxa de miss (%)")
        ax.set_title(titulo)
        ax.legend()
        ax.grid(True, alpha=0.3, axis="y")

    fig.suptitle("Localidade de referencia (Valgrind Cachegrind)", fontsize=12)
    fig.tight_layout()
    fig.savefig(out_dir / "localidade_cache_miss.png", dpi=150)
    plt.close(fig)

    # MapaVetor vs Hash: D1 miss rate comparativo
    fig, ax = plt.subplots(figsize=(9, 5))
    labels = []
    vetor_vals = []
    hash_vals = []
    for r in rows:
        if r["variante"] != "vetor":
            continue
        exp = r["experimento"]
        hash_r = next((x for x in rows if x["experimento"] == exp and x["variante"] == "hash"), None)
        if hash_r:
            labels.append(f"{r['perfil']}\n({r.get('tipo_consulta', 'misto')})")
            vetor_vals.append(float(r["d1_miss_rate_pct"]))
            hash_vals.append(float(hash_r["d1_miss_rate_pct"]))

    if labels:
        xp = range(len(labels))
        ax.bar([xi - 0.2 for xi in xp], vetor_vals, width=0.4, label="MapaVetor")
        ax.bar([xi + 0.2 for xi in xp], hash_vals, width=0.4, label="Hash")
        ax.set_xticks(list(xp))
        ax.set_xticklabels(labels, fontsize=8)
        ax.set_ylabel("D1 miss rate (%)")
        ax.set_title("MapaVetor vs Hash — localidade de dados (menor = melhor)")
        ax.legend()
        ax.grid(True, alpha=0.3, axis="y")
        fig.tight_layout()
        fig.savefig(out_dir / "localidade_vetor_vs_hash.png", dpi=150)
        plt.close(fig)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--csv", default=str(CSV_PADRAO))
    parser.add_argument("-o", "--output", default=str(SAIDA))
    parser.add_argument(
        "--localidade", action="store_true",
        help="Gera apenas graficos de localidade (Valgrind)",
    )
    parser.add_argument(
        "--csv-localidade", default=str(CSV_LOCALIDADE),
        help="CSV gerado por valgrind_localidade.py",
    )
    args = parser.parse_args()

    out_dir = Path(args.output)
    out_dir.mkdir(parents=True, exist_ok=True)

    if args.localidade:
        loc_rows = carregar(args.csv_localidade)
        if not loc_rows:
            print(f"CSV vazio: {args.csv_localidade}. Rode: make localidade-rapido",
                  file=sys.stderr)
            sys.exit(1)
        plot_localidade(loc_rows, out_dir)
        print(f"Graficos de localidade salvos em {out_dir}/")
        return

    rows = carregar(args.csv)
    if not rows:
        print("CSV vazio ou inexistente. Rode: python3 bench/roteiro_bench.py")
        sys.exit(1)

    plot_escala(rows, out_dir)
    plot_filtros(rows, out_dir)
    plot_comparacoes(rows, out_dir)

    loc_rows = carregar(args.csv_localidade)
    if loc_rows:
        plot_localidade(loc_rows, out_dir)

    print(f"Graficos salvos em {out_dir}/")


if __name__ == "__main__":
    main()
