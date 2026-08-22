#ifndef USUARIO_HPP
#define USUARIO_HPP
#include <string>

class Usuario {
private:
    int _id;
    int _idade;
    std::string _nome;
    std::string _cidade;
    std::string _estado;
    std::string _nacionalidade;

public:
    Usuario(int id, int idade, const std::string& nome, const std::string& cidade,
         const std::string& estado, const std::string& nacionalidade);

    int get_id() const;
    std::string get_nome() const;
    std::string get_cidade() const;
    std::string get_estado() const;
    std::string get_nacionalidade() const;
    void imprimir_consulta(int indice_resultado) const;
    
    long long memoria_bytes() const;
};

#endif // USUARIO_HPP
