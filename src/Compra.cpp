#include "Compra.hpp"
#include <iostream>

Compra::Compra(int id, int timestamp, int id_usuario,
        const int* ids, const int* qtds, int n)
    : _id(id), _timestamp(timestamp), _id_usuario(id_usuario), _n_produtos(n) {
    _id_produtos = new int[n];
    _qtd_produtos = new int[n];
    for (int i = 0; i < n; ++i) {
        _id_produtos[i] = ids[i];
        _qtd_produtos[i] = qtds[i];
    }
    ordenar_por_id_produto();
}

Compra::~Compra() {
    delete[] _id_produtos;
    delete[] _qtd_produtos;
}

Compra::Compra(const Compra& outro)
    : _id(outro._id), _timestamp(outro._timestamp), _id_usuario(outro._id_usuario),
      _n_produtos(outro._n_produtos), _id_produtos(0), _qtd_produtos(0) {
    if (_n_produtos > 0) {
        _id_produtos = new int[_n_produtos];
        _qtd_produtos = new int[_n_produtos];
        for (int i = 0; i < _n_produtos; ++i) {
            _id_produtos[i] = outro._id_produtos[i];
            _qtd_produtos[i] = outro._qtd_produtos[i];
        }
    }
}

Compra& Compra::operator=(const Compra& outro) {
    if (this == &outro) {
        return *this;
    }
    delete[] _id_produtos;
    delete[] _qtd_produtos;
    _id = outro._id;
    _timestamp = outro._timestamp;
    _id_usuario = outro._id_usuario;
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

void Compra::ordenar_por_id_produto() {
    //produtos na saida LC devem aparecer em ordem crescente de id
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

int Compra::get_id() const {
    return _id;
}

int Compra::get_timestamp() const {
    return _timestamp;
}

int Compra::get_id_usuario() const {
    return _id_usuario;
}

bool Compra::contem_produto(int id_produto) const {
    for (int i = 0; i < _n_produtos; ++i) {
        if (_id_produtos[i] == id_produto) {
            return true;
        }
    }
    return false;
}

void Compra::imprimir_consulta(int indice_resultado) const {
    std::cout << "LC resultado_" << indice_resultado << " compra " << _id
              << " timestamp " << _timestamp << " usuario " << _id_usuario << '\n';
    for (int i = 0; i < _n_produtos; ++i) {
        std::cout << "produto_" << (i + 1) << " " << _id_produtos[i]
                  << " " << _qtd_produtos[i];
        if (i + 1 < _n_produtos) {
            std::cout << ' ';
        }
    }
    std::cout << '\n';
}

long long Compra::memoria_bytes() const {
    if (_n_produtos <= 0) {
        return 0;
    }
    return static_cast<long long>(_n_produtos) * 2 *
           static_cast<long long>(sizeof(int));
}
