#ifndef CONSULTA_EVALUATOR_HPP
#define CONSULTA_EVALUATOR_HPP

#include <string>
#include "Vetor.hpp"
#include "ConsultaParser.hpp"
#include "ConjuntoIds.hpp"

class Sistema;

//Avalia consultas e retorna conjuntoIds de resultados
//seguindo o floxo, ConsultaParser detecta modo And implicito ou booleano
//para o modo AND implicito usa intersect_multiplo dos filtros
//para modo booleano converte infixo para rpn, depois avalia a pilha
class ConsultaEvaluator {
private:
    const Sistema* _sistema;

    ConjuntoIds avaliar_and(const std::string& cmd, const Vetor<std::string>& tokens);
    ConjuntoIds avaliar_expr(const std::string& cmd, const Vetor<std::string>& tokens);
    ConjuntoIds avaliar_rpn(const std::string& cmd, const Vetor<TokenExpr>& rpn);
    ConjuntoIds resolver_filtro(const std::string& cmd, const FiltroAtomico& filtro);
    ConjuntoIds universo(const std::string& cmd) const;

    int precedencia(TipoTokenExpr tipo) const;
    Vetor<TokenExpr> para_rpn(const Vetor<TokenExpr>& infix) const;

public:
    explicit ConsultaEvaluator(const Sistema* sistema);

    //tokens apos o comando LU/LP/LC/LR.
    ConjuntoIds avaliar(const std::string& cmd, const Vetor<std::string>& tokens);
};

#endif
