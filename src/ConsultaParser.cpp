#include "ConsultaParser.hpp"

static bool eh_operador(const std::string& s) {
    return s == "AND" || s == "OR" || s == "NOT";
}

// Linha sem AND/OR/NOT -> todos os filtros sao combinados por interseccao
bool tem_operador_booleano(const Vetor<std::string>& tokens) {
    for (int i = 0; i < tokens.tamanho(); ++i) {
        if (eh_operador(tokens[i])) {
            return true;
        }
    }
    return false;
}

bool eh_numerico(const std::string& s) {
    if (s.empty()) {
        return false;
    }
    bool ponto = false;
    for (int i = 0; i < static_cast<int>(s.size()); ++i) {
        char c = s[i];
        if (c == '.') {
            if (ponto) {
                return false;
            }
            ponto = true;
        } else if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

bool atributo_suporta_faixa(const std::string& cmd, const std::string& attr) {
    if (cmd == "LU") {
        return attr == "idade";
    }
    if (cmd == "LP") {
        return attr == "preco" || attr == "qtd";
    }
    if (cmd == "LC" || cmd == "LR") {
        return attr == "timestamp";
    }
    return false;
}

static FiltroAtomico ler_filtro(const std::string& cmd,
        const Vetor<std::string>& tokens, int& pos) {
    FiltroAtomico filtro;
    filtro.atributo = tokens[pos++];
    filtro.faixa = false;

    // Dois numeros consecutivos apos atributo com faixa -> [min, max] inclusivo
    if (atributo_suporta_faixa(cmd, filtro.atributo)
            && pos + 1 < tokens.tamanho()
            && eh_numerico(tokens[pos])
            && eh_numerico(tokens[pos + 1])) {
        filtro.faixa = true;
        filtro.v1 = tokens[pos++];
        filtro.v2 = tokens[pos++];
    } else {
        filtro.v1 = tokens[pos++];
        filtro.v2 = "";
    }
    return filtro;
}

Vetor<FiltroAtomico> parse_filtros_and(const std::string& cmd,
        const Vetor<std::string>& tokens) {
    Vetor<FiltroAtomico> filtros;
    int pos = 0;
    while (pos < tokens.tamanho()) {
        filtros.push_back(ler_filtro(cmd, tokens, pos));
    }
    return filtros;
}

Vetor<TokenExpr> parse_expr(const std::string& cmd,
        const Vetor<std::string>& tokens) {
    Vetor<TokenExpr> saida;
    int pos = 0;
    while (pos < tokens.tamanho()) {
        if (tokens[pos] == "NOT") {
            TokenExpr t;
            t.tipo = TOK_NOT;
            saida.push_back(t);
            ++pos;
            continue;
        }
        if (tokens[pos] == "AND") {
            TokenExpr t;
            t.tipo = TOK_AND;
            saida.push_back(t);
            ++pos;
            continue;
        }
        if (tokens[pos] == "OR") {
            TokenExpr t;
            t.tipo = TOK_OR;
            saida.push_back(t);
            ++pos;
            continue;
        }

        TokenExpr t;
        t.tipo = TOK_FILTRO;
        t.filtro = ler_filtro(cmd, tokens, pos);
        saida.push_back(t);
    }
    return saida;
}
