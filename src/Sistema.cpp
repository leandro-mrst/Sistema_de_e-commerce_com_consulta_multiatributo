#include "Sistema.hpp"
#include "ConsultaEvaluator.hpp"
#include "MedidorMemoria.hpp"
#include <iostream>
#include <sstream>

static int parse_int(const std::string& s) {
    std::stringstream ss(s);
    int v = 0;
    ss >> v;
    return v;
}

static double parse_double(const std::string& s) {
    std::stringstream ss(s);
    double v = 0.0;
    ss >> v;
    return v;
}

Sistema::Sistema()
    : _prox_id_usuario(0), _prox_id_produto(0),
      _prox_id_compra(0), _prox_id_reposicao(0) {
}

void Sistema::cadastrar_usuario(const std::string& nome, int idade,
        const std::string& cidade, const std::string& estado,
        const std::string& nacionalidade) {
    int id = _prox_id_usuario++;
    _usuarios.push_back(Usuario(id, idade, nome, cidade, estado, nacionalidade));
    _historico.ao_cadastrar_usuario(id);

    _idx_u_nome.inserir(nome, id);
    _idx_u_cidade.inserir(cidade, id);
    _idx_u_estado.inserir(estado, id);
    _idx_u_nacionalidade.inserir(nacionalidade, id);
    _idx_u_id.inserir(id, id);
    _idx_u_idade.inserir(idade, id);
    _idx_u_idade_faixa.inserir(idade, id);

    std::cout << "U " << id << '\n';
}

void Sistema::cadastrar_produto(const std::string& nome, double preco, int qtd,
        const std::string& categoria, const std::string& marca,
        const std::string& condicao) {
    int id = _prox_id_produto++;
    _produtos.push_back(Produto(id, nome, preco, qtd, categoria, marca, condicao));
    _historico.ao_cadastrar_produto(id);

    _idx_p_nome.inserir(nome, id);
    _idx_p_categoria.inserir(categoria, id);
    _idx_p_marca.inserir(marca, id);
    _idx_p_condicao.inserir(condicao, id);
    _idx_p_id.inserir(id, id);
    _idx_p_preco.inserir(preco, id);
    _idx_p_qtd.inserir(qtd, id);

    std::cout << "P " << id << '\n';
}

void Sistema::atualizar_indice_qtd_produto(int id_produto, int qtd_antiga, int qtd_nova) {
    // qtd muda em C/R: remove id do valor antigo e insere no valor novo.
    _idx_p_qtd.remover(qtd_antiga, id_produto);
    _idx_p_qtd.inserir(qtd_nova, id_produto);
}

void Sistema::registrar_reposicao(int timestamp, const int* ids, const int* qtds, int n) {
    int id = _prox_id_reposicao++;
    _reposicoes.push_back(Reposicao(id, timestamp, ids, qtds, n));

    for (int i = 0; i < n; ++i) {
        int id_prod = ids[i];
        int qtd_antiga = _produtos[id_prod].get_qtd();
        _produtos[id_prod].incrementar(qtds[i]);
        atualizar_indice_qtd_produto(id_prod, qtd_antiga, _produtos[id_prod].get_qtd());
    }

    _idx_r_id.inserir(id, id);
    _idx_r_timestamp.inserir(timestamp, id);
    _idx_r_timestamp_faixa.inserir(timestamp, id);
    for (int i = 0; i < n; ++i) {
        _idx_r_produto.inserir(ids[i], id);
    }

    std::cout << "R " << id << '\n';
}

void Sistema::registrar_compra(int timestamp, int id_usuario,
        const int* ids, const int* qtds, int n) {
    // Fase 1: validar estoque de todos os produtos antes de alterar qualquer estado.
    for (int i = 0; i < n; ++i) {
        if (!_produtos[ids[i]].tem_estoque(qtds[i])) {
            std::cout << "C INV\n";
            return;  //compra invalida: nenhuma alteracao no sistema
        }
    }

    int id = _prox_id_compra++;
    _compras.push_back(Compra(id, timestamp, id_usuario, ids, qtds, n));

    for (int i = 0; i < n; ++i) {
        int id_prod = ids[i];
        int qtd_antiga = _produtos[id_prod].get_qtd();
        _produtos[id_prod].decrementar(qtds[i]);
        atualizar_indice_qtd_produto(id_prod, qtd_antiga, _produtos[id_prod].get_qtd());
    }

    _historico.registrar_compra(id_usuario, ids, qtds, n);

    _idx_c_id.inserir(id, id);
    _idx_c_timestamp.inserir(timestamp, id);
    _idx_c_timestamp_faixa.inserir(timestamp, id);
    _idx_c_usuario.inserir(id_usuario, id);
    for (int i = 0; i < n; ++i) {
        _idx_c_produto.inserir(ids[i], id);
    }

    std::cout << "C " << id << '\n';
}

