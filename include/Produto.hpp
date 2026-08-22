#ifndef PRODUTO_HPP
#define PRODUTO_HPP
//entidade cadastrada pelo comando P
#include <string>

class Produto {
private:
    int _id;
    int _qtd;
    double _preco;
    std::string _nome;
    std::string _categoria;
    std::string _marca;
    std::string _condicao;

public:
    Produto(int id, const std::string& nome, double preco, int qtd,
     const std::string& categoria, const std::string& marca, const std::string& condicao);

    int get_id() const;
    int get_qtd() const;
    double get_preco() const;
    const std::string& get_nome() const;
    const std::string& get_categoria() const;
    const std::string& get_marca() const;
    const std::string& get_condicao() const;

    bool tem_estoque(int quantidade) const;
    void decrementar(int quantidade);
    void incrementar(int quantidade);

    // Formato: LP resultado_i produto <id> <nome> <preco> <qtd> <categoria> <marca> <condicao>
    void imprimir_consulta(int indice_resultado) const;
    long long memoria_bytes() const;
};

#endif
