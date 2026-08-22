#include "ConjuntoIds.hpp"
#include "MedidorMemoria.hpp"
#include "Metricas.hpp"

ConjuntoIds::ConjuntoIds() {}

void ConjuntoIds::append(int id) {
    _ids.push_back(id);
}

void ConjuntoIds::inserir(int id) {
    int n = _ids.tamanho();
    int pos = 0;
    while (pos < n && _ids[pos] < id) {
        ++pos;
    }
    if (pos < n && _ids[pos] == id) {
        return;
    }
    _ids.push_back(0);
    for (int i = n; i > pos; --i) {
        _ids[i] = _ids[i - 1];
    }
    _ids[pos] = id;
}

void ConjuntoIds::remover(int id) {
    int n = _ids.tamanho();
    for (int i = 0; i < n; ++i) {
        if (_ids[i] == id) {
            for (int j = i; j + 1 < n; ++j) {
                _ids[j] = _ids[j + 1];
            }
            _ids.pop_back();
            return;
        }
    }
}

bool ConjuntoIds::contem(int id) const {
    int lo = 0;
    int hi = _ids.tamanho() - 1;
    while (lo <= hi) {
        g_metricas.comparacoes_conjunto += 1;
        int mid = lo + (hi - lo) / 2;
        if (_ids[mid] == id) {
            return true;
        }
        if (_ids[mid] < id) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return false;
}

int ConjuntoIds::tamanho() const {
    return _ids.tamanho();
}

int ConjuntoIds::operator[](int i) const {
    return _ids[i];
}

const ConjuntoIds& ConjuntoIds::conjunto_vazio() {
    static ConjuntoIds vazio;
    return vazio;
}

ConjuntoIds ConjuntoIds::intersect(const ConjuntoIds& a, const ConjuntoIds& b) {
    ConjuntoIds resultado;
    int i = 0;
    int j = 0;
    // two-pointer sobre conjuntos ja ordenados
    while (i < a.tamanho() && j < b.tamanho()) {
        g_metricas.comparacoes_conjunto += 1;
        if (a[i] < b[j]) {
            ++i;
        } else if (a[i] > b[j]) {
            ++j;
        } else {
            resultado.append(a[i]);
            ++i;
            ++j;
        }
    }
    return resultado;
}

ConjuntoIds ConjuntoIds::uniao(const ConjuntoIds& a, const ConjuntoIds& b) {
    ConjuntoIds resultado;
    int i = 0;
    int j = 0;
    while (i < a.tamanho() && j < b.tamanho()) {
        g_metricas.comparacoes_conjunto += 1;
        if (a[i] < b[j]) {
            resultado.append(a[i++]);
        } else if (a[i] > b[j]) {
            resultado.append(b[j++]);
        } else {
            resultado.append(a[i++]);
            ++j;
        }
    }
    while (i < a.tamanho()) {
        resultado.append(a[i++]);
    }
    while (j < b.tamanho()) {
        resultado.append(b[j++]);
    }
    return resultado;
}

ConjuntoIds ConjuntoIds::complemento(const ConjuntoIds& universo, const ConjuntoIds& a) {
    ConjuntoIds resultado;
    int i = 0;
    int j = 0;
    // universo \ a — ambos ordenados.
    while (i < universo.tamanho()) {
        if (j >= a.tamanho() || universo[i] < a[j]) {
            g_metricas.comparacoes_conjunto += 1;
            resultado.append(universo[i++]);
        } else if (universo[i] > a[j]) {
            g_metricas.comparacoes_conjunto += 1;
            ++j;
        } else {
            g_metricas.comparacoes_conjunto += 1;
            ++i;
            ++j;
        }
    }
    return resultado;
}

long long ConjuntoIds::memoria_bytes() const {
    return _ids.memoria_buffer();
}

ConjuntoIds ConjuntoIds::intersect_multiplo(const Vetor<ConjuntoIds>& conjuntos) {
    if (conjuntos.tamanho() == 0) {
        return ConjuntoIds();
    }
    ConjuntoIds resultado = conjuntos[0];
    for (int i = 1; i < conjuntos.tamanho(); ++i) {
        resultado = intersect(resultado, conjuntos[i]);
    }
    return resultado;
}
