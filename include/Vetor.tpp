#ifndef VETOR_TPP
#define VETOR_TPP

#include <new>

template<typename T>
void Vetor<T>::destruir_elementos() {
    for (int i = 0; i < _tamanho; ++i) {
        _dados[i].~T();
    }
}

template<typename T>
void Vetor<T>::expandir() {
    int nova = (_capacidade == 0) ? CAPACIDADE_INICIAL : _capacidade * 2;
    reservar(nova);
}

template<typename T>
void Vetor<T>::alocar_buffer_inicial() {
    _dados = static_cast<T*>(::operator new(CAPACIDADE_INICIAL * sizeof(T)));
    _capacidade = CAPACIDADE_INICIAL;
}

template<typename T>
Vetor<T>::Vetor() : _dados(0), _capacidade(0), _tamanho(0) {
    alocar_buffer_inicial();
}

template<typename T>
Vetor<T>::~Vetor() {
    destruir_elementos();
    ::operator delete(_dados);
}

template<typename T>
Vetor<T>::Vetor(const Vetor& outro) : _dados(0), _capacidade(0), _tamanho(0) {
    if (outro._tamanho == 0) {
        alocar_buffer_inicial();
        return;
    }
    _dados = static_cast<T*>(::operator new(outro._tamanho * sizeof(T)));
    _capacidade = outro._tamanho;
    for (int i = 0; i < outro._tamanho; ++i) {
        new (&_dados[i]) T(outro._dados[i]);
    }
    _tamanho = outro._tamanho;
}

template<typename T>
Vetor<T>& Vetor<T>::operator=(const Vetor& outro) {
    if (this == &outro) {
        return *this;
    }
    destruir_elementos();
    ::operator delete(_dados);
    _dados = 0;
    _capacidade = 0;
    _tamanho = 0;
    if (outro._tamanho > 0) {
        _dados = static_cast<T*>(::operator new(outro._tamanho * sizeof(T)));
        _capacidade = outro._tamanho;
        for (int i = 0; i < outro._tamanho; ++i) {
            new (&_dados[i]) T(outro._dados[i]);
        }
        _tamanho = outro._tamanho;
    } else {
        alocar_buffer_inicial();
    }
    return *this;
}

template<typename T>
void Vetor<T>::reservar(int n) {
    if (n <= _capacidade) {
        return;
    }
    // Placement new, copia objetos ja construidos para novo bloco de memoria
    T* novo = static_cast<T*>(::operator new(n * sizeof(T)));
    for (int i = 0; i < _tamanho; ++i) {
        new (&novo[i]) T(_dados[i]);
    }
    destruir_elementos();
    ::operator delete(_dados);
    _dados = novo;
    _capacidade = n;
}

template<typename T>
void Vetor<T>::push_back(const T& item) {
    if (_tamanho >= _capacidade) {
        expandir();
    }
    // para permitir T sem construtor padrao, no caso de usuario e produto
    new (&_dados[_tamanho]) T(item);
    ++_tamanho;
}

template<typename T>
void Vetor<T>::pop_back() {
    if (_tamanho > 0) {
        --_tamanho;
        _dados[_tamanho].~T();
    }
}

template<typename T>
T& Vetor<T>::operator[](int i) {
    return _dados[i];
}

template<typename T>
const T& Vetor<T>::operator[](int i) const {
    return _dados[i];
}

template<typename T>
int Vetor<T>::tamanho() const {
    return _tamanho;
}

template<typename T>
int Vetor<T>::capacidade() const {
    return _capacidade;
}

template<typename T>
long long Vetor<T>::memoria_buffer() const {
    return static_cast<long long>(_capacidade) * static_cast<long long>(sizeof(T));
}

#endif
