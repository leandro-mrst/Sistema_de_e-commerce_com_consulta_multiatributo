#ifndef INDICE_ORDENADO_HPP
#define INDICE_ORDENADO_HPP

//indice ordenado por valor numerico
#include "ConjuntoIds.hpp"
#include "MedidorMemoria.hpp"

template<typename T>
class IndiceOrdenado {
private:
    struct No {
        T chave;
        ConjuntoIds ids;
        int altura;
        No* esq;
        No* dir;

        No(const T& chave);
    };

    No* _raiz;

    int altura(No* no) const;
    int balanceamento(No* no) const;
    void atualizar_altura(No* no);
    No* rotacionar_direita(No* a);
    No* rotacionar_esquerda(No* b);
    No* balancear(No* no);
    No* inserir_rec(No* no, const T& chave, int id);
    No* buscar_no(No* no, const T& chave) const;
    No* remover_rec(No* no, const T& chave, int id);
    void faixa_rec(No* no, const T& min, const T& max, ConjuntoIds& resultado) const;
    void destruir_rec(No* no);
    long long memoria_rec(No* no) const;

public:
    IndiceOrdenado();
    ~IndiceOrdenado();

    void inserir(const T& chave, int id);
    void remover(const T& chave, int id);  // necessario para qtd dinamico em LP
    const ConjuntoIds& buscar(const T& chave) const;
    ConjuntoIds buscar_faixa(const T& min, const T& max) const;
    long long memoria_bytes() const;
};

#include "IndiceOrdenado.tpp"

#endif
