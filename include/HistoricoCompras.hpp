#ifndef HISTORICO_COMPRAS_HPP
#define HISTORICO_COMPRAS_HPP

//HistoicoCompras, implementacao com lista é o padrao,
//implementacao com matriz é feita com a flag -DUSE_HIST_MATRIZ 
#ifdef USE_HIST_MATRIZ
#include "HistoricoMatriz.hpp"
typedef HistoricoMatriz HistoricoCompras;
#else
#include "HistoricoLista.hpp"
typedef HistoricoLista HistoricoCompras;
#endif

#endif // HISTORICO_COMPRAS_HPP
