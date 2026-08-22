#ifndef HISTORICO_MATRIZ_HPP
#define HISTORICO_MATRIZ_HPP

//matriz densa usuario x produto
#include "Vetor.hpp"

class HistoricoMatriz {
private:
    Vetor<Vetor<int>> _usuario_produto; // [u][p] = qtd total comprada
    Vetor<Vetor<int>> _produto_usuario;//[p][u] = qtd total comprada

    static void expandir_linha(Vetor<int>& linha, int novo_tamanho);
    static void somar_celula(Vetor<int>& linha, int indice, int qtd);

public:
    void ao_cadastrar_usuario(int id);
    void ao_cadastrar_produto(int id);

    void registrar_compra(int id_usuario, 
        const int* id_produtos, const int* qtds, int n);

    void imprimir_produtos_usuario(int id_usuario) const;
    void imprimir_usuarios_produto(int id_produto) const;
    long long memoria_bytes() const;
};

#endif 
