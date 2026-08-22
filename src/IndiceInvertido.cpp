#include "IndiceInvertido.hpp"
#include "Metricas.hpp"

void IndiceInvertido::inserir(const std::string& valor, int id) {
    metricas_registrar_insercao_indice();
    _mapa.inserir(valor, id);
}

const ConjuntoIds& IndiceInvertido::buscar(const std::string& valor) const {
    return _mapa.buscar(valor);
}

long long IndiceInvertido::memoria_bytes() const {
    return _mapa.memoria_bytes();
}
