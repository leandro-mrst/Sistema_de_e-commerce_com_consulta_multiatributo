CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Iinclude

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

ALL_CPP = $(wildcard $(SRC_DIR)/*.cpp)
CORE_CPP = $(filter-out $(SRC_DIR)/HistoricoLista.cpp $(SRC_DIR)/HistoricoMatriz.cpp,$(ALL_CPP))
LISTA_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CORE_CPP)) $(OBJ_DIR)/HistoricoLista.o

TARGET = $(BIN_DIR)/tp3.out
TARGET_HASH = $(BIN_DIR)/tp3_hash.out
TARGET_HIST_MATRIZ = $(BIN_DIR)/tp3_hist_matriz.out
TARGET_HASH_HIST_MATRIZ = $(BIN_DIR)/tp3_hash_hist_matriz.out
TARGET_LOCALIDADE_MATRIZ = $(BIN_DIR)/localidade_demo_matriz.out
TARGET_LOCALIDADE = $(BIN_DIR)/localidade_demo.out

.PHONY: all hash clean bench bench-build roteiro roteiro-rapido localidade localidade-rapido localidade-traco experimentos experimentos-rapido hist-matriz hist-matriz-build historico-bench hash-hist-matriz-build

all: $(TARGET)

hash: clean
	$(MAKE) hash-build

hash-build: CXXFLAGS += -DUSE_HASH
hash-build: $(TARGET_HASH)

hist-matriz: clean
	$(MAKE) hist-matriz-build

hist-matriz-build: CXXFLAGS += -DUSE_HIST_MATRIZ
hist-matriz-build: $(TARGET_HIST_MATRIZ)

hash-hist-matriz-build: CXXFLAGS += -DUSE_HASH -DUSE_HIST_MATRIZ
hash-hist-matriz-build: $(TARGET_HASH_HIST_MATRIZ)

$(TARGET): $(LISTA_OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TARGET_HASH): $(CORE_CPP) $(SRC_DIR)/HistoricoLista.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -DUSE_HASH $(CORE_CPP) $(SRC_DIR)/HistoricoLista.cpp -o $@

$(TARGET_HIST_MATRIZ): $(CORE_CPP) $(SRC_DIR)/HistoricoMatriz.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -DUSE_HIST_MATRIZ $(CORE_CPP) $(SRC_DIR)/HistoricoMatriz.cpp -o $@

$(TARGET_HASH_HIST_MATRIZ): $(CORE_CPP) $(SRC_DIR)/HistoricoMatriz.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -DUSE_HASH -DUSE_HIST_MATRIZ $(CORE_CPP) $(SRC_DIR)/HistoricoMatriz.cpp -o $@

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

historico-bench:
	$(MAKE) all
	$(MAKE) hist-matriz-build
	python3 bench/rodar_historico.py --skip-build
	python3 bench/plot_historico.py

clean:
	rm -f $(BIN_DIR)/*.out $(OBJ_DIR)/*.o

bench:
	python3 bench/rodar_bench.py

bench-build:
	$(MAKE) all
	$(MAKE) hash-build
	python3 bench/rodar_bench.py --skip-build

roteiro-rapido:
	$(MAKE) all
	$(MAKE) hash-build
	python3 bench/roteiro_bench.py --modo rapido --skip-build

roteiro:
	$(MAKE) all
	$(MAKE) hash-build
	python3 bench/roteiro_bench.py --modo completo --repeticoes 3 --skip-build

localidade-rapido:
	$(MAKE) all
	$(MAKE) hash-build
	python3 bench/valgrind_localidade.py --modo rapido --skip-build
	python3 bench/plot_resultados.py --localidade

localidade:
	$(MAKE) all
	$(MAKE) hash-build
	python3 bench/valgrind_localidade.py --modo completo --skip-build
	python3 bench/plot_resultados.py --localidade

localidade-traco:
	$(MAKE) all
	$(MAKE) hash-build
	python3 bench/lackey_localidade.py --skip-build
	python3 bench/plot_traco_localidade.py

experimentos:
	$(MAKE) all
	$(MAKE) hash-build
	python3 bench/rodar_experimentos.py --skip-build
	python3 bench/plot_experimentos.py

experimentos-rapido:
	$(MAKE) all
	$(MAKE) hash-build
	python3 bench/rodar_experimentos.py --skip-build --experimento E01_usuarios
	python3 bench/plot_experimentos.py
