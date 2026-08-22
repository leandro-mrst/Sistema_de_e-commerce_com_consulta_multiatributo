# Sobre o Projeto

Este repositório contém a implementação do Trabalho Prático 3 (TP3) da disciplina DCC221 - Estruturas de Dados (DCC/ICEx/UFMG, 2026/1). O objetivo principal é desenvolver um sistema simplificado de e-commerce capaz de realizar buscas rápidas através da implementação de índices invertidos.

O sistema mantém o registro dinâmico e o histórico de usuários, produtos, compras e reposições de estoque. Para respeitar as restrições didáticas, o projeto foi desenvolvido integralmente em C++11, não utilizando contêineres e estruturas pré-implementadas da biblioteca padrão, como `std::vector`, `std::list` ou `std::map`.

## Estrutura do Repositório

O projeto segue a estrutura rígida de diretórios exigida para a compilação e avaliação:

*   `src/`: Contém os arquivos de código-fonte (`.cpp`).
*   `include/`: Armazena os cabeçalhos das estruturas de dados e classes (`.h`).
*   `bin/`: Diretório de destino para os executáveis gerados, em especial o `tp3.out`.
*   `obj/`: Destinado aos arquivos objeto (`.o`) gerados durante a compilação.
*   `bench/`: Contém os scripts em Python desenvolvidos para realizar a análise experimental de desempenho, plotagem de gráficos e análise de localidade.

## Compilação e Execução

O sistema possui um `Makefile` configurado para compilar diferentes versões das estruturas de dados (via macros) e facilitar a execução de testes experimentais exigidos na documentação.

*   **`make all`**: Compila a versão padrão do sistema, gerando o executável principal `bin/tp3.out`.
*   **`make hash` / `make hist-matriz`**: Compila versões alternativas do sistema utilizando macros de pré-processamento (`-DUSE_HASH`, `-DUSE_HIST_MATRIZ`) para variar a abordagem estrutural.
*   **`make bench` / `make roteiro`**: Invoca os scripts automatizados de Python para rodar e testar os variados perfis de carga.
*   **`make localidade`**: Executa a análise de localidade de referência com a ferramenta Valgrind para avaliar o acesso à memória.
*   **`make clean`**: Remove todos os arquivos objeto e binários do ambiente.

## Funcionalidades e Consultas

O e-commerce lê dados da entrada padrão e opera através de identificadores únicos (IDs) atribuídos sequencialmente a partir de zero. A tabela abaixo resume as operações:

| Entidade | Cadastro / Ação | Consulta Multiatributo |
| :--- | :--- | :--- |
| **Usuários** | Comando `U` | Comando `LU` |
| **Produtos** | Comando `P` | Comando `LP` |
| **Compras** | Comando `C` (Valida estoque) | Comando `LC` |
| **Reposições** | Comando `R` (Aumenta estoque) | Comando `LR` |
