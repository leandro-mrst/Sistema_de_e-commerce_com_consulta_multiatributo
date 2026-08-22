#!/usr/bin/env python3
"""
Gera entrada sintetica para benchmark do MercadoDCC.

Perfis (secao 10.1 do enunciado):
  cadastros  — ~70%% U+P (custo de insercao nos indices)
  transacoes — ~50%% C+R (estoque e historicos)
  consultas  — ~70%% LU/LP/LC/LR (intersecao, faixas, booleanos)

Tipos de consulta (secao 10.3):
  exata    — AND implicito, igualdade
  faixa    — faixas numericas inclusivas
  booleano — expressoes com NOT/AND/OR
  misto    — combinacao aleatoria
"""

import argparse
import random
import sys

CIDADES = ["SaoPaulo", "RioDeJaneiro", "BeloHorizonte", "Curitiba", "Salvador"]
ESTADOS = ["SP", "RJ", "MG", "PR", "BA"]
NACIONALIDADES = ["Brasileira", "Argentina", "Chilena", "Portuguesa"]
CATEGORIAS = ["Eletronicos", "Livros", "Roupas", "Alimentos", "Esportes"]
MARCAS = ["MarcaA", "MarcaB", "MarcaC", "MarcaD", "MarcaE"]
CONDICOES = ["Novo", "Usado", "Recondicionado"]

CONSULTAS = ["LU", "LP", "LC", "LR"]


def distribuir_perfil(perfil, total):
    """Retorna (u, p, compras, reposicoes, consultas) conforme perfil."""
    util = max(0, total - 2)  # MEM + margem
    if perfil == "cadastros":
        cad = int(0.70 * util)
        u = cad // 2
        p = cad - u
        resto = util - cad
        c = resto // 3
        r = resto // 3
        q = resto - c - r
    elif perfil == "transacoes":
        base = max(200, int(0.10 * util))
        u = base // 2
        p = base - u
        trans = int(0.50 * (util - base))
        c = trans // 2
        r = trans - c
        q = util - base - trans
    elif perfil == "consultas":
        base = max(200, int(0.15 * util))
        u = base // 2
        p = base - u
        resto = util - base
        q = int(0.70 * resto)
        trans = resto - q
        c = trans // 2
        r = trans - c
    else:  # misto / custom via flags
        u = p = c = r = q = util // 5
    return max(1, u), max(1, p), max(0, c), max(0, r), max(0, q)


def filtro_exato_lu(rng, args):
    attr = rng.choice(["nome", "idade", "cidade", "estado", "nacionalidade"])
    if attr == "nome":
        return [attr, f"User{rng.randint(0, max(0, args.usuarios - 1))}"]
    if attr == "idade":
        return [attr, str(rng.randint(18, 70))]
    if attr == "cidade":
        return [attr, rng.choice(CIDADES)]
    if attr == "estado":
        return [attr, rng.choice(ESTADOS)]
    return [attr, rng.choice(NACIONALIDADES)]


def filtro_faixa_lu(rng, args):
    lo = rng.randint(18, 40)
    hi = lo + rng.randint(5, 25)
    return ["idade", str(lo), str(hi)]


def filtro_faixa_preco_qtd_lp(rng, args):
    """Faixa exclusiva em preco ou qtd (experimento de faixas LP)."""
    if rng.random() < 0.5:
        lo = round(rng.uniform(10.0, 100.0), 2)
        hi = round(lo + rng.uniform(50.0, 200.0), 2)
        return ["preco", f"{lo:.2f}", f"{hi:.2f}"]
    lo = rng.randint(5, 50)
    hi = lo + rng.randint(20, 100)
    return ["qtd", str(lo), str(hi)]


def filtro_exato_lp(rng, args):
    attr = rng.choice(["nome", "categoria", "marca", "condicao"])
    if attr == "nome":
        return [attr, f"Prod{rng.randint(0, max(0, args.produtos - 1))}"]
    if attr == "categoria":
        return [attr, rng.choice(CATEGORIAS)]
    if attr == "marca":
        return [attr, rng.choice(MARCAS)]
    return [attr, rng.choice(CONDICOES)]


def filtro_faixa_lp(rng, args):
    if rng.random() < 0.5:
        lo = round(rng.uniform(10.0, 100.0), 2)
        hi = round(lo + rng.uniform(50.0, 200.0), 2)
        return ["preco", f"{lo:.2f}", f"{hi:.2f}"]
    lo = rng.randint(5, 50)
    hi = lo + rng.randint(20, 100)
    return ["qtd", str(lo), str(hi)]


