#ifndef INDICE_INVERTIDO_INT_HPP
#define INDICE_INVERTIDO_INT_HPP

//Mapeia valor int para ConjuntoIds de entidades com aquele valor
#include "MapaIndice.hpp"
#include "ConjuntoIds.hpp"

class IndiceInvertidoInt {
    private:
    MapaIndiceInt _mapa;

public:
    void inserir(int valor, int id);
    const ConjuntoIds& buscar(int valor) const;
    long long memoria_bytes() const;
};

#endif
