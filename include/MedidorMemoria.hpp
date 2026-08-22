#ifndef MEDIDOR_MEMORIA_HPP
#define MEDIDOR_MEMORIA_HPP

//usado para analise experimental de memoria
#include <iostream>
#include <string>

long long memoria_string(const std::string& s);

long long memoria_chave_int(int);
long long memoria_chave_double(double);
long long memoria_chave_string(const std::string& s);

template<typename K>
long long memoria_chave_mapa(const K&) {
    return 0;
}

inline long long memoria_chave_mapa(const std::string& s) {
    return memoria_chave_string(s);
}

long long rss_kb_atual();

struct RelatorioMemoria {
    long long usuarios;
    long long produtos;
    long long compras;
    long long reposicoes;
    long long indices;
    long long historico;
    long long total_estimado;
    long long rss_kb;

    void reset();
    void imprimir(std::ostream& out) const;
};

#endif
