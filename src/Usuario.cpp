#include "Usuario.hpp"
#include "MedidorMemoria.hpp"
#include <iostream>

Usuario::Usuario(int id, int idade, const std::string& nome, 
        const std::string& cidade, const std::string& estado,
        const std::string& nacionalidade): _id(id), _idade(idade),_nome(nome), _cidade(cidade),
        _estado(estado), _nacionalidade(nacionalidade){}


    int Usuario::get_id() const{
        return _id;
    }
    std::string Usuario::get_nome() const{
        return _nome;
    }
std::string Usuario::get_cidade() const{
    return _cidade;
    }
std::string Usuario::get_estado() const{
    return _estado;
    }
std::string Usuario::get_nacionalidade() const{
    return _nacionalidade;
    }
void Usuario::imprimir_consulta(int indice_resultado) const{
    std::cout << "LU resultado_" << indice_resultado << " usuario " << _id << " " <<
    _nome << " " << _idade << " " << _cidade << " " << _estado << " " <<
    _nacionalidade << '\n';
}

long long Usuario::memoria_bytes() const {
    return memoria_string(_nome) + memoria_string(_cidade) +
           memoria_string(_estado) + memoria_string(_nacionalidade);
}
