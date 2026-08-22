#ifndef PAR_ID_QTD_HPP
#define PAR_ID_QTD_HPP

struct ParIdQtd {
    int id;// id do produto (historico de usuario) ou id do usuario (historico de produto)
    int qtd;// soma das quantidades em todas as compras validas
};

#endif
