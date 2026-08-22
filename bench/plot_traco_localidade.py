#!/usr/bin/env python3
"""
Grafico endereco x ciclo (localidade espacial e temporal).

Gera visualizacoes com zoom em segmentos sequenciais e carga demo isolada,
onde faixas diagonais (espacial) e horizontais (temporal) ficam visiveis.
"""

import argparse
import csv
import sys
from pathlib import Path

try:
    import matplotlib.pyplot as plt
    import numpy as np
except ImportError:
    print("Instale dependencias: pip install matplotlib numpy", file=sys.stderr)
    sys.exit(1)

ROOT = Path(__file__).resolve().parent.parent
LACKEY_DIR = ROOT / "bench" / "lackey"
SAIDA = ROOT / "bench" / "graficos"

# Enderecos de stack Valgrind (Linux x86_64) — excluidos do grafico principal.
LIMITE_STACK = 0x08000000


def carregar_traco(csv_path):
    ciclos, addrs = [], []
    with open(csv_path, newline="", encoding="utf-8") as f:
        for row in csv.DictReader(f):
            ciclos.append(int(row["ciclo"]))
            addrs.append(int(row["endereco"]))
    return np.array(ciclos, dtype=np.int64), np.array(addrs, dtype=np.int64)


def filtrar_heap(ciclos, addrs):
    mask = addrs < LIMITE_STACK
    return ciclos[mask], addrs[mask]


def endereco_relativo_local(addrs):
    """Endereco relativo ao minimo *dentro da janela* (eixo Y compacto)."""
    base = addrs.min()
    return (addrs - base) // 8


