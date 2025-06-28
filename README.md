# Trabalho Prático - Grafos - Etapa 1, 2 e 3

Este repositório contém os programas desenvolvidos para a disciplina **GCC262 - Grafos e suas Aplicações** do curso de Sistemas de Informação. O projeto é dividido em três etapas complementares para analisar, visualizar e resolver problemas de roteamento em grafos.

  - **Etapa 1**: Um programa em C++ que realiza o processamento de grafos, extraindo métricas estruturais.
  - **Etapa 2**: Um programa em C++ que aplica uma heurística construtiva simples para resolver problemas de roteamento.
  - **Etapa 3 (Resolvedor Avançado)**: Evolução da Etapa 2, este programa implementa uma **meta-heurística híbrida de alto desempenho** para encontrar soluções de alta qualidade para o problema de roteamento.
  - **Visualiza\_Grafos**: Um notebook em Python (Jupyter) que desenha visualmente os grafos.

## Estrutura do Projeto

  - **`output/`**: Pasta contendo os arquivos de instância `.dat` para teste.
  - **`solucoes/`**: Pasta criada automaticamente pelo programa da Etapa 3, onde as soluções (`sol-*.txt`) são salvas.
  - **`TP_Grafos_Etapa1.cpp`**: Código-fonte do programa de análise estrutural.
  - **`TP_Grafos_Etapa3.cpp`**: Código-fonte do resolvedor de roteamento avançado.
  - **`Visualiza_Grafos.ipynb`**: Notebook Jupyter para a visualização gráfica.
  - **`README.md`**: Este arquivo.

-----

## TP\_Grafos\_Etapa1 (Análise de Grafos)

### Arquivo principal:

`TP_Grafos_Etapa1.cpp`

### Descrição:

Este programa lê instâncias de grafos e calcula diversas métricas estruturais para análise.

### Requisitos:

  - Compilador C++ compatível (g++, Clang, MSVC).

### Como executar:

1.  **Atenção**: É necessário abrir o código e **adicionar manualmente os nomes dos arquivos `.dat`** que deseja analisar no vetor `arquivos[]`.
2.  Compile o arquivo:
    ```bash
    g++ TP_Grafos_Etapa1.cpp -o TP_Grafos_Etapa1
    ```
3.  Execute o programa:
    ```bash
    ./TP_Grafos_Etapa1
    ```
4.  O programa irá calcular e exibir no console as seguintes métricas para cada grafo:
      - Quantidade de vértices, arestas e arcos (requeridos e não requeridos).
      - Densidade do grafo.
      - Grau mínimo e máximo.
      - E outras métricas como intermediação, caminho médio e diâmetro.

-----

## TP\_Grafos\_Etapa3 (Resolvedor de Roteamento)

### Arquivo principal:

`TP_Grafos_Etapa3.cpp`

### Descrição:

Este é um resolvedor avançado que aplica uma meta-heurística híbrida para encontrar soluções de alta qualidade para o Problema de Roteamento de Veículos Capacitado com Serviços em Nós e Arcos.

A arquitetura do algoritmo é composta por:

  - **Heurística de Construção:** **Clarke & Wright (Savings)** para gerar uma solução inicial focada em minimizar o número de veículos.
  - **Busca Local:** **Variable Neighborhood Descent (VND)**, que explora sistematicamente 4 tipos de movimentos para refinar a solução: `Relocate`, `Swap`, `2-Opt` e `(2,1)-Exchange`.
  - **Meta-heurística Global:** **Iterated Local Search (ILS)**, que usa uma perturbação poderosa para escapar de ótimos locais e explorar o espaço de busca de forma ampla.
  - **Mecanismo de Perturbação:** **Large Neighborhood Search (LNS)**, que "destrói" uma parte da solução e a "repara" de forma inteligente.

### Requisitos:

  - Compilador C++17 ou superior (devido ao uso de `<filesystem>`).

### Como executar:

1.  Coloque todos os arquivos de instância (`.dat`) na mesma pasta que o executável.
2.  Compile o arquivo:
    ```bash
    g++ -std=c++17 TP_Grafos_Etapa3.cpp -o TP_Grafos_Etapa3
    ```
3.  Execute o programa. Ele irá **automaticamente** encontrar e processar todos os arquivos `.dat` na pasta.
    ```bash
    ./TP_Grafos_Etapa3
    ```
4.  O programa irá:
      - Para cada instância, aplicar a sequência `Savings -> VND -> ILS` para encontrar uma solução otimizada.
      - Gerar um arquivo de solução formatado na pasta `solucoes/` (ex: `solucoes/sol-BHW1.txt`).
      - Exibir no console um resumo da solução final e as estatísticas de performance dos operadores da busca local.

-----

## Visualiza\_Grafos (Python)

### Arquivo principal:

`Visualiza_Grafos.ipynb`

### Descrição:

Um notebook Jupyter que utiliza a biblioteca `matplotlib` para desenhar os grafos, oferecendo uma representação visual das instâncias.

### Requisitos:

  - Python 3
  - Jupyter Notebook ou Jupyter Lab
  - Bibliotecas: `matplotlib`
    ```bash
    pip install matplotlib jupyterlab
    ```

### Como executar:

1.  Inicie o Jupyter Lab/Notebook no seu terminal.
2.  Abra o arquivo `Visualiza_Grafos.ipynb`.
3.  Na célula de código apropriada, **informe o nome do arquivo da instância** que deseja visualizar.
4.  Execute as células do notebook para gerar e exibir o grafo.

## Autores

  - Izac Moreira
  - Nadson Matos
  - **Curso:** Sistemas de Informação
  - **Data da Versão Final:** 28 de junho de 2025