def filtro_exato_lc(rng, args, ts):
    attr = rng.choice(["id", "timestamp", "usuario", "produto"])
    if attr == "usuario":
        return [attr, str(rng.randint(0, max(0, args.usuarios - 1)))]
    if attr == "produto":
        return [attr, str(rng.randint(0, max(0, args.produtos - 1)))]
    if attr == "timestamp":
        return [attr, str(rng.randint(1, max(1, ts)))]
    return [attr, str(rng.randint(0, max(0, args.compras - 1)))]


def filtro_faixa_lc(rng, args, ts):
    lo = 1
    hi = max(lo, ts // 2)
    return ["timestamp", str(lo), str(hi)]


def filtro_exato_lr(rng, args, ts):
    attr = rng.choice(["id", "timestamp", "produto"])
    if attr == "produto":
        return [attr, str(rng.randint(0, max(0, args.produtos - 1)))]
    if attr == "timestamp":
        return [attr, str(rng.randint(1, max(1, ts)))]
    return [attr, str(rng.randint(0, max(0, args.reposicoes - 1)))]


def filtro_faixa_lr(rng, args, ts):
    return ["timestamp", "1", str(max(1, ts))]


def montar_filtros_and(rng, cmd, args, ts, tipo):
    nf = args.filtros
    partes = []
    for _ in range(nf):
        if tipo == "faixa":
            if cmd == "LU":
                partes.extend(filtro_faixa_lu(rng, args))
            elif cmd == "LP":
                partes.extend(filtro_faixa_lp(rng, args))
            elif cmd == "LC":
                partes.extend(filtro_faixa_lc(rng, args, ts))
            else:
                partes.extend(filtro_faixa_lr(rng, args, ts))
        elif tipo == "exata":
            if cmd == "LU":
                partes.extend(filtro_exato_lu(rng, args))
            elif cmd == "LP":
                partes.extend(filtro_exato_lp(rng, args))
            elif cmd == "LC":
                partes.extend(filtro_exato_lc(rng, args, ts))
            else:
                partes.extend(filtro_exato_lr(rng, args, ts))
        else:  # misto
            if rng.random() < 0.35:
                if cmd == "LU":
                    partes.extend(filtro_faixa_lu(rng, args))
                elif cmd == "LP":
                    partes.extend(filtro_faixa_lp(rng, args))
                elif cmd == "LC":
                    partes.extend(filtro_faixa_lc(rng, args, ts))
                else:
                    partes.extend(filtro_faixa_lr(rng, args, ts))
            else:
                if cmd == "LU":
                    partes.extend(filtro_exato_lu(rng, args))
                elif cmd == "LP":
                    partes.extend(filtro_exato_lp(rng, args))
                elif cmd == "LC":
                    partes.extend(filtro_exato_lc(rng, args, ts))
                else:
                    partes.extend(filtro_exato_lr(rng, args, ts))
    return partes


def montar_consulta_booleana(rng, cmd, args, ts):
    """Gera expressao booleana: F1 AND F2, F1 OR F2 ou NOT F1."""
    f1 = montar_filtros_and(rng, cmd, args, ts, "exata")
    if rng.random() < 0.25:
        return ["NOT"] + f1
    f2 = montar_filtros_and(rng, cmd, args, ts, "exata")
    op = rng.choice(["AND", "OR"])
    return f1 + [op] + f2


def escolher_tipo_consulta(rng, args):
    if getattr(args, "pct_booleano", 0) > 0:
        if rng.randint(1, 100) <= args.pct_booleano:
            return "booleano"
        return "exata"
    if args.tipo_consulta != "misto":
        return args.tipo_consulta
    return rng.choice(["exata", "faixa", "booleano"])


def gerar_consulta_cmd(rng, cmd, args, ts):
    pct_faixa = getattr(args, "pct_faixa_preco_qtd", 0)
    if cmd == "LP" and pct_faixa > 0 and rng.randint(1, 100) <= pct_faixa:
        partes = filtro_faixa_preco_qtd_lp(rng, args)
        if args.filtros > 1:
            for _ in range(args.filtros - 1):
                partes.extend(filtro_exato_lp(rng, args))
        return cmd + " " + " ".join(partes)

    tipo = escolher_tipo_consulta(rng, args)
    if tipo == "booleano":
        partes = montar_consulta_booleana(rng, cmd, args, ts)
    else:
        partes = montar_filtros_and(rng, cmd, args, ts, tipo)
    return cmd + " " + " ".join(partes)


def gerar_consulta(rng, args, ts):
    cmd = rng.choice(CONSULTAS)
    return gerar_consulta_cmd(rng, cmd, args, ts)


def gerar(args):
    rng = random.Random(args.seed)

    if args.perfil:
        u, p, c, r, q = distribuir_perfil(args.perfil, args.total_linhas)
        args.usuarios = u if args.usuarios is None else args.usuarios
        args.produtos = p if args.produtos is None else args.produtos
        args.compras = c if args.compras is None else args.compras
        args.reposicoes = r if args.reposicoes is None else args.reposicoes
        args.consultas = q if args.consultas is None else args.consultas
    else:
        args.usuarios = args.usuarios or 1000
        args.produtos = args.produtos or 1000
        args.compras = args.compras or 500
        args.reposicoes = args.reposicoes or 200
        args.consultas = args.consultas or 300

    linhas = []

    for i in range(args.usuarios):
        linhas.append(
            f"U User{i} {rng.randint(18, 70)} "
            f"{rng.choice(CIDADES)} {rng.choice(ESTADOS)} {rng.choice(NACIONALIDADES)}"
        )

    for i in range(args.produtos):
        linhas.append(
            f"P Prod{i} {rng.uniform(5.0, 500.0):.2f} {rng.randint(10, 200)} "
            f"{rng.choice(CATEGORIAS)} {rng.choice(MARCAS)} {rng.choice(CONDICOES)}"
        )

    ts = 1
    n_itens = args.itens_transacao
    for _ in range(args.reposicoes):
        n = min(n_itens, args.produtos)
        ids = rng.sample(range(args.produtos), max(1, n))
        pares = " ".join(f"{pid} {rng.randint(5, 30)}" for pid in ids)
        linhas.append(f"R {ts} {pares}")
        ts += 1

    for _ in range(args.compras):
        uid = rng.randint(0, max(0, args.usuarios - 1))
        n = min(n_itens, args.produtos)
        ids = rng.sample(range(args.produtos), max(1, n))
        pares = " ".join(f"{pid} {rng.randint(1, 3)}" for pid in ids)
        linhas.append(f"C {ts} {uid} {pares}")
        ts += 1

    consultas_por_tipo = [
        ("LU", getattr(args, "consultas_lu", None)),
        ("LP", getattr(args, "consultas_lp", None)),
        ("LC", getattr(args, "consultas_lc", None)),
        ("LR", getattr(args, "consultas_lr", None)),
    ]
    if any(n is not None for _, n in consultas_por_tipo):
        for cmd, n in consultas_por_tipo:
            if n is None:
                continue
            for _ in range(n):
                linhas.append(gerar_consulta_cmd(rng, cmd, args, ts))
    else:
        for _ in range(args.consultas):
            linhas.append(gerar_consulta(rng, args, ts))

    if args.com_metricas:
        linhas.append("MET")
    if args.com_memoria:
        linhas.append("MEM")
    return linhas


def main():
    parser = argparse.ArgumentParser(description="Gera entrada de benchmark MercadoDCC")
    parser.add_argument(
        "--perfil",
        choices=["cadastros", "transacoes", "consultas"],
        default="",
        help="Perfil de carga conforme secao 10.1 do enunciado",
    )
    parser.add_argument(
        "--total-linhas", type=int, default=10000,
        help="Total aproximado de linhas (com perfil)",
    )
    parser.add_argument("-u", "--usuarios", type=int, default=None)
    parser.add_argument("-m", "--produtos", type=int, default=None)
    parser.add_argument("--compras", type=int, default=None)
    parser.add_argument("--reposicoes", type=int, default=None)
    parser.add_argument("--consultas", type=int, default=None)
    parser.add_argument(
        "--filtros", type=int, default=2,
        help="Numero de filtros por consulta (1, 2 ou 4)",
    )
    parser.add_argument(
        "--itens-transacao", type=int, default=3,
        help="Produtos por compra/reposicao (testar 1, 5, 20)",
    )
    parser.add_argument(
        "--tipo-consulta",
        choices=["exata", "faixa", "booleano", "misto"],
        default="misto",
    )
    parser.add_argument(
        "--pct-booleano", type=int, default=0,
        help="Percentual de consultas booleanas (0-100); restante exata",
    )
    parser.add_argument(
        "--pct-faixa-preco-qtd", type=int, default=0,
        help="Percentual de consultas LP com faixa em preco ou qtd (0-100)",
    )
    parser.add_argument("--consultas-lu", type=int, default=None)
    parser.add_argument("--consultas-lp", type=int, default=None)
    parser.add_argument("--consultas-lc", type=int, default=None)
    parser.add_argument("--consultas-lr", type=int, default=None)
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--sem-metricas", action="store_true")
    parser.add_argument("--sem-memoria", action="store_true")
    parser.add_argument("-o", "--output", type=str, default="")
    args = parser.parse_args()

    args.com_metricas = not args.sem_metricas
    args.com_memoria = not args.sem_memoria

    linhas = gerar(args)
    texto = "\n".join(linhas) + "\n"
    if args.output:
        with open(args.output, "w", encoding="utf-8") as f:
            f.write(texto)
    else:
        sys.stdout.write(texto)


if __name__ == "__main__":
    main()
