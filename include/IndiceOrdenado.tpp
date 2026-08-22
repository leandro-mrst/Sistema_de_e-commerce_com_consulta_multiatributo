#ifndef INDICE_ORDENADO_TPP
#define INDICE_ORDENADO_TPP

#include "Metricas.hpp"

template<typename T>
IndiceOrdenado<T>::No::No(const T& chave)
    : chave(chave), altura(1), esq(0), dir(0) {}

template<typename T>
int IndiceOrdenado<T>::altura(No* no) const {
    return no ? no->altura : 0;
}

template<typename T>
int IndiceOrdenado<T>::balanceamento(No* no) const {
    return no ? altura(no->esq) - altura(no->dir) : 0;
}

template<typename T>
void IndiceOrdenado<T>::atualizar_altura(No* no) {
    int h_esq = altura(no->esq);
    int h_dir = altura(no->dir);
    no->altura = 1 + (h_esq > h_dir ? h_esq : h_dir);
}

template<typename T>
typename IndiceOrdenado<T>::No* IndiceOrdenado<T>::rotacionar_direita(No* a) {
    No* b = a->esq;
    No* t = b->dir;
    b->dir = a;
    a->esq = t;
    atualizar_altura(a);
    atualizar_altura(b);
    return b;
}

template<typename T>
typename IndiceOrdenado<T>::No* IndiceOrdenado<T>::rotacionar_esquerda(No* b) {
    No* a = b->dir;
    No* t = a->esq;
    a->esq = b;
    b->dir = t;
    atualizar_altura(b);
    atualizar_altura(a);
    return a;
}

template<typename T>
typename IndiceOrdenado<T>::No* IndiceOrdenado<T>::balancear(No* no) {
    atualizar_altura(no);
    int bf = balanceamento(no);
    if (bf > 1) {
        if (balanceamento(no->esq) < 0) {
            no->esq = rotacionar_esquerda(no->esq);
        }
        return rotacionar_direita(no);
    }
    if (bf < -1) {
        if (balanceamento(no->dir) > 0) {
            no->dir = rotacionar_direita(no->dir);
        }
        return rotacionar_esquerda(no);
    }
    return no;
}

template<typename T>
typename IndiceOrdenado<T>::No* IndiceOrdenado<T>::inserir_rec(No* no, const T& chave, int id) {
    if (!no) {
        No* novo = new No(chave);
        novo->ids.inserir(id);
        return novo;
    }
    if (chave < no->chave) {
        no->esq = inserir_rec(no->esq, chave, id);
    } else if (no->chave < chave) {
        no->dir = inserir_rec(no->dir, chave, id);
    } else {
        no->ids.inserir(id);
        return no;
    }
    return balancear(no);
}

template<typename T>
typename IndiceOrdenado<T>::No* IndiceOrdenado<T>::buscar_no(No* no, const T& chave) const {
    if (!no) {
        return 0;
    }
    if (chave < no->chave) {
        return buscar_no(no->esq, chave);
    }
    if (no->chave < chave) {
        return buscar_no(no->dir, chave);
    }
    return no;
}

template<typename T>
typename IndiceOrdenado<T>::No* IndiceOrdenado<T>::remover_rec(No* no, const T& chave, int id) {
    if (!no) {
        return 0;
    }
    if (chave < no->chave) {
        no->esq = remover_rec(no->esq, chave, id);
    } else if (no->chave < chave) {
        no->dir = remover_rec(no->dir, chave, id);
    } else {
        no->ids.remover(id);
    }
    return balancear(no);
}

template<typename T>
void IndiceOrdenado<T>::faixa_rec(No* no, const T& min, const T& max,
        ConjuntoIds& resultado) const {
    if (!no) {
        return;
    }
    // Percorre apenas subarvores que podem conter chaves em [min, max].
    if (min < no->chave) {
        faixa_rec(no->esq, min, max, resultado);
    }
    if (no->chave >= min && no->chave <= max) {
        for (int i = 0; i < no->ids.tamanho(); ++i) {
            resultado.inserir(no->ids[i]);
        }
    }
    if (no->chave < max) {
        faixa_rec(no->dir, min, max, resultado);
    }
}

template<typename T>
void IndiceOrdenado<T>::destruir_rec(No* no) {
    if (!no) {
        return;
    }
    destruir_rec(no->esq);
    destruir_rec(no->dir);
    delete no;
}

template<typename T>
IndiceOrdenado<T>::IndiceOrdenado() : _raiz(0) {}

template<typename T>
IndiceOrdenado<T>::~IndiceOrdenado() {
    destruir_rec(_raiz);
}

template<typename T>
void IndiceOrdenado<T>::inserir(const T& chave, int id) {
    metricas_registrar_insercao_indice();
    _raiz = inserir_rec(_raiz, chave, id);
}

template<typename T>
void IndiceOrdenado<T>::remover(const T& chave, int id) {
    _raiz = remover_rec(_raiz, chave, id);
}

template<typename T>
const ConjuntoIds& IndiceOrdenado<T>::buscar(const T& chave) const {
    No* no = buscar_no(_raiz, chave);
    if (!no) {
        return ConjuntoIds::conjunto_vazio();
    }
    return no->ids;
}

template<typename T>
ConjuntoIds IndiceOrdenado<T>::buscar_faixa(const T& min, const T& max) const {
    ConjuntoIds resultado;
    faixa_rec(_raiz, min, max, resultado);
    return resultado;
}

template<typename T>
long long IndiceOrdenado<T>::memoria_rec(No* no) const {
    if (!no) {
        return 0;
    }
    long long total = static_cast<long long>(sizeof(No));
    total += memoria_chave_mapa(no->chave);
    total += no->ids.memoria_bytes();
    total += memoria_rec(no->esq);
    total += memoria_rec(no->dir);
    return total;
}

template<typename T>
long long IndiceOrdenado<T>::memoria_bytes() const {
    return memoria_rec(_raiz);
}

#endif