#ifndef METRICAS_HPP
#define METRICAS_HPP

//instrumentacao para analise experimental
#include <iostream>

enum TipoOperacao {
    OP_U = 0,
    OP_P,
    OP_C,
    OP_R,
    OP_LU,
    OP_LP,
    OP_LC,
    OP_LR,
    OP_TOTAL
};

struct MetricasOperacao {
    long long contagem;
    long long tempo_ns;

    MetricasOperacao();
    void reset();
};

struct MetricasLocalidade {
    long long leituras_vetor_ids;
    long long leituras_vetor_ids_seq;
    long long nos_indice;
    long long acessos_entidade;
    long long acessos_entidade_seq;

    void reset();
};

struct Metricas {
    MetricasOperacao operacoes[OP_TOTAL];
    long long comparacoes_conjunto;
    long long insercoes_indice;
    MetricasLocalidade localidade;

    void reset();
    void imprimir(std::ostream& out) const;
};

extern Metricas g_metricas;

void metricas_iniciar();
long long metricas_marcar_inicio();
void metricas_registrar(TipoOperacao op, long long inicio_ns);
void metricas_registrar_insercao_indice();

void metricas_registrar_leituras_vetor(int i, int j);
void metricas_registrar_no_indice();
void metricas_registrar_acesso_entidade(int id, int id_anterior);

const char* metricas_nome_operacao(TipoOperacao op);

#endif // METRICAS_HPP