ConjuntoIds Sistema::filtro_usuario_exato(const std::string& attr, const std::string& valor) const {
    if (attr == "id") {
        return _idx_u_id.buscar(parse_int(valor));
    }
    if (attr == "nome") {
        return _idx_u_nome.buscar(valor);
    }
    if (attr == "idade") {
        return _idx_u_idade.buscar(parse_int(valor));
    }
    if (attr == "cidade") {
        return _idx_u_cidade.buscar(valor);
    }
    if (attr == "estado") {
        return _idx_u_estado.buscar(valor);
    }
    if (attr == "nacionalidade") {
        return _idx_u_nacionalidade.buscar(valor);
    }
    return ConjuntoIds::conjunto_vazio();
}

ConjuntoIds Sistema::filtro_produto_exato(const std::string& attr, const std::string& valor) const {
    if (attr == "id") {
        return _idx_p_id.buscar(parse_int(valor));
    }
    if (attr == "nome") {
        return _idx_p_nome.buscar(valor);
    }
    if (attr == "categoria") {
        return _idx_p_categoria.buscar(valor);
    }
    if (attr == "marca") {
        return _idx_p_marca.buscar(valor);
    }
    if (attr == "condicao") {
        return _idx_p_condicao.buscar(valor);
    }
    if (attr == "preco") {
        return _idx_p_preco.buscar(parse_double(valor));
    }
    if (attr == "qtd") {
        return _idx_p_qtd.buscar(parse_int(valor));
    }
    return ConjuntoIds::conjunto_vazio();
}

ConjuntoIds Sistema::filtro_compra_exato(const std::string& attr, const std::string& valor) const {
    if (attr == "id") {
        return _idx_c_id.buscar(parse_int(valor));
    }
    if (attr == "timestamp") {
        return _idx_c_timestamp.buscar(parse_int(valor));
    }
    if (attr == "id_usuario") {
        return _idx_c_usuario.buscar(parse_int(valor));
    }
    if (attr == "id_produto") {
        return _idx_c_produto.buscar(parse_int(valor));
    }
    return ConjuntoIds::conjunto_vazio();
}

ConjuntoIds Sistema::filtro_reposicao_exato(const std::string& attr, const std::string& valor) const {
    if (attr == "id") {
        return _idx_r_id.buscar(parse_int(valor));
    }
    if (attr == "timestamp") {
        return _idx_r_timestamp.buscar(parse_int(valor));
    }
    if (attr == "id_produto") {
        return _idx_r_produto.buscar(parse_int(valor));
    }
    return ConjuntoIds::conjunto_vazio();
}

ConjuntoIds Sistema::avaliar_filtro(const std::string& cmd,
        const FiltroAtomico& filtro) const {
    if (filtro.faixa) {
        if (cmd == "LU" && filtro.atributo == "idade") {
            return _idx_u_idade_faixa.buscar_faixa(parse_int(filtro.v1), parse_int(filtro.v2));
        }
        if (cmd == "LP" && filtro.atributo == "preco") {
            return _idx_p_preco.buscar_faixa(parse_double(filtro.v1),parse_double(filtro.v2));
        }
        if (cmd == "LP" && filtro.atributo == "qtd") {
            return _idx_p_qtd.buscar_faixa(parse_int(filtro.v1), parse_int(filtro.v2));
        }
        if (cmd == "LC" && filtro.atributo == "timestamp") {
            return _idx_c_timestamp_faixa.buscar_faixa(parse_int(filtro.v1), parse_int(filtro.v2));
        }
        if (cmd == "LR" && filtro.atributo == "timestamp") {
            return _idx_r_timestamp_faixa.buscar_faixa(parse_int(filtro.v1),parse_int(filtro.v2));
        }
        return ConjuntoIds::conjunto_vazio();
    }

    if (cmd == "LU") {
        return filtro_usuario_exato(filtro.atributo, filtro.v1);
    }
    if (cmd == "LP") {
        return filtro_produto_exato(filtro.atributo, filtro.v1);
    }
    if (cmd == "LC") {
        return filtro_compra_exato(filtro.atributo, filtro.v1);
    }
    if (cmd == "LR") {
        return filtro_reposicao_exato(filtro.atributo, filtro.v1);
    }
    return ConjuntoIds::conjunto_vazio();
}

ConjuntoIds Sistema::universo_da_consulta(const std::string& cmd) const {
    // Conjunto de ids cadastrados,  base para NOT
    ConjuntoIds u;
    if (cmd == "LU") {
        for (int i = 0; i < _prox_id_usuario; ++i) {
            u.inserir(i);
        }
    } else if (cmd == "LP") {
        for (int i = 0; i < _prox_id_produto; ++i) {
            u.inserir(i);
        }
    } else if (cmd == "LC") {
        for (int i = 0; i < _prox_id_compra; ++i) {
            u.inserir(i);
        }
    } else if (cmd == "LR") {
        for (int i = 0; i < _prox_id_reposicao; ++i) {
            u.inserir(i);
        }
    }
    return u;
}

