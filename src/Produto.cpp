#include "Produto.hpp"
#include "MedidorMemoria.hpp"
#include <iostream>
#include <iomanip>

Produto::Produto(int id, const std::string& nome, double preco, int qtd,
        const std::string& categoria, const std::string& marca,
        const std::string& condicao): _id(id), _qtd(qtd), _preco(preco), _nome(nome),
      _categoria(categoria), _marca(marca), _condicao(condicao) {}

int Produto::get_id() const {
    return _id;
}

int Produto::get_qtd() const {
    return _qtd;
}

double Produto::get_preco() const {
    return _preco;
}

const std::string& Produto::get_nome() const {
    return _nome;
}

const std::string& Produto::get_categoria() const {
    return _categoria;
}

const std::string& Produto::get_marca() const {
    return _marca;
}

const std::string& Produto::get_condicao() const {
    return _condicao;
}

bool Produto::tem_estoque(int quantidade) const {
    return _qtd >= quantidade;
}

void Produto::decrementar(int quantidade) {
    _qtd -= quantidade;
}

void Produto::incrementar(int quantidade) {
    _qtd += quantidade;
}

void Produto::imprimir_consulta(int indice_resultado) const {
    std::cout << "LP resultado_" << indice_resultado << " produto " << _id << " "<< _nome << " "
              << std::fixed << std::setprecision(2) << _preco
              << std::defaultfloat << " " << _qtd << " "
              << _categoria << " " << _marca << " " << _condicao << '\n';
}

long long Produto::memoria_bytes() const {
    return memoria_string(_nome) + memoria_string(_categoria) + 
    memoria_string(_marca) + memoria_string(_condicao);
}
