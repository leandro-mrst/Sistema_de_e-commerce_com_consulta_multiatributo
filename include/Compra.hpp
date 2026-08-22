#ifndef COMPRA_HPP
#define COMPRA_HPP

//Classe para registrar transacao de compra valida, implementando rule of three
//só é instanciada quando ha estoque suficiente para todos os itens
// Consultas LC filtram por id, timestamp, id_usuario ou id_produto.
class Compra {
private:
    int _id;
    int _timestamp;
    int _id_usuario;
    int _n_produtos;
    int* _id_produtos;// ids dos produtos ordenados crescentemente
    int* _qtd_produtos; // quantidades correspondentes

    void ordenar_por_id_produto();

public:
    Compra(int id, int timestamp, int id_usuario,
           const int* ids, const int* qtds, int n);
    Compra(const Compra& outro);
    Compra& operator=(const Compra& outro);
    ~Compra();

    int get_id() const;
    int get_timestamp() const;
    int get_id_usuario() const;
    bool contem_produto(int id_produto) const;

    void imprimir_consulta(int indice_resultado) const;
    long long memoria_bytes() const;
};

#endif // COMPRA_HPP
