#include "MapaVetor.hpp"
#include "MedidorMemoria.hpp"

int MapaVetorInt::buscar_pos(int chave) const {
    int lo = 0;
    int hi = _entradas.tamanho() - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (_entradas[mid].chave < chave) {
            lo = mid + 1;
        } else if (_entradas[mid].chave > chave) {
            hi = mid - 1;
        } else {
            return mid;
        }
    }
    return lo;
}

void MapaVetorInt::inserir(int chave, int id) {
    int pos = buscar_pos(chave);
    if (pos < _entradas.tamanho() && _entradas[pos].chave == chave) {
        _entradas[pos].ids.inserir(id);
        return;
    }

    EntradaVetorInt vazia;
    vazia.chave = 0;
    _entradas.push_back(vazia);
    for (int i = _entradas.tamanho() - 1; i > pos; --i) {
        _entradas[i] = _entradas[i - 1];
    }
    _entradas[pos].chave = chave;
    _entradas[pos].ids = ConjuntoIds();
    _entradas[pos].ids.inserir(id);
}

const ConjuntoIds& MapaVetorInt::buscar(int chave) const {
    int pos = buscar_pos(chave);
    if (pos < _entradas.tamanho() && _entradas[pos].chave == chave) {
        return _entradas[pos].ids;
    }
    return ConjuntoIds::conjunto_vazio();
}

long long MapaVetorInt::memoria_bytes() const {
    long long total = _entradas.memoria_buffer();
    for (int i = 0; i < _entradas.tamanho(); ++i) {
        total += memoria_chave_int(_entradas[i].chave);
        total += _entradas[i].ids.memoria_bytes();
    }
    return total;
}

int MapaVetorString::buscar_pos(const std::string& chave) const {
    int lo = 0;
    int hi = _entradas.tamanho() - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (_entradas[mid].chave < chave) {
            lo = mid + 1;
        } else if (_entradas[mid].chave > chave) {
            hi = mid - 1;
        } else {
            return mid;
        }
    }
    return lo;
}

void MapaVetorString::inserir(const std::string& chave, int id) {
    int pos = buscar_pos(chave);
    if (pos < _entradas.tamanho() && _entradas[pos].chave == chave) {
        _entradas[pos].ids.inserir(id);
        return;
    }

    EntradaVetorString vazia;
    _entradas.push_back(vazia);
    for (int i = _entradas.tamanho() - 1; i > pos; --i) {
        _entradas[i] = _entradas[i - 1];
    }
    _entradas[pos].chave = chave;
    _entradas[pos].ids = ConjuntoIds();
    _entradas[pos].ids.inserir(id);
}

const ConjuntoIds& MapaVetorString::buscar(const std::string& chave) const {
    int pos = buscar_pos(chave);
    if (pos < _entradas.tamanho() && _entradas[pos].chave == chave) {
        return _entradas[pos].ids;
    }
    return ConjuntoIds::conjunto_vazio();
}

long long MapaVetorString::memoria_bytes() const {
    long long total = _entradas.memoria_buffer();
    for (int i = 0; i < _entradas.tamanho(); ++i) {
        total += memoria_string(_entradas[i].chave);
        total += _entradas[i].ids.memoria_bytes();
    }
    return total;
}
