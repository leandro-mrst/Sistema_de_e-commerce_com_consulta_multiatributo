#ifndef SISTEMA_HPP
#define SISTEMA_HPP
#include <string>
#include "Vetor.hpp"
#include "Usuario.hpp"
#include "Produto.hpp"
#include "Compra.hpp"
#include "Reposicao.hpp"
#include "HistoricoCompras.hpp"
#include "IndiceInvertido.hpp"
#include "IndiceInvertidoInt.hpp"
#include "IndiceOrdenado.hpp"
#include "ConjuntoIds.hpp"
#include "ConsultaParser.hpp"
#include "MedidorMemoria.hpp"

class ConsultaEvaluator;
//orquestrados principal
class Sistema {
    //armazenamento principal, id da entidade == indice no vetor.
    Vetor<Usuario> _usuarios;
    Vetor<Produto> _produtos;
    Vetor<Compra> _compras;
    Vetor<Reposicao> _reposicoes;

    int _prox_id_usuario;
    int _prox_id_produto;
    int _prox_id_compra;// so incrementa em compra valida
    int _prox_id_reposicao;

    // indices de usuario
    IndiceInvertido _idx_u_nome;
    IndiceInvertido _idx_u_cidade;
    IndiceInvertido _idx_u_estado;
    IndiceInvertido _idx_u_nacionalidade;
    IndiceInvertidoInt _idx_u_id;
    IndiceInvertidoInt _idx_u_idade;
    IndiceOrdenado<int> _idx_u_idade_faixa;

    // indices de produto
    IndiceInvertido _idx_p_nome;
    IndiceInvertido _idx_p_categoria;
    IndiceInvertido _idx_p_marca;
    IndiceInvertido _idx_p_condicao;
    IndiceInvertidoInt _idx_p_id;
    IndiceOrdenado<double> _idx_p_preco;
    IndiceOrdenado<int> _idx_p_qtd;//atualizado a cada C/R valida

    // indices de compra
    IndiceInvertidoInt _idx_c_id;
    IndiceInvertidoInt _idx_c_timestamp;
    IndiceInvertidoInt _idx_c_usuario;
    IndiceInvertidoInt _idx_c_produto;
    IndiceOrdenado<int> _idx_c_timestamp_faixa;

    //i ndices de reposicao
    IndiceInvertidoInt _idx_r_id;
    IndiceInvertidoInt _idx_r_timestamp;
    IndiceInvertidoInt _idx_r_produto;
    IndiceOrdenado<int> _idx_r_timestamp_faixa;

    HistoricoCompras _historico;

    // Resolve um filtro de igualdade exata para a entidade indicada por cmd.
    ConjuntoIds filtro_usuario_exato(const std::string& attr, const std::string& valor) const;
    ConjuntoIds filtro_produto_exato(const std::string& attr, const std::string& valor) const;
    ConjuntoIds filtro_compra_exato(const std::string& attr, const std::string& valor) const;
    ConjuntoIds filtro_reposicao_exato(const std::string& attr, const std::string& valor) const;

    // Reindexa qtd no IndiceOrdenado quando estoque muda (compra ou reposicao).
    void atualizar_indice_qtd_produto(int id_produto, int qtd_antiga, int qtd_nova);

    friend class ConsultaEvaluator;

public:
    Sistema();

    void cadastrar_usuario(const std::string& nome, int idade,
        const std::string& cidade, const std::string& estado,
    const std::string& nacionalidade);

    void cadastrar_produto(const std::string& nome, double preco, int qtd,
        const std::string& categoria, const std::string& marca, const std::string& condicao);

    void registrar_reposicao(int timestamp, const int* ids, const int* qtds, int n);
    void registrar_compra(int timestamp, int id_usuario, const int* ids, const int* qtds, int n);

    void consultar_usuarios(const Vetor<std::string>& tokens);
    void consultar_produtos(const Vetor<std::string>& tokens);
    void consultar_compras(const Vetor<std::string>& tokens);
    void consultar_reposicoes(const Vetor<std::string>& tokens);

    // Usado pelo ConsultaEvaluator, retorna ids que satisfazem um filtro atomico
    ConjuntoIds avaliar_filtro(const std::string& cmd, const FiltroAtomico& filtro) const;

    // Conjunto com todos os ids da entidade, utilizado para operador NOT
    ConjuntoIds universo_da_consulta(const std::string& cmd) const;

    RelatorioMemoria medir_memoria() const;
    void imprimir_memoria() const;
};

#endif 
