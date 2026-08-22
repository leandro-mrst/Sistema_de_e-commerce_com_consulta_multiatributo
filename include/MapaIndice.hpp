#ifndef MAPA_INDICE_HPP
#define MAPA_INDICE_HPP

//seleciona o indice invertido em tempo de compilacao
//padrao é utilizacao de vetor
//make hash utiliza tabela hash com encadeamento
#include <string>

#ifdef USE_HASH
#include "TabelaHash.hpp"

typedef MapaHashString MapaIndiceString;
typedef MapaHashInt MapaIndiceInt;

#else
#include "MapaVetor.hpp"

typedef MapaVetorString MapaIndiceString;
typedef MapaVetorInt MapaIndiceInt;

#endif

#endif // MAPA_INDICE_HPP
