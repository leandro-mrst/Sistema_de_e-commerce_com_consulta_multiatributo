#include "Sistema.hpp"
#include "Metricas.hpp"
#include <iostream>
#include <sstream>
#include <string>

static Vetor<std::string> tokenizar_resto(std::stringstream& ss) {
    Vetor<std::string> tokens;
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

static void ler_pares(std::stringstream& ss, Vetor<int>& ids, Vetor<int>& qtds) {
    int id = 0;
    int qtd = 0;
    while (ss >> id >> qtd) {
        ids.push_back(id);
        qtds.push_back(qtd);
    }
}

int main() {
    metricas_iniciar();
    Sistema sistema;
    std::string linha;

    while (std::getline(std::cin, linha)) {
        if (linha.empty()) {
            continue;
        }

        std::stringstream ss(linha);
        std::string cmd;
        ss >> cmd;

        long long inicio = metricas_marcar_inicio();

        if (cmd == "U") {
            std::string nome;
            std::string cidade;
            std::string estado;
            std::string nacionalidade;
            int idade = 0;
            ss >> nome >> idade >> cidade >> estado >> nacionalidade;
            sistema.cadastrar_usuario(nome, idade, cidade, estado, nacionalidade);
            metricas_registrar(OP_U, inicio);
        } else if (cmd == "P") {
            std::string nome;
            std::string categoria;
            std::string marca;
            std::string condicao;
            double preco = 0.0;
            int qtd = 0;
            ss >> nome >> preco >> qtd >> categoria >> marca >> condicao;
            sistema.cadastrar_produto(nome, preco, qtd, categoria, marca, condicao);
            metricas_registrar(OP_P, inicio);
        } else if (cmd == "R") {
            int timestamp = 0;
            ss >> timestamp;
            Vetor<int> ids;
            Vetor<int> qtds;
            ler_pares(ss, ids, qtds);
            sistema.registrar_reposicao(timestamp, &ids[0], &qtds[0], ids.tamanho());
            metricas_registrar(OP_R, inicio);
        } else if (cmd == "C") {
            int timestamp = 0;
            int id_usuario = 0;
            ss >> timestamp >> id_usuario;
            Vetor<int> ids;
            Vetor<int> qtds;
            ler_pares(ss, ids, qtds);
            sistema.registrar_compra(timestamp, id_usuario, &ids[0], &qtds[0], ids.tamanho());
            metricas_registrar(OP_C, inicio);
        } else if (cmd == "LU") {
            sistema.consultar_usuarios(tokenizar_resto(ss));
            metricas_registrar(OP_LU, inicio);
        } else if (cmd == "LP") {
            sistema.consultar_produtos(tokenizar_resto(ss));
            metricas_registrar(OP_LP, inicio);
        } else if (cmd == "LC") {
            sistema.consultar_compras(tokenizar_resto(ss));
            metricas_registrar(OP_LC, inicio);
        } else if (cmd == "LR") {
            sistema.consultar_reposicoes(tokenizar_resto(ss));
            metricas_registrar(OP_LR, inicio);
        } else if (cmd == "MEM") {
            sistema.imprimir_memoria();
        } else if (cmd == "MET") {
            g_metricas.imprimir(std::cout);
        }
    }

    return 0;
}
