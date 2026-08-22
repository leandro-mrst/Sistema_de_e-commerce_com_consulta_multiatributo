#ifndef HISTORICO_LISTA_HPP
#define HISTORICO_LISTA_HPP

//Historico lista com listas esparsas ordenadas de ParIDQtd por entidade
#include "Vetor.hpp"
#include "ParIdQtd.hpp"

class HistoricoLista {
private:
    Vetor<Vetor<ParIdQtd> > _por_usuario;
    Vetor<Vetor<ParIdQtd> > _por_produto;

    static void inserir_ou_somar(Vetor<ParIdQtd>& lista, int id, int qtd);

public:
    void ao_cadastrar_usuario(int id);
    void ao_cadastrar_produto(int id);

    void registrar_compra(int id_usuario, const int* id_produtos, const int* qtds, int n);

    void imprimir_produtos_usuario(int id_usuario) const;
    void imprimir_usuarios_produto(int id_produto) const;
    long long memoria_bytes() const;
};

#endif // HISTORICO_LISTA_HPP
