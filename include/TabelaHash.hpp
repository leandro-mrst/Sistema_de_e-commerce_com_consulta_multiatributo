#ifndef TABELA_HASH_HPP
#define TABELA_HASH_HPP

#include <string>
#include "ConjuntoIds.hpp"

struct EntradaHashInt {
    int chave;
    ConjuntoIds ids;// ids das entidades com este valor de atributo
    EntradaHashInt* prox; // proximo no na lista do bucket

    EntradaHashInt(int chave);
};

struct EntradaHashString {
    std::string chave;
    ConjuntoIds ids;
    EntradaHashString* prox;

    EntradaHashString(const std::string& chave);
};

class MapaHashInt {
    private:
    EntradaHashInt** _buckets;
    int _capacidade;// numero de buckets na tabela
    int _tamanho; // numero de chaves distintas armazenadas

    unsigned long hash(int chave) const;
    void redimensionar();
    void destruir();

public:
    MapaHashInt();
    ~MapaHashInt();

    // Associa id ao valor chave, cria entrada se necessario
    void inserir(int chave, int id);
    // Retorna conjunto de ids ou conjunto_vazio() se chave inexistente
    const ConjuntoIds& buscar(int chave) const;
    long long memoria_bytes() const;
};

class MapaHashString {
private:
    EntradaHashString** _buckets;
    int _capacidade;
    int _tamanho;

    unsigned long hash(const std::string& chave) const;
    void redimensionar();
    void destruir();

public:
    MapaHashString();
    ~MapaHashString();

    void inserir(const std::string& chave, int id);
    const ConjuntoIds& buscar(const std::string& chave) const;
    long long memoria_bytes() const;
};

#endif
