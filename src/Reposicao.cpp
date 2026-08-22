#include "Reposicao.hpp"
#include <iostream>

Reposicao::Reposicao(int id, int timestamp, const int* ids, const int* qtds, int n)
    : _id(id), _timestamp(timestamp), _n_produtos(n) {
    _id_produtos = new int[n];
    _qtd_produtos = new int[n];
    for (int i = 0; i < n; ++i) {
        _id_produtos[i] = ids[i];
        _qtd_produtos[i] = qtds[i];
    }
    ordenar_por_id_produto();
}

Reposicao::~Reposicao() {
    delete[] _id_produtos;
    delete[] _qtd_produtos;
}

Reposicao::Reposicao(const Reposicao& outro)
    : _id(outro._id), _timestamp(outro._timestamp), _n_produtos(outro._n_produtos),
      _id_produtos(0), _qtd_produtos(0) {
    if (_n_produtos > 0) {
        _id_produtos = new int[_n_produtos];
        _qtd_produtos = new int[_n_produtos];
        for (int i = 0; i < _n_produtos; ++i) {
            _id_produtos[i] = outro._id_produtos[i];
            _qtd_produtos[i] = outro._qtd_produtos[i];
        }
    }
}

Reposicao& Reposicao::operator=(const Reposicao& outro) {
    if (this == &outro) {
        return *this;
    }
    delete[] _id_produtos;
    delete[] _qtd_produtos;
    _id = outro._id;
    _timestamp = outro._timestamp;
    _n_produtos = outro._n_produtos;
    _id_produtos = 0;
    _qtd_produtos = 0;
    if (_n_produtos > 0) {
        _id_produtos = new int[_n_produtos];
        _qtd_produtos = new int[_n_produtos];
        for (int i = 0; i < _n_produtos; ++i) {
            _id_produtos[i] = outro._id_produtos[i];
            _qtd_produtos[i] = outro._qtd_produtos[i];
        }
    }
    return *this;
}

void Reposicao::ordenar_por_id_produto() {
    for (int i = 0; i < _n_produtos; ++i) {
        for (int j = i + 1; j < _n_produtos; ++j) {
            if (_id_produtos[j] < _id_produtos[i]) {
                int tmp_id = _id_produtos[i];
                _id_produtos[i] = _id_produtos[j];
                _id_produtos[j] = tmp_id;
                int tmp_qtd = _qtd_produtos[i];
                _qtd_produtos[i] = _qtd_produtos[j];
                _qtd_produtos[j] = tmp_qtd;
            }
        }
    }
}

int Reposicao::get_id() const {
    return _id;
}

int Reposicao::get_timestamp() const {
    return _timestamp;
}

bool Reposicao::contem_produto(int id_produto) const {
    for (int i = 0; i < _n_produtos; ++i) {
        if (_id_produtos[i] == id_produto) {
            return true;
        }
    }
    return false;
}

void Reposicao::imprimir_consulta(int indice_resultado) const {
    std::cout << "LR resultado_" << indice_resultado << " reposicao " << _id
              << " timestamp " << _timestamp << '\n';
    for (int i = 0; i < _n_produtos; ++i) {
        std::cout << "produto_" << (i + 1) << " " << _id_produtos[i]
                  << " " << _qtd_produtos[i];
        if (i + 1 < _n_produtos) {
            std::cout << ' ';
        }
    }
    std::cout << '\n';
}

long long Reposicao::memoria_bytes() const {
    if (_n_produtos <= 0) {
        return 0;
    }
    return static_cast<long long>(_n_produtos) * 2 *
           static_cast<long long>(sizeof(int));
}
