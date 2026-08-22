#ifndef INDICE_INVERTIDO_HPP
#define INDICE_INVERTIDO_HPP

//IndiceInvertido para atributos textuais
//mapeia string para ConjuntoIds de entidades com aquele valor
#include <string>
#include "MapaIndice.hpp"
#include "ConjuntoIds.hpp"

class IndiceInvertido {
private:
    MapaIndiceString _mapa;

public:
    void inserir(const std::string& valor, int id);
    const ConjuntoIds& buscar(const std::string& valor) const;
    long long memoria_bytes() const;
};

#endif
