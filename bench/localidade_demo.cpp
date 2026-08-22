/*
 * Carga isolada para traço Lackey — localidade espacial e temporal.
 *
 * Exercita Vetor<int>, ConjuntoIds e MapaVetor com padrões de acesso
 * contíguos (espacial) e repetidos (temporal), sem ruído de I/O ou stack.
 */
#include "ConjuntoIds.hpp"
#include "MapaVetor.hpp"
#include "Vetor.hpp"

static void fase_interseccao() {
    ConjuntoIds a, b;
    const int n = 4000;
    for (int i = 0; i < n; ++i) {
        a.inserir(i);
        b.inserir((i * 3) % (n + n / 4));
    }
    for (int rep = 0; rep < 80; ++rep) {
        ConjuntoIds c = ConjuntoIds::intersect(a, b);
        (void)c.tamanho();
    }
}

static void fase_varredura_vetor() {
    Vetor<int> dados;
    const int n = 8000;
    for (int i = 0; i < n; ++i) {
        dados.push_back(i);
    }
    volatile int soma = 0;
    for (int rep = 0; rep < 120; ++rep) {
        for (int i = 0; i < dados.tamanho(); ++i) {
            soma += dados[i];
        }
    }
    (void)soma;
}

static void fase_mapa_vetor() {
    MapaVetorInt mapa;
    const int n = 2000;
    for (int i = 0; i < n; ++i) {
        mapa.inserir(i % 400, i);
    }
    for (int rep = 0; rep < 200; ++rep) {
        for (int chave = 0; chave < 400; ++chave) {
            const ConjuntoIds& ids = mapa.buscar(chave);
            (void)ids.tamanho();
        }
    }
}

static void fase_temporal() {
    volatile int acc = 0;
    for (int rep = 0; rep < 50000; ++rep) {
        acc += rep & 7;
    }
    (void)acc;
}

int main() {
    fase_interseccao();
    fase_varredura_vetor();
    fase_mapa_vetor();
    fase_temporal();
    return 0;
}
