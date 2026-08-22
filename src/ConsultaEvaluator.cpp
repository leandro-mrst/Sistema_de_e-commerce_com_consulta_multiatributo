#include "ConsultaEvaluator.hpp"
#include "ConsultaParser.hpp"
#include "Sistema.hpp"

ConsultaEvaluator::ConsultaEvaluator(const Sistema* sistema) : _sistema(sistema) {}

// NOT=3, AND=2, OR=1, maior numero tem maior precedencia.
int ConsultaEvaluator::precedencia(TipoTokenExpr tipo) const {
    if (tipo == TOK_NOT) {
        return 3;
    }
    if (tipo == TOK_AND) {
        return 2;
    }
    if (tipo == TOK_OR) {
        return 1;
    }
    return 0;
}

// infix para notacao polonesa reversa RPN
Vetor<TokenExpr> ConsultaEvaluator::para_rpn(const Vetor<TokenExpr>& infix) const {
    Vetor<TokenExpr> saida;
    Vetor<TokenExpr> operadores;

    for (int i = 0; i < infix.tamanho(); ++i) {
        const TokenExpr& t = infix[i];
        if (t.tipo == TOK_FILTRO) {
            saida.push_back(t);
        } else if (t.tipo == TOK_NOT) {
            operadores.push_back(t);
        } else {
            while (operadores.tamanho() > 0) {
                TipoTokenExpr topo = operadores[operadores.tamanho() - 1].tipo;
                if (topo == TOK_NOT || precedencia(topo) >= precedencia(t.tipo)) {
                    saida.push_back(operadores[operadores.tamanho() - 1]);
                    operadores.pop_back();
                } else {
                    break;
                }
            }
            operadores.push_back(t);
        }
    }

    while (operadores.tamanho() > 0) {
        saida.push_back(operadores[operadores.tamanho() - 1]);
        operadores.pop_back();
    }
    return saida;
}

ConjuntoIds ConsultaEvaluator::resolver_filtro(const std::string& cmd,
        const FiltroAtomico& filtro) {
    return _sistema->avaliar_filtro(cmd, filtro);
}

ConjuntoIds ConsultaEvaluator::universo(const std::string& cmd) const {
    return _sistema->universo_da_consulta(cmd);
}

//pilha de ConjuntoIds: FILTRO empilha, NOT/AND/OR desempilham e recombinam
ConjuntoIds ConsultaEvaluator::avaliar_rpn(const std::string& cmd,
        const Vetor<TokenExpr>& rpn) {
    Vetor<ConjuntoIds> pilha;

    for (int i = 0; i < rpn.tamanho(); ++i) {
        const TokenExpr& t = rpn[i];
        if (t.tipo == TOK_FILTRO) {
            pilha.push_back(resolver_filtro(cmd, t.filtro));
        } else if (t.tipo == TOK_NOT) {
            ConjuntoIds a = pilha[pilha.tamanho() - 1];
            pilha.pop_back();
            // Complemento em relacao ao universo da entidade consultada.
            pilha.push_back(ConjuntoIds::complemento(universo(cmd), a));
        } else if (t.tipo == TOK_AND) {
            ConjuntoIds b = pilha[pilha.tamanho() - 1];
            pilha.pop_back();
            ConjuntoIds a = pilha[pilha.tamanho() - 1];
            pilha.pop_back();
            pilha.push_back(ConjuntoIds::intersect(a, b));
        } else if (t.tipo == TOK_OR) {
            ConjuntoIds b = pilha[pilha.tamanho() - 1];
            pilha.pop_back();
            ConjuntoIds a = pilha[pilha.tamanho() - 1];
            pilha.pop_back();
            pilha.push_back(ConjuntoIds::uniao(a, b));
        }
    }

    if (pilha.tamanho() == 0) {
        return ConjuntoIds();
    }
    return pilha[0];
}

ConjuntoIds ConsultaEvaluator::avaliar_and(const std::string& cmd,
        const Vetor<std::string>& tokens) {
    Vetor<FiltroAtomico> filtros = parse_filtros_and(cmd, tokens);
    Vetor<ConjuntoIds> conjuntos;
    for (int i = 0; i < filtros.tamanho(); ++i) {
        conjuntos.push_back(resolver_filtro(cmd, filtros[i]));
    }
    return ConjuntoIds::intersect_multiplo(conjuntos);
}

ConjuntoIds ConsultaEvaluator::avaliar_expr(const std::string& cmd,
        const Vetor<std::string>& tokens) {
    Vetor<TokenExpr> infix = parse_expr(cmd, tokens);
    Vetor<TokenExpr> rpn = para_rpn(infix);
    return avaliar_rpn(cmd, rpn);
}

ConjuntoIds ConsultaEvaluator::avaliar(const std::string& cmd,
        const Vetor<std::string>& tokens) {
    if (tem_operador_booleano(tokens)) {
        return avaliar_expr(cmd, tokens);
    }
    return avaliar_and(cmd, tokens);
}
