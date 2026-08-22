#include "HistoricoMatriz.hpp"
#include <iostream>

void HistoricoMatriz::expandir_linha(Vetor<int>& linha, int novo_tamanho) {
    while (linha.tamanho() < novo_tamanho) {
        int zero = 0;
        linha.push_back(zero);
    }
}

void HistoricoMatriz::somar_celula(Vetor<int>& linha, int indice, int qtd) {
    expandir_linha(linha, indice + 1);
    linha[indice] += qtd;
}

void HistoricoMatriz::ao_cadastrar_usuario(int id) {
    while (_usuario_produto.tamanho() <= id) {
        Vetor<int> linha;
        expandir_linha(linha, _produto_usuario.tamanho());
        _usuario_produto.push_back(linha);
    }
    for (int p = 0; p < _produto_usuario.tamanho(); ++p) {
        expandir_linha(_produto_usuario[p], id + 1);
    }
}

void HistoricoMatriz::ao_cadastrar_produto(int id) {
    while (_produto_usuario.tamanho() <= id) {
        Vetor<int> linha;
        expandir_linha(linha, _usuario_produto.tamanho());
        _produto_usuario.push_back(linha);
    }
    for (int u = 0; u < _usuario_produto.tamanho(); ++u) {
        expandir_linha(_usuario_produto[u], id + 1);
    }
}

void HistoricoMatriz::registrar_compra(int id_usuario,
        const int* id_produtos, const int* qtds, int n) {
    for (int i = 0; i < n; ++i) {
        int pid = id_produtos[i];
        somar_celula(_usuario_produto[id_usuario], pid, qtds[i]);
        somar_celula(_produto_usuario[pid], id_usuario, qtds[i]);
    }
}

void HistoricoMatriz::imprimir_produtos_usuario(int id_usuario) const {
    if (id_usuario < 0 || id_usuario >= _usuario_produto.tamanho()) {
        return;
    }
    const Vetor<int>& linha = _usuario_produto[id_usuario];
    int rotulo = 0;
    for (int p = 0; p < linha.tamanho(); ++p) {
        if (linha[p] <= 0) {
            continue;
        }
        if (rotulo > 0) {
            std::cout << ' ';
        }
        ++rotulo;
        std::cout << "produto_" << rotulo << " " << p << " " << linha[p];
    }
    if (rotulo > 0) {
        std::cout << '\n';
    }
}

void HistoricoMatriz::imprimir_usuarios_produto(int id_produto) const {
    if (id_produto < 0 || id_produto >= _produto_usuario.tamanho()) {
        return;
    }
    const Vetor<int>& linha = _produto_usuario[id_produto];
    int rotulo = 0;
    for (int u = 0; u < linha.tamanho(); ++u) {
        if (linha[u] <= 0) {
            continue;
        }
        if (rotulo > 0) {
            std::cout << ' ';
        }
        ++rotulo;
        std::cout << "usuario_" << rotulo << " " << u << " " << linha[u];
    }
    if (rotulo > 0) {
        std::cout << '\n';
    }
}

long long HistoricoMatriz::memoria_bytes() const {
    long long total = _usuario_produto.memoria_buffer() + _produto_usuario.memoria_buffer();
    for (int u = 0; u < _usuario_produto.tamanho(); ++u) {
        total += _usuario_produto[u].memoria_buffer();
    }
    for (int p = 0; p < _produto_usuario.tamanho(); ++p) {
        total += _produto_usuario[p].memoria_buffer();
    }
    return total;
}
