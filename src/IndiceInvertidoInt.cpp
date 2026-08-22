#include "IndiceInvertidoInt.hpp"
#include "Metricas.hpp"

void IndiceInvertidoInt::inserir(int valor, int id) {
    metricas_registrar_insercao_indice();
    _mapa.inserir(valor, id);
}

const ConjuntoIds& IndiceInvertidoInt::buscar(int valor) const {
    return _mapa.buscar(valor);
}

long long IndiceInvertidoInt::memoria_bytes() const {
    return _mapa.memoria_bytes();
}
