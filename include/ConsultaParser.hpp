#ifndef CONSULTA_PARSER_HPP
#define CONSULTA_PARSER_HPP

//Realiza a tokenizacao e analise sintatica de consultas LU/LP/LC/LR
// Suporte aos dois modos de consulta, AND implicito e expressao booleana
#include <string>
#include "Vetor.hpp"

// Filtro atomico: igualdade v1 ou faixa inclusiva [v1, v2] quando faixa == true.
struct FiltroAtomico {
    std::string atributo;
    bool faixa;
    std::string v1;
    std::string v2;
};

enum TipoTokenExpr {
    TOK_FILTRO, // filtro atomico atributo + valores
    TOK_NOT,
    TOK_AND,
    TOK_OR
};

struct TokenExpr {
    TipoTokenExpr tipo;
    FiltroAtomico filtro;//valido apenas quando tipo == TOK_FILTRO
};

// True se algum token for NOT, AND ou OR
bool tem_operador_booleano(const Vetor<std::string>& tokens);

// True se a string representa um numero
bool eh_numerico(const std::string& s);

// Verifica se o atributo aceita faixa na entidade cmd LU/LP/LC/LR.
bool atributo_suporta_faixa(const std::string& cmd, const std::string& attr);

// Modo AND implicito: lista de filtros atomicos
Vetor<FiltroAtomico> parse_filtros_and(const std::string& cmd,const Vetor<std::string>& tokens);

// Modo booleano: lista de tokens infixos (filtros + operadores).
Vetor<TokenExpr> parse_expr(const std::string& cmd, const Vetor<std::string>& tokens);

#endif
