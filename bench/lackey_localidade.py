#!/usr/bin/env python3
"""
Traço Lackey — endereco x ciclo para analise de localidade.

Gera:
  traco_demo.csv         — carga isolada (ConjuntoIds, Vetor, MapaVetor)
  traco_demo_matriz.csv  — HistoricoMatriz (varredura contigua)
  traco_vetor/hash.csv   — executavel completo (consultas)
"""

import argparse
import csv
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BIN_VETOR = ROOT / "bin" / "tp3.out"
BIN_HASH = ROOT / "bin" / "tp3_hash.out"
BIN_HIST_MATRIZ = ROOT / "bin" / "tp3_hist_matriz.out"
BIN_DEMO = ROOT / "bin" / "localidade_demo.out"
BIN_DEMO_MATRIZ = ROOT / "bin" / "localidade_demo_matriz.out"
GERADOR = ROOT / "bench" / "gerar_entrada.py"
LACKEY_DIR = ROOT / "bench" / "lackey"

CARGA_TRACO = {
    "usuarios": 60, "produtos": 60, "compras": 15, "reposicoes": 15,
    "consultas_lu": 40, "consultas_lp": 40, "consultas_lc": 40, "consultas_lr": 40,
    "filtros": 2, "seed": 42,
}


def verificar_valgrind():
    try:
        subprocess.run(["valgrind", "--tool=lackey", "--help"],
                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
    except (FileNotFoundError, subprocess.CalledProcessError):
        print("Erro: valgrind nao encontrado.", file=sys.stderr)
        sys.exit(1)


def compilar_demos():
    cxx = "g++"
    flags = ["-std=c++11", "-Wall", "-Iinclude", "-O1"]
    core = [
        "src/ConjuntoIds.cpp", "src/MapaVetor.cpp", "src/MedidorMemoria.cpp",
        "src/Metricas.cpp", "src/HistoricoMatriz.cpp",
    ]
    subprocess.check_call(
        [cxx, *flags, "bench/localidade_demo.cpp", *core,
         "-o", str(BIN_DEMO)], cwd=ROOT,
    )
    subprocess.check_call(
        [cxx, *flags, "-DUSE_HIST_MATRIZ", "bench/localidade_demo_matriz.cpp",
         "src/HistoricoMatriz.cpp", "-o", str(BIN_DEMO_MATRIZ)], cwd=ROOT,
    )


def gerar_entrada():
    p = CARGA_TRACO
    cmd = [
        sys.executable, str(GERADOR),
        "-u", str(p["usuarios"]), "-m", str(p["produtos"]),
        "--compras", str(p["compras"]), "--reposicoes", str(p["reposicoes"]),
        "--consultas-lu", str(p["consultas_lu"]), "--consultas-lp", str(p["consultas_lp"]),
        "--consultas-lc", str(p["consultas_lc"]), "--consultas-lr", str(p["consultas_lr"]),
        "--filtros", str(p["filtros"]), "--seed", str(p["seed"]),
        "--sem-metricas", "--sem-memoria",
    ]
    return subprocess.check_output(cmd, text=True)


def rodar_lackey(binario, log_path):
    log_path.parent.mkdir(parents=True, exist_ok=True)
    if log_path.exists():
        log_path.unlink()
    subprocess.run(
        ["valgrind", "--tool=lackey", "--trace-mem=yes", f"--log-file={log_path}", str(binario)],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True,
    )


def contar_loads(log_path):
    n = 0
    with open(log_path, encoding="utf-8", errors="replace") as f:
        for linha in f:
            if linha.startswith(" L "):
                n += 1
    return n


def extrair_traco(log_path, max_points, skip_frac=0.0):
    total = contar_loads(log_path)
    if total == 0:
        return [], 0
    pular = int(total * skip_frac)
    ciclo = 0
    pontos = []
    with open(log_path, encoding="utf-8", errors="replace") as f:
        for linha in f:
            if not linha.startswith(" L "):
                continue
            ciclo += 1
            if ciclo <= pular:
                continue
            addr = int(linha.split()[1].split(",")[0], 16)
            pontos.append((ciclo, addr))
            if len(pontos) >= max_points:
                break
    return pontos, total


def salvar_csv(pontos, csv_path, variante, total_loads, skip_frac):
    if not pontos:
        return
    with open(csv_path, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["variante", "ciclo", "endereco", "endereco_norm"])
        addr_min = min(a for _, a in pontos)
        for ciclo, addr in pontos:
            w.writerow([variante, ciclo, addr, (addr - addr_min) // 8])


def executar(nome, binario, entrada, max_points, skip_frac, keep_raw):
    log_path = LACKEY_DIR / f"lackey_{nome}.log"
    csv_path = LACKEY_DIR / f"traco_{nome}.csv"
    print(f"== Lackey [{nome}] ==")
    if entrada is not None:
        subprocess.run(
            ["valgrind", "--tool=lackey", "--trace-mem=yes",
             f"--log-file={log_path}", str(binario)],
            input=entrada, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
            text=True, check=True,
        )
    else:
        rodar_lackey(binario, log_path)
    pontos, total = extrair_traco(log_path, max_points, skip_frac)
    salvar_csv(pontos, csv_path, nome, total, skip_frac)
    print(f"  loads: {total:,}  amostra: {len(pontos):,}")
    if not keep_raw:
        log_path.unlink(missing_ok=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--max-points", type=int, default=40000)
    parser.add_argument("--skip-frac", type=float, default=0.42)
    parser.add_argument("--keep-raw", action="store_true")
    parser.add_argument("--skip-build", action="store_true")
    args = parser.parse_args()

    verificar_valgrind()
    if not args.skip_build:
        subprocess.check_call(["make", "all"], cwd=ROOT)
        subprocess.check_call(["make", "hash-build"], cwd=ROOT)
        subprocess.check_call(["make", "hist-matriz-build"], cwd=ROOT)
    compilar_demos()

    entrada = gerar_entrada()
    LACKEY_DIR.mkdir(parents=True, exist_ok=True)

    executar("demo", BIN_DEMO, None, args.max_points, 0.0, args.keep_raw)
    executar("demo_matriz", BIN_DEMO_MATRIZ, None, args.max_points, 0.0, args.keep_raw)
    executar("vetor", BIN_VETOR, entrada, args.max_points, args.skip_frac, args.keep_raw)
    executar("hash", BIN_HASH, entrada, args.max_points, args.skip_frac, args.keep_raw)
    executar("historico_lista", BIN_VETOR, entrada, args.max_points, args.skip_frac, args.keep_raw)
    executar("historico_matriz", BIN_HIST_MATRIZ, entrada, args.max_points, args.skip_frac, args.keep_raw)

    print("\nGraficos: python3 bench/plot_traco_localidade.py")


if __name__ == "__main__":
    main()
