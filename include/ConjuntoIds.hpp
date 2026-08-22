#ifndef CONJUNTOIDS_HPP
#define CONJUNTOIDS_HPP

#include "Vetor.hpp"

//Conjunto ordenado de ids, sem duplicatadas
//Utilizado em consultas multiatributos, cada filtro retorno um ConjuntoIds e 
//os resultados são combinados por AND, OR ou NOT em relacao ao universo da conulta
class ConjuntoIds {
private:
    Vetor<int> _ids;

    //append ao final quando o merge ja garante ordem crescente
    void append(int id);

public:
    ConjuntoIds();

    void inserir(int id);// insere mantendo ordenacao e ignora duplicatas
    void remover(int id);
    bool contem(int id) const;
    int tamanho() const;
    int operator[](int i) const;// i-esimo id em ordem crescente

    static const ConjuntoIds& conjunto_vazio();

    static ConjuntoIds intersect(const ConjuntoIds& a, const ConjuntoIds& b);
    static ConjuntoIds uniao(const ConjuntoIds& a, const ConjuntoIds& b);
    static ConjuntoIds complemento(const ConjuntoIds& universo, const ConjuntoIds& a);
    static ConjuntoIds intersect_multiplo(const Vetor<ConjuntoIds>& conjuntos);

    long long memoria_bytes() const;
};

#endif
