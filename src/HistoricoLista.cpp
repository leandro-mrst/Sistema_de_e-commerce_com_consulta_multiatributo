#include "HistoricoLista.hpp"
#include "MedidorMemoria.hpp"
#include <iostream>

void HistoricoLista::inserir_ou_somar(Vetor<ParIdQtd>& lista, int id, int qtd) {
    int n = lista.tamanho();
    int pos = 0;
    while (pos < n && lista[pos].id < id) {
        ++pos;
    }
    if (pos < n && lista[pos].id == id) {
        lista[pos].qtd += qtd;
        return;
    }

    ParIdQtd vazio;
    vazio.id = 0;
    vazio.qtd = 0;
    lista.push_back(vazio);
    for (int i = n; i > pos; --i) {
        lista[i] = lista[i - 1];
    }
    lista[pos].id = id;
    lista[pos].qtd = qtd;
}

void HistoricoLista::ao_cadastrar_usuario(int id) {
    while (_por_usuario.tamanho() <= id) {
        Vetor<ParIdQtd> vazio;
        _por_usuario.push_back(vazio);
    }
}

void HistoricoLista::ao_cadastrar_produto(int id) {
    while (_por_produto.tamanho() <= id) {
        Vetor<ParIdQtd> vazio;
        _por_produto.push_back(vazio);
    }
}

void HistoricoLista::registrar_compra(int id_usuario,
        const int* id_produtos, const int* qtds, int n) {
    for (int i = 0; i < n; ++i) {
        inserir_ou_somar(_por_usuario[id_usuario], id_produtos[i], qtds[i]);
        inserir_ou_somar(_por_produto[id_produtos[i]], id_usuario, qtds[i]);
    }
}

void HistoricoLista::imprimir_produtos_usuario(int id_usuario) const {
    const Vetor<ParIdQtd>& lista = _por_usuario[id_usuario];
    if (lista.tamanho() == 0) {
        return;
    }
    for (int i = 0; i < lista.tamanho(); ++i) {
        std::cout << "produto_" << (i + 1) << " " << lista[i].id
                  << " " << lista[i].qtd;
        if (i + 1 < lista.tamanho()) {
            std::cout << ' ';
        }
    }
    std::cout << '\n';
}

void HistoricoLista::imprimir_usuarios_produto(int id_produto) const {
    const Vetor<ParIdQtd>& lista = _por_produto[id_produto];
    if (lista.tamanho() == 0) {
        return;
    }
    for (int i = 0; i < lista.tamanho(); ++i) {
        std::cout << "usuario_" << (i + 1) << " " << lista[i].id
                  << " " << lista[i].qtd;
        if (i + 1 < lista.tamanho()) {
            std::cout << ' ';
        }
    }
    std::cout << '\n';
}

long long HistoricoLista::memoria_bytes() const {
    long long total = _por_usuario.memoria_buffer() + _por_produto.memoria_buffer();
    for (int i = 0; i < _por_usuario.tamanho(); ++i) {
        total += _por_usuario[i].memoria_buffer();
    }
    for (int i = 0; i < _por_produto.tamanho(); ++i) {
        total += _por_produto[i].memoria_buffer();
    }
    return total;
}