void Sistema::consultar_usuarios(const Vetor<std::string>& tokens) {
    ConsultaEvaluator eval(this);
    ConjuntoIds resultado = eval.avaliar("LU", tokens);

    if (resultado.tamanho() == 0) {
        std::cout << "LU VAZIO\n";
        return;
    }

    for (int i = 0; i < resultado.tamanho(); ++i) {
        int id = resultado[i];
        _usuarios[id].imprimir_consulta(i + 1);
        _historico.imprimir_produtos_usuario(id);
    }
}

void Sistema::consultar_produtos(const Vetor<std::string>& tokens) {
    ConsultaEvaluator eval(this);
    ConjuntoIds resultado = eval.avaliar("LP", tokens);

    if (resultado.tamanho() == 0) {
        std::cout << "LP VAZIO\n";
        return;
    }

    for (int i = 0; i < resultado.tamanho(); ++i) {
        int id = resultado[i];
        _produtos[id].imprimir_consulta(i + 1);
        _historico.imprimir_usuarios_produto(id);
    }
}

void Sistema::consultar_compras(const Vetor<std::string>& tokens) {
    ConsultaEvaluator eval(this);
    ConjuntoIds resultado = eval.avaliar("LC", tokens);

    if (resultado.tamanho() == 0) {
        std::cout << "LC VAZIO\n";
        return;
    }

    for (int i = 0; i < resultado.tamanho(); ++i) {
        _compras[resultado[i]].imprimir_consulta(i + 1);
    }
}

void Sistema::consultar_reposicoes(const Vetor<std::string>& tokens) {
    ConsultaEvaluator eval(this);
    ConjuntoIds resultado = eval.avaliar("LR", tokens);

    if (resultado.tamanho() == 0) {
        std::cout << "LR VAZIO\n";
        return;
    }

    for (int i = 0; i < resultado.tamanho(); ++i) {
        _reposicoes[resultado[i]].imprimir_consulta(i + 1);
    }
}

static long long memoria_vetor_usuarios(const Vetor<Usuario>& vetor) {
    long long total = vetor.memoria_buffer();
    for (int i = 0; i < vetor.tamanho(); ++i) {
        total += vetor[i].memoria_bytes();
    }
    return total;
}

static long long memoria_vetor_produtos(const Vetor<Produto>& vetor) {
    long long total = vetor.memoria_buffer();
    for (int i = 0; i < vetor.tamanho(); ++i) {
        total += vetor[i].memoria_bytes();
    }
    return total;
}

static long long memoria_vetor_compras(const Vetor<Compra>& vetor) {
    long long total = vetor.memoria_buffer();
    for (int i = 0; i < vetor.tamanho(); ++i) {
        total += vetor[i].memoria_bytes();
    }
    return total;
}

static long long memoria_vetor_reposicoes(const Vetor<Reposicao>& vetor) {
    long long total = vetor.memoria_buffer();
    for (int i = 0; i < vetor.tamanho(); ++i) {
        total += vetor[i].memoria_bytes();
    }
    return total;
}

RelatorioMemoria Sistema::medir_memoria() const {
    RelatorioMemoria rel;
    rel.reset();

    rel.usuarios = memoria_vetor_usuarios(_usuarios);
    rel.produtos = memoria_vetor_produtos(_produtos);
    rel.compras = memoria_vetor_compras(_compras);
    rel.reposicoes = memoria_vetor_reposicoes(_reposicoes);

    rel.indices =
        _idx_u_nome.memoria_bytes() + _idx_u_cidade.memoria_bytes() +
        _idx_u_estado.memoria_bytes() + _idx_u_nacionalidade.memoria_bytes() +
        _idx_u_id.memoria_bytes() + _idx_u_idade.memoria_bytes() +
        _idx_u_idade_faixa.memoria_bytes() +
        _idx_p_nome.memoria_bytes() + _idx_p_categoria.memoria_bytes() +
        _idx_p_marca.memoria_bytes() + _idx_p_condicao.memoria_bytes() +
        _idx_p_id.memoria_bytes() + _idx_p_preco.memoria_bytes() +
        _idx_p_qtd.memoria_bytes() +
        _idx_c_id.memoria_bytes() + _idx_c_timestamp.memoria_bytes() +
        _idx_c_usuario.memoria_bytes() + _idx_c_produto.memoria_bytes() +
        _idx_c_timestamp_faixa.memoria_bytes() +
        _idx_r_id.memoria_bytes() + _idx_r_timestamp.memoria_bytes() +
        _idx_r_produto.memoria_bytes() + _idx_r_timestamp_faixa.memoria_bytes();

    rel.historico = _historico.memoria_bytes();
    rel.total_estimado = rel.usuarios + rel.produtos + rel.compras +
                         rel.reposicoes + rel.indices + rel.historico;
    rel.rss_kb = rss_kb_atual();
    return rel;
}

void Sistema::imprimir_memoria() const {
    medir_memoria().imprimir(std::cout);
}
