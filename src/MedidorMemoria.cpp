#include "MedidorMemoria.hpp"

#include <cstdio>
#include <sys/resource.h>
#include <unistd.h>

long long memoria_string(const std::string& s) {
    return static_cast<long long>(sizeof(std::string)) + static_cast<long long>(s.capacity());
}

long long memoria_chave_int(int) {
    return 0;
}

long long memoria_chave_double(double) {
    return 0;
}

long long memoria_chave_string(const std::string& s) {
    return memoria_string(s);
}

long long rss_kb_atual() {
    long long paginas = 0;
    FILE* f = std::fopen("/proc/self/statm", "r");
    if (f) {
        long long total = 0;
        if (std::fscanf(f, "%lld %lld", &total, &paginas) != 2) {
            paginas = 0;
        }
        std::fclose(f);
    }
    if (paginas > 0) {
        return paginas * static_cast<long long>(sysconf(_SC_PAGESIZE)) / 1024;
    }

    struct rusage uso;
    if (getrusage(RUSAGE_SELF, &uso) == 0) {
        return static_cast<long long>(uso.ru_maxrss);
    }
    return 0;
}

void RelatorioMemoria::reset() {
    usuarios = 0;
    produtos = 0;
    compras = 0;
    reposicoes = 0;
    indices = 0;
    historico = 0;
    total_estimado = 0;
    rss_kb = 0;
}

void RelatorioMemoria::imprimir(std::ostream& out) const {
#ifdef USE_HASH
    const char* indice = "hash";
#else
    const char* indice = "vetor";
#endif
#ifdef USE_HIST_MATRIZ
    const char* historico_tipo = "matriz";
#else
    const char* historico_tipo = "lista";
#endif
    out << "MEM variante " << indice << '\n';
    out << "MEM indice " << indice << '\n';
    out << "MEM historico_tipo " << historico_tipo << '\n';
    out << "MEM usuarios " << usuarios << '\n';
    out << "MEM produtos " << produtos << '\n';
    out << "MEM compras " << compras << '\n';
    out << "MEM reposicoes " << reposicoes << '\n';
    out << "MEM indices " << indices << '\n';
    out << "MEM historico " << historico << '\n';
    out << "MEM total_estimado " << total_estimado << '\n';
    out << "MEM rss_kb " << rss_kb << '\n';
}
