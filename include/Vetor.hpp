#ifndef VETOR_HPP
#define VETOR_HPP

//classe vetor com templates
template<typename T>
class Vetor {
    public:
    static const int CAPACIDADE_INICIAL = 8;

    private:
    T* _dados;
    int _capacidade;
    int _tamanho;

    void destruir_elementos();
    void expandir();
    void alocar_buffer_inicial();

public:
    Vetor();
    ~Vetor();
    Vetor(const Vetor& outro);
    Vetor& operator=(const Vetor& outro);

    void reservar(int n);
    void push_back(const T& item);
    void pop_back();

    T& operator[](int i);
    const T& operator[](int i) const;

    int tamanho() const;
    int capacidade() const;
    long long memoria_buffer() const;
};

#include "Vetor.tpp"

#endif 
