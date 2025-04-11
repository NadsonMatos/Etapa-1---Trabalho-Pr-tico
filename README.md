# Trabalho Prático - Grafos - Etapa 1

Este repositório contém dois programas desenvolvidos como parte da disciplina GCC262 - Grafos e suas Aplicações do curso de Sistemas de Informação. O objetivo é analisar e visualizar grafos por meio de duas abordagens complementares:

- **TP_Grafos_Etapa1**: Um programa em C++ que realiza o processamento de grafos, extraindo métricas estruturais e exportando resultados.
- **Visualiza_Grafos**: Um notebook em Python (Jupyter) que desenha visualmente os grafos a partir dos arquivos processados no "TP_Grafos_Etapa1".

## Estrutura da pasta do Projeto

**vscode/** Arquivos do vscode como settings.json para executar o   código.

**output/** Pasta onde estão localizados os arquivos .dat para uso de teste.
                    
**README.md** Este arquivo.

**TP_Grafos_Etapa1.c++** Código-fonte do programa. 

**TP_Grafos_Etapa1.exe** Executável do programa.

**Visualiza_Grafos.ipynb**cNotebook Jupyter do programa de vizualização de grafos.


## TP_Grafos_Etapa1 (C++)

### Arquivo principal:
TP_Grafos_Etapa1.c++

### Requisitos:
Compilador C++ compatível (ex: g++ ou compilador do Visual Studio Code)

### Como executar:
1. Compile o arquivo se necessário:
   ````
    g++ TP_Grafos_Etapa1.c++ -o TP_Grafos_Etapa1
   ````
2. Execute o programa:
   ```
   ./TP_Grafos_Etapa1
   ```
3. **Atenção**: Antes de executar,**é necessário** abrir o código e **adicionar manualmente os nomes dos arquivos .dat que deseja executar** no vetor arquivos[].

4. O programa irá:

- Ler os arquivos .dat

- Calcular as seguintes métricas:

        1. Quantidade de vértices;
        2. Quantidade de arestas;
        3. Quantidade de arcos;
        4. Quantidade de vértices requeridos;
        5. Quantidade de arestas requeridas;
        6. Quantidade de arcos requeridos;
        7. Densidade do grafo (order strength);
        8. Grau mínimo dos vértices;
        9. Grau máximo dos vértices;
        10. Intermediação;
        11. Caminho médio;
        12. Diâmetro.

- Exibir os resultados no console

- Gerar arquivos .txt com os dados que foram processsdos retirados da pasta output/

## Visualiza_Grafos (Python / Jupyter Notebook)

### Arquivo principal:
Visualiza_Grafos.ipynb

### Requisitos:
- Python 3
- Jupyter Notebook instalado(navegador ou vs code)
- Biblioteca matplotlib instalada:
  ```bash
  pip install matplotlib
  ```
### Como executar:
1. Abra o arquivo **Visualiza_Grafos.ipynb** com o Jupyter (via navegador ou VS Code).
2. Na seção de arquivos do código no Jupyter, **adicione manualmente os .txt gerados pelo "TP_Grafos_Etapa1"**.
3. Execute o código para visualizar os grafos desenhados.

## Autores
- Izac Moreira
- Nadson Matos
- Curso: Sistemas de Informação
- Data: 11 de abril de 2025