def melhor_janela_espacial(ciclos, addrs, janela=600):
    """Retorna indice da janela com maior correlacao ciclo-endereco (diagonal)."""
    n = len(ciclos)
    if n < janela:
        return 0, n

    melhor_corr, melhor_i = -1.0, 0
    rel_full = endereco_relativo_local(addrs) if len(addrs) else addrs

    for i in range(0, n - janela, max(1, janela // 20)):
        seg_c = ciclos[i : i + janela]
        seg_a = addrs[i : i + janela]
        seg_y = (seg_a - seg_a.min()) // 8
        if np.std(seg_c) == 0 or np.std(seg_y) == 0:
            continue
        corr = float(np.corrcoef(seg_c, seg_y)[0, 1])
        if corr > melhor_corr:
            melhor_corr, melhor_i = corr, i

    return melhor_i, janela


def melhor_janela_temporal(ciclos, addrs, janela=400):
    """Janela com endereco quase constante (faixa horizontal)."""
    n = len(ciclos)
    if n < janela:
        return 0, n

    melhor_score, melhor_i = -1.0, 0
    for i in range(0, n - janela, max(1, janela // 15)):
        seg_y = (addrs[i : i + janela] - addrs[i : i + janela].min()) // 8
        amp = int(seg_y.max() - seg_y.min())
        if amp == 0:
            score = 1.0
        else:
            score = 1.0 - float(np.std(seg_y)) / max(amp, 1)
        if score > melhor_score:
            melhor_score, melhor_i = score, i
    return melhor_i, janela


def extrair_runs_sequenciais(ciclos, addrs, passo_max=64, min_len=80):
    """Segmentos contiguos em ciclo com endereco crescente em passos pequenos."""
    runs = []
    i = 0
    n = len(addrs)
    while i < n - 1:
        start = i
        while i + 1 < n and (ciclos[i + 1] - ciclos[i]) == 1:
            delta = addrs[i + 1] - addrs[i]
            if not (0 < delta <= passo_max):
                break
            i += 1
        if i - start + 1 >= min_len:
            runs.append((start, i + 1))
        i += 1
    runs.sort(key=lambda r: r[1] - r[0], reverse=True)
    return runs


def plot_janela(ciclos, addrs, titulo, out_path, tipo="espacial"):
    """Um painel com zoom na melhor janela de localidade."""
    c, a = filtrar_heap(ciclos, addrs)
    if len(c) < 50:
        c, a = ciclos, addrs

    if tipo == "espacial":
        i, w = melhor_janela_espacial(c, a, janela=min(800, len(c) // 2))
    else:
        i, w = melhor_janela_temporal(c, a, janela=min(500, len(c) // 2))

    w = min(w, len(c) - i)
    xc = c[i : i + w]
    yn = (a[i : i + w] - a[i : i + w].min()) // 8

    fig, ax = plt.subplots(figsize=(8, 5))
    ax.scatter(xc, yn, s=4, c="#2166ac", alpha=0.7, linewidths=0, rasterized=True)
    ax.plot(xc, yn, c="#2166ac", alpha=0.25, linewidth=0.5)

    ax.set_xlabel("Ciclo")
    ax.set_ylabel("Endereco relativo (unidades de 8 bytes)")
    ax.set_title(titulo)
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    plt.close(fig)


def plot_combinado_espacial_temporal(ciclos, addrs, titulo, out_path):
    """Dois paineis: zoom espacial (diagonal) e temporal (horizontal)."""
    c, a = filtrar_heap(ciclos, addrs)
    if len(c) < 100:
        c, a = ciclos, addrs

    fig, axes = plt.subplots(1, 2, figsize=(13, 5))

    for ax, tipo, cor, label in [
        (axes[0], "espacial", "#2166ac", "Localidade espacial"),
        (axes[1], "temporal", "#7d3c98", "Localidade temporal"),
    ]:
        if tipo == "espacial":
            i, w = melhor_janela_espacial(c, a)
        else:
            i, w = melhor_janela_temporal(c, a)
        w = min(w, len(c) - i)
        xc = c[i : i + w]
        yn = (a[i : i + w] - a[i : i + w].min()) // 8

        ax.scatter(xc, yn, s=5, c=cor, alpha=0.75, linewidths=0, rasterized=True)
        ax.plot(xc, yn, c=cor, alpha=0.3, linewidth=0.6)
        ax.set_xlabel("Ciclo")
        ax.set_ylabel("Endereco relativo")
        ax.set_title(label)
        ax.grid(True, alpha=0.3)

    fig.suptitle(titulo, fontsize=12)
    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    plt.close(fig)


def plot_run_sequencial(ciclos, addrs, titulo, out_path):
    """Plota o maior segmento sequencial detectado (diagonal clara)."""
    c, a = filtrar_heap(ciclos, addrs)
    runs = extrair_runs_sequenciais(c, a, min_len=60)
    if not runs:
        runs = extrair_runs_sequenciais(ciclos, addrs, min_len=30)
        c, a = ciclos, addrs

    if not runs:
        plot_janela(ciclos, addrs, titulo + " (janela)", out_path, "espacial")
        return

    s, e = runs[0]
    xc = c[s:e]
    yn = (a[s:e] - a[s]) // 8

    fig, ax = plt.subplots(figsize=(8, 5))
    ax.scatter(xc, yn, s=6, c="#2166ac", alpha=0.85, linewidths=0)
    ax.plot(xc, yn, c="#c0392b", alpha=0.5, linewidth=1.0)
    ax.set_xlabel("Ciclo")
    ax.set_ylabel("Endereco relativo (offset / 8 B)")
    ax.set_title(titulo)
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    plt.close(fig)


def plot_comparacao(dados, out_path, suptitle):
    fig, axes = plt.subplots(1, 2, figsize=(14, 5.5))
    for ax, (ciclos, addrs, label) in zip(axes, dados):
        c, a = filtrar_heap(ciclos, addrs)
        if len(c) < 100:
            c, a = ciclos, addrs
        i, w = melhor_janela_espacial(c, a, janela=min(700, max(200, len(c) // 3)))
        w = min(w, len(c) - i)
        xc = c[i : i + w]
        yn = (a[i : i + w] - a[i : i + w].min()) // 8
        ax.scatter(xc, yn, s=5, c="#2166ac", alpha=0.75, linewidths=0)
        ax.plot(xc, yn, c="#2166ac", alpha=0.35, linewidth=0.6)
        ax.set_xlabel("Ciclo")
        ax.set_ylabel("Endereco relativo")
        ax.set_title(label)
        ax.grid(True, alpha=0.3)
    fig.suptitle(suptitle, fontsize=12)
    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    plt.close(fig)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output", default=str(SAIDA))
    args = parser.parse_args()

    out_dir = Path(args.output)
    out_dir.mkdir(parents=True, exist_ok=True)

    def load(name):
        p = LACKEY_DIR / f"traco_{name}.csv"
        return carregar_traco(p) if p.exists() else None

    tracos = {n: load(n) for n in (
        "demo", "demo_matriz", "vetor", "hash",
        "historico_lista", "historico_matriz",
    )}

    # Figuras principais do relatorio: carga demo com zoom visivel.
    if tracos.get("demo"):
        c, a = tracos["demo"]
        plot_combinado_espacial_temporal(
            c, a,
            "Traço Lackey — ConjuntoIds, Vetor e MapaVetor (carga demo)",
            out_dir / "localidade_traco_vetor.png",
        )
        plot_run_sequencial(
            c, a,
            "Segmento sequencial — varredura em Vetor<int>",
            out_dir / "localidade_traco_sequencial.png",
        )

    if tracos.get("demo_matriz"):
        c, a = tracos["demo_matriz"]
        plot_combinado_espacial_temporal(
            c, a,
            "Traço Lackey — HistoricoMatriz (varredura de linhas LU/LP)",
            out_dir / "localidade_traco_historico.png",
        )

    if tracos.get("vetor") and tracos.get("hash"):
        plot_janela(
            *tracos["hash"],
            "Traço Lackey — Tabela Hash (consultas)",
            out_dir / "localidade_traco_hash.png",
            "espacial",
        )
        plot_comparacao(
            [
                (*tracos["vetor"], "MapaVetor (MercadoDCC)"),
                (*tracos["hash"], "Tabela Hash (MercadoDCC)"),
            ],
            out_dir / "localidade_traco_comparacao.png",
            "Zoom em localidade espacial — executavel completo",
        )

    if tracos.get("historico_lista") and tracos.get("historico_matriz"):
        plot_comparacao(
            [
                (*tracos["historico_lista"], "HistoricoLista"),
                (*tracos["historico_matriz"], "HistoricoMatriz"),
            ],
            out_dir / "localidade_traco_historico_completo.png",
            "Zoom — historico no executavel completo",
        )

    print(f"Graficos em {out_dir}/")


if __name__ == "__main__":
    main()
