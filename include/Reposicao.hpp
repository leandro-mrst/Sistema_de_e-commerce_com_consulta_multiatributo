#ifndef REPOSICAO_HPP
#define REPOSICAO_HPP

//registro de reposicao de estoque
class Reposicao {
    int _id;
    int _timestamp;
    int _n_produtos;
    int* _id_produtos;// ids dos produtos, ordenados crescentemente
    int* _qtd_produtos;// quantidades repostas

    void ordenar_por_id_produto();

public:
    Reposicao(int id, int timestamp, const int* ids, const int* qtds, int n);
    Reposicao(const Reposicao& outro);
    Reposicao& operator=(const Reposicao& outro);
    ~Reposicao();

    int get_id() const;
    int get_timestamp() const;
    bool contem_produto(int id_produto) const;
    void imprimir_consulta(int indice_resultado) const;
    
    long long memoria_bytes() const;
};

#endif
