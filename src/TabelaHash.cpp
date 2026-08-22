#include "TabelaHash.hpp"
#include "MedidorMemoria.hpp"

// implementacao concreta das tabelas hash, variante experimental eh make hash

EntradaHashInt::EntradaHashInt(int chave) : chave(chave), prox(0) {}

EntradaHashString::EntradaHashString(const std::string& chave) : chave(chave), prox(0) {}

MapaHashInt::MapaHashInt() : _buckets(0), _capacidade(0), _tamanho(0) {
    _capacidade = 16;
    _buckets = new EntradaHashInt*[_capacidade];
    for (int i = 0; i < _capacidade; ++i) {
        _buckets[i] = 0;
    }
}

MapaHashInt::~MapaHashInt() {
    destruir();
}

unsigned long MapaHashInt::hash(int chave) const {
    unsigned long h = static_cast<unsigned long>(chave);
    return h % static_cast<unsigned long>(_capacidade);
}

void MapaHashInt::destruir() {
    for (int i = 0; i < _capacidade; ++i) {
        EntradaHashInt* atual = _buckets[i];
        while (atual) {
            EntradaHashInt* prox = atual->prox;
            delete atual;
            atual = prox;
        }
    }
    delete[] _buckets;
    _buckets = 0;
    _capacidade = 0;
    _tamanho = 0;
}

void MapaHashInt::redimensionar() {
    // Rehash: dobra capacidade e reinsere todas as entradas.
    int nova_cap = (_capacidade == 0) ? 16 : _capacidade * 2;
    EntradaHashInt** antigo = _buckets;
    int cap_antiga = _capacidade;

    _buckets = new EntradaHashInt*[nova_cap];
    _capacidade = nova_cap;
    _tamanho = 0;
    for (int i = 0; i < _capacidade; ++i) {
        _buckets[i] = 0;
    }

    for (int i = 0; i < cap_antiga; ++i) {
        EntradaHashInt* atual = antigo[i];
        while (atual) {
            EntradaHashInt* prox = atual->prox;
            for (int j = 0; j < atual->ids.tamanho(); ++j) {
                inserir(atual->chave, atual->ids[j]);
            }
            delete atual;
            atual = prox;
        }
    }
    delete[] antigo;
}

void MapaHashInt::inserir(int chave, int id) {
    if (_tamanho * 2 >= _capacidade) {
        redimensionar();
    }

    unsigned long indice = hash(chave);
    EntradaHashInt* atual = _buckets[indice];
    while (atual) {
        if (atual->chave == chave) {
            atual->ids.inserir(id);
            return;
        }
        atual = atual->prox;
    }

    EntradaHashInt* nova = new EntradaHashInt(chave);
    nova->ids.inserir(id);
    nova->prox = _buckets[indice];
    _buckets[indice] = nova;
    ++_tamanho;
}

const ConjuntoIds& MapaHashInt::buscar(int chave) const {
    if (_capacidade == 0) {
        return ConjuntoIds::conjunto_vazio();
    }
    unsigned long indice = hash(chave);
    EntradaHashInt* atual = _buckets[indice];
    while (atual) {
        if (atual->chave == chave) {
            return atual->ids;
        }
        atual = atual->prox;
    }
    return ConjuntoIds::conjunto_vazio();
}

long long MapaHashInt::memoria_bytes() const {
    long long total = static_cast<long long>(_capacidade) *
                      static_cast<long long>(sizeof(EntradaHashInt*));
    for (int i = 0; i < _capacidade; ++i) {
        EntradaHashInt* atual = _buckets[i];
        while (atual) {
            total += static_cast<long long>(sizeof(EntradaHashInt));
            total += atual->ids.memoria_bytes();
            atual = atual->prox;
        }
    }
    return total;
}

MapaHashString::MapaHashString() : _buckets(0), _capacidade(0), _tamanho(0) {
    _capacidade = 16;
    _buckets = new EntradaHashString*[_capacidade];
    for (int i = 0; i < _capacidade; ++i) {
        _buckets[i] = 0;
    }
}

MapaHashString::~MapaHashString() {
    destruir();
}

unsigned long MapaHashString::hash(const std::string& chave) const {
    unsigned long h = 0;
    for (int i = 0; i < static_cast<int>(chave.size()); ++i) {
        h = h * 31 + static_cast<unsigned char>(chave[i]);
    }
    return h % static_cast<unsigned long>(_capacidade);
}

void MapaHashString::destruir() {
    for (int i = 0; i < _capacidade; ++i) {
        EntradaHashString* atual = _buckets[i];
        while (atual) {
            EntradaHashString* prox = atual->prox;
            delete atual;
            atual = prox;
        }
    }
    delete[] _buckets;
    _buckets = 0;
    _capacidade = 0;
    _tamanho = 0;
}

void MapaHashString::redimensionar() {
    // Rehash: dobra capacidade e reinsere todas as entradas
    int nova_cap = (_capacidade == 0) ? 16 : _capacidade * 2;
    EntradaHashString** antigo = _buckets;
    int cap_antiga = _capacidade;

    _buckets = new EntradaHashString*[nova_cap];
    _capacidade = nova_cap;
    _tamanho = 0;
    for (int i = 0; i < _capacidade; ++i) {
        _buckets[i] = 0;
    }

    for (int i = 0; i < cap_antiga; ++i) {
        EntradaHashString* atual = antigo[i];
        while (atual) {
            EntradaHashString* prox = atual->prox;
            for (int j = 0; j < atual->ids.tamanho(); ++j) {
                inserir(atual->chave, atual->ids[j]);
            }
            delete atual;
            atual = prox;
        }
    }
    delete[] antigo;
}

void MapaHashString::inserir(const std::string& chave, int id) {
    if (_tamanho * 2 >= _capacidade) {
        redimensionar();
    }

    unsigned long indice = hash(chave);
    EntradaHashString* atual = _buckets[indice];
    while (atual) {
        if (atual->chave == chave) {
            atual->ids.inserir(id);
            return;
        }
        atual = atual->prox;
    }

    EntradaHashString* nova = new EntradaHashString(chave);
    nova->ids.inserir(id);
    nova->prox = _buckets[indice];
    _buckets[indice] = nova;
    ++_tamanho;
}

const ConjuntoIds& MapaHashString::buscar(const std::string& chave) const {
    if (_capacidade == 0) {
        return ConjuntoIds::conjunto_vazio();
    }
    unsigned long indice = hash(chave);
    EntradaHashString* atual = _buckets[indice];
    while (atual) {
        if (atual->chave == chave) {
            return atual->ids;
        }
        atual = atual->prox;
    }
    return ConjuntoIds::conjunto_vazio();
}

long long MapaHashString::memoria_bytes() const {
    long long total = static_cast<long long>(_capacidade) *
                      static_cast<long long>(sizeof(EntradaHashString*));
    for (int i = 0; i < _capacidade; ++i) {
        EntradaHashString* atual = _buckets[i];
        while (atual) {
            total += static_cast<long long>(sizeof(EntradaHashString));
            total += memoria_string(atual->chave);
            total += atual->ids.memoria_bytes();
            atual = atual->prox;
        }
    }
    return total;
}
