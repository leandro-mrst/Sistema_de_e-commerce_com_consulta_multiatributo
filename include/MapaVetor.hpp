#ifndef MAPA_VETOR_HPP
#define MAPA_VETOR_HPP

//mapa chave para ConjuntoIds via vetor ordenado
#include <string>
#include "Vetor.hpp"
#include "ConjuntoIds.hpp"

struct EntradaVetorInt {
    int chave;
    ConjuntoIds ids;
};

struct EntradaVetorString {
    std::string chave;
    ConjuntoIds ids;
};

class MapaVetorInt {
    Vetor<EntradaVetorInt> _entradas;

    int buscar_pos(int chave) const;

public:
    void inserir(int chave, int id);
    const ConjuntoIds& buscar(int chave) const;
    long long memoria_bytes() const;
};

class MapaVetorString {
    Vetor<EntradaVetorString> _entradas;

    int buscar_pos(const std::string& chave) const;

public:
    void inserir(const std::string& chave, int id);
    const ConjuntoIds& buscar(const std::string& chave) const;
    long long memoria_bytes() const;
};

#endif