/*
 * Demo Lackey — HistoricoMatriz: varredura contigua de linhas na impressao LU/LP.
 */
#include "HistoricoMatriz.hpp"
#include <iostream>

int main() {
    HistoricoMatriz hist;
    const int u = 80;
    const int p = 80;
    for (int i = 0; i < u; ++i) {
        hist.ao_cadastrar_usuario(i);
    }
    for (int i = 0; i < p; ++i) {
        hist.ao_cadastrar_produto(i);
    }

    int produtos[3] = {0, 1, 2};
    int qtds[3] = {1, 2, 1};
    for (int rep = 0; rep < 30; ++rep) {
        for (int uid = 0; uid < u; ++uid) {
            hist.registrar_compra(uid, produtos, qtds, 3);
        }
    }

    for (int rep = 0; rep < 200; ++rep) {
        for (int uid = 0; uid < u; ++uid) {
            hist.imprimir_produtos_usuario(uid);
        }
        for (int pid = 0; pid < p; ++pid) {
            hist.imprimir_usuarios_produto(pid);
        }
    }
    return 0;
}
