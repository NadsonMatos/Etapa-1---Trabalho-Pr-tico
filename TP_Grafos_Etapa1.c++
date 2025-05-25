/*
 * Autor: Izac Moreira e Nadson Matos
 * Curso: Sistemas de Informação
 * Projeto: Trabalho Prático - Grafos -  Etapa 2 
 * Data: 11 de abril de 2025
 * Descrição: Este programa lê arquivos de grafos no formato .dat,
 *            calcula métricas estruturais (grau, densidade, caminho médio, etc.),
 *            aplica o algoritmo de Floyd-Warshall e exporta a visualização.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <queue>
#include <limits>
#include <algorithm>
#include <iomanip>

using namespace std;

// Definimos um valor grande para representar "infinito"
const int INFINITO = 1e9;
int CAPACIDADE_VEICULO = 999; // será lido do arquivo

// Estrutura que representa um nó (vértice) do grafo
struct No {
    int id;
    int demanda = 0;
    bool requerido = false; // Indica se o nó é obrigatório
};

// Estrutura que representa uma aresta ou arco do grafo
struct Aresta {
    int origem, destino;
    int custo;
    int demanda = 0;
    bool requerido = false;
    bool direcionada = false;
};

struct ServicoRequerido {
    int id, origem, destino;
    int custo, demanda;
    bool visitado;
};

struct Rota {
    vector<ServicoRequerido> servicos;
    int custo_total, demanda_total;
};

int extrairCapacidade(const string& nomeArquivo) {
    ifstream arq(nomeArquivo);
    string linha;
    while (getline(arq, linha)) {
        if (linha.find("Capacity:") != string::npos) {
            stringstream ss(linha);
            string temp;
            int capacidade;
            ss >> temp >> capacidade;
            return capacidade;
        }
    }
    return 999;
}

// Função para ler um arquivo .dat e preencher as estruturas de nós e arestas
void lerArquivo(const string& nomeArquivo, map<int, No>& nos, vector<Aresta>& arestas) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << nomeArquivo << endl;
        return;
    }

    string linha;

    // Lê até encontrar a seção de nós requeridos
    while (getline(arquivo, linha)) {
        if (linha.find("ReN.") != string::npos) break;
    }

    // Lê os nós requeridos
    while (getline(arquivo, linha)) {
        if (linha.empty() || linha.find("ReE.") != string::npos) break;
        if (linha[0] == 'N') {
            string etiqueta;
            int demanda, custo_servico;
            stringstream ss(linha);
            ss >> etiqueta >> demanda >> custo_servico;
            int id = stoi(etiqueta.substr(1));
            nos[id] = {id, demanda, true};
        }
    }

    // Lê as arestas requeridas
    while (getline(arquivo, linha)) {
        if (linha.empty() || linha.find("EDGE") != string::npos) break;
        if (linha[0] == 'E') {
            string etiqueta;
            int origem, destino, custo, demanda, custo_servico;
            stringstream ss(linha);
            ss >> etiqueta >> origem >> destino >> custo >> demanda >> custo_servico;
            arestas.push_back({origem, destino, custo, demanda, true, false});
        }
    }

    // Lê as arestas não requeridas
    while (getline(arquivo, linha)) {
        if (linha.empty() || linha.find("ReA.") != string::npos) break;
        if (isdigit(linha[0])) {
            int origem, destino, custo;
            stringstream ss(linha);
            ss >> origem >> destino >> custo;
            arestas.push_back({origem, destino, custo, 0, false, false});
        }
    }

    // Lê os arcos requeridos (direcionados)
    while (getline(arquivo, linha)) {
        if (linha.empty() || linha.find("ARC") != string::npos) break;
        if (linha[0] == 'A') {
            string etiqueta;
            int origem, destino, custo, demanda, custo_servico;
            stringstream ss(linha);
            ss >> etiqueta >> origem >> destino >> custo >> demanda >> custo_servico;
            arestas.push_back({origem, destino, custo, demanda, true, true});
        }
    }

    // Lê os arcos não requeridos
    while (getline(arquivo, linha)) {
        if (linha.empty()) break;
        if (linha.substr(0, 3) == "NrA") {
            string etiqueta;
            int origem, destino, custo;
            stringstream ss(linha);
            ss >> etiqueta >> origem >> destino >> custo;
            arestas.push_back({origem, destino, custo, 0, false, true});
        }
    }

    arquivo.close();
}

// Identifica todos os serviços requeridos 
vector<ServicoRequerido> identificarServicos(const vector<Aresta>& arestas, const map<int, No>& nos) {
    vector<ServicoRequerido> servicos;
    int id = 1;

    // 1. Adiciona nós requeridos
    for (const auto& [_, no] : nos) {
        if (no.requerido && no.demanda > 0) {
            servicos.push_back({id, no.id, no.id, 0, no.demanda, false});
            cout << "  Serviço ID " << id << " (NO " << no.id << ")\n";
            id++;
        }
    }

    // 2. Adiciona arestas e arcos requeridos
    for (const auto& a : arestas) {
        if (a.requerido && a.demanda > 0) {
            servicos.push_back({id, a.origem, a.destino, a.custo, a.demanda, false});
            cout << "  Serviço ID " << id << " (ARESTA/ARCO " << a.origem << " → " << a.destino << ")\n";
            id++;
        }
    }


    return servicos;
}

// Função principal que processa o grafo de um arquivo e calcula métricas
void processarArquivo(const string& nomeArquivo) {
    map<int, No> nos;
    vector<Aresta> arestas;
    lerArquivo(nomeArquivo, nos, arestas);

    cout << "\n====== Resultados para o arquivo: " << nomeArquivo << " ======\n";

    set<int> conjunto_nos;
    int num_nos_requeridos = 0, num_arestas_requeridas = 0, num_arcos_requeridos = 0;
    int num_arestas = 0, num_arcos = 0;
    map<int, vector<pair<int, int>>> adjacencia;

    // Construção da lista de adjacência
    for (auto& a : arestas) {
        conjunto_nos.insert(a.origem);
        conjunto_nos.insert(a.destino);
        adjacencia[a.origem].push_back({a.destino, a.custo});
        if (!a.direcionada)
            adjacencia[a.destino].push_back({a.origem, a.custo});

        if (a.direcionada) {
            num_arcos++;
            if (a.requerido) num_arcos_requeridos++;
        } else {
            num_arestas++;
            if (a.requerido) num_arestas_requeridas++;
        }
    }

    // Contagem de nós requeridos
    for (auto& [id, no] : nos) {
        conjunto_nos.insert(id);
        if (no.requerido) num_nos_requeridos++;
    }

    // Cálculo da densidade
    int V = conjunto_nos.size();
    int total_conexoes = num_arestas + num_arcos;
    double densidade = (double)total_conexoes / (V * (V - 1));

    // Cálculo dos graus
    map<int, int> graus;
    for (auto& [u, vizinhos] : adjacencia) {
        graus[u] += vizinhos.size();
        for (auto& [v, _] : vizinhos) graus[v] += 0;
    }

    int grau_min = INFINITO, grau_max = 0;
    for (auto& [_, g] : graus) {
        grau_min = min(grau_min, g);
        grau_max = max(grau_max, g);
    }

    // Inicialização das distâncias para Floyd-Warshall
    map<int, map<int, int>> dist, pred;
    for (int u : conjunto_nos) {
        for (int v : conjunto_nos) {
            dist[u][v] = (u == v) ? 0 : INFINITO;
            pred[u][v] = -1;
        }
    }

    // Preenche com os custos diretos
    for (auto& [u, vizinhos] : adjacencia) {
        for (auto& [v, custo] : vizinhos) {
            dist[u][v] = min(dist[u][v], custo);
            pred[u][v] = u;
        }
    }

    // Algoritmo de Floyd-Warshall para caminho mínimo entre todos os pares
    for (int k : conjunto_nos) {
        for (int i : conjunto_nos) {
            for (int j : conjunto_nos) {
                if (dist[i][k] < INFINITO && dist[k][j] < INFINITO &&
                    dist[i][j] > dist[i][k] + dist[k][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    pred[i][j] = pred[k][j];
                }
            }
        }
    }

    // Cálculo da intermediacao
    map<int, int> intermediacao;
    for (int origem : conjunto_nos) {
        for (int destino : conjunto_nos) {
            if (origem == destino) continue;
            for (int k : conjunto_nos) {
                if (k != origem && k != destino &&
                    dist[origem][destino] == dist[origem][k] + dist[k][destino]) {
                    intermediacao[k]++;
                }
            }
        }
    }

    // Calcula diâmetro
    double soma = 0;
    int pares = 0, diametro = 0;
    for (int i : conjunto_nos) {
        for (int j : conjunto_nos) {
            if (i != j && dist[i][j] < INFINITO) {
                soma += dist[i][j];
                pares++;
                diametro = max(diametro, dist[i][j]);
            }
        }
    }

    // Calcula Caminho Médio
    double caminho_medio = soma / pares;

    // Impressão dos resultados
    cout << fixed << setprecision(4);
    cout << "1. Quantidade de vertices: " << V << endl;
    cout << "2. Quantidade de arestas: " << num_arestas << endl;
    cout << "3. Quantidade de arcos: " << num_arcos << endl;
    cout << "4. Nos requeridos: " << num_nos_requeridos << endl;
    cout << "5. Arestas requeridas: " << num_arestas_requeridas << endl;
    cout << "6. Arcos requeridos: " << num_arcos_requeridos << endl;
    cout << "7. Densidade: " << densidade << endl;
    cout << "8. Grau minimo: " << grau_min << endl;
    cout << "9. Grau maximo: " << grau_max << endl;
    cout << "10. Intermediacao:" << endl;
    for (auto& [no, valor] : intermediacao) {
        cout << "   No " << no << ": " << valor << endl;
    }
    cout << "11. Caminho medio: " << caminho_medio << endl;
    cout << "12. Diametro: " << diametro << endl;

    // Exportar grafo para visualização em Python
    string nomeSaida = nomeArquivo + ".txt";
    ofstream saida(nomeSaida);
    if (!saida.is_open()) {
        cerr << "Erro ao criar arquivo de visualização: " << nomeSaida << endl;
        return;
    }

    saida << "V: ";
    for (int v : conjunto_nos) {
        saida << v << " ";
    }
    saida << "\nE: ";
    for (const auto& a : arestas) {
        // Exporta arestas e arcos como pares
        saida << "(" << a.origem << "," << a.destino << ") ";
        if (!a.direcionada) {
            // Se não for direcionada, adiciona também o inverso para simetria
            saida << "(" << a.destino << "," << a.origem << ") ";
        }
    }
    saida << endl;
    saida.close();
}


int main() {
    vector<string> arquivos = {
        "BHW6.dat", "BHW2.dat", "BHW3.dat", "BHW4.dat"
    };

    for (const string& nomeArquivo : arquivos) {
        processarArquivo(nomeArquivo);
    }

    return 0;
}
