#include "Metricas.hpp"

#include <chrono>

Metricas g_metricas;

MetricasOperacao::MetricasOperacao() : contagem(0), tempo_ns(0) {}

void MetricasOperacao::reset() {
    contagem = 0;
    tempo_ns = 0;
}

void Metricas::reset() {
    for (int i = 0; i < OP_TOTAL; ++i) {
        operacoes[i].reset();
    }
    comparacoes_conjunto = 0;
    insercoes_indice = 0;
}

const char* metricas_nome_operacao(TipoOperacao op) {
    switch (op) {
        case OP_U: return "U";
        case OP_P: return "P";
        case OP_C: return "C";
        case OP_R: return "R";
        case OP_LU: return "LU";
        case OP_LP: return "LP";
        case OP_LC: return "LC";
        case OP_LR: return "LR";
        default: return "?";
    }
}

void metricas_iniciar() {
    g_metricas.reset();
}

long long metricas_marcar_inicio() {
    std::chrono::high_resolution_clock::time_point agora =
        std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        agora.time_since_epoch()).count();
}

void metricas_registrar(TipoOperacao op, long long inicio_ns) {
    if (op < 0 || op >= OP_TOTAL) {
        return;
    }
    long long fim_ns = metricas_marcar_inicio();
    g_metricas.operacoes[op].contagem += 1;
    g_metricas.operacoes[op].tempo_ns += (fim_ns - inicio_ns);
}

void metricas_registrar_insercao_indice() {
    g_metricas.insercoes_indice += 1;
}

void Metricas::imprimir(std::ostream& out) const {
    for (int i = 0; i < OP_TOTAL; ++i) {
        const MetricasOperacao& m = operacoes[i];
        out << "MET op " << metricas_nome_operacao(static_cast<TipoOperacao>(i))
            << " count " << m.contagem
            << " tempo_ns_total " << m.tempo_ns;
        if (m.contagem > 0) {
            out << " tempo_ns_medio " << (m.tempo_ns / m.contagem);
        } else {
            out << " tempo_ns_medio 0";
        }
        out << '\n';
    }
    out << "MET comparacoes_conjunto " << comparacoes_conjunto << '\n';
    out << "MET insercoes_indice " << insercoes_indice << '\n';
}
