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

const int INFINITO = 1e9;

struct No {
    int id;
    int demanda = 0;
    bool requerido = false;
};

struct Aresta {
    int origem, destino;
    int custo;
    int demanda = 0;
    bool requerido = false;
    bool direcionada = false;
};

void lerArquivo(const string& nomeArquivo, map<int, No>& nos, vector<Aresta>& arestas) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << nomeArquivo << endl;
        return;
    }

    string linha;
    while (getline(arquivo, linha)) {
        if (linha.find("ReN.") != string::npos) break;
    }

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

    while (getline(arquivo, linha)) {
        if (linha.empty() || linha.find("ReA.") != string::npos) break;
        if (isdigit(linha[0])) {
            int origem, destino, custo;
            stringstream ss(linha);
            ss >> origem >> destino >> custo;
            arestas.push_back({origem, destino, custo, 0, false, false});
        }
    }

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

void processarArquivo(const string& nomeArquivo) {
    map<int, No> nos;
    vector<Aresta> arestas;
    lerArquivo(nomeArquivo, nos, arestas);

    cout << "\n====== Resultados para o arquivo: " << nomeArquivo << " ======\n";

    set<int> conjunto_nos;
    int num_nos_requeridos = 0, num_arestas_requeridas = 0, num_arcos_requeridos = 0;
    int num_arestas = 0, num_arcos = 0;
    map<int, vector<pair<int, int>>> adjacencia;

    for (auto& a : arestas) {
        conjunto_nos.insert(a.origem);
        conjunto_nos.insert(a.destino);
        adjacencia[a.origem].push_back({a.destino, a.custo});
        if (!a.direcionada) adjacencia[a.destino].push_back({a.origem, a.custo});
        if (a.direcionada) {
            num_arcos++;
            if (a.requerido) num_arcos_requeridos++;
        } else {
            num_arestas++;
            if (a.requerido) num_arestas_requeridas++;
        }
    }

    for (auto& [id, no] : nos) {
        conjunto_nos.insert(id);
        if (no.requerido) num_nos_requeridos++;
    }

    int V = conjunto_nos.size();
    int total_conexoes = num_arestas + num_arcos;
    double densidade = (double)total_conexoes / (V * (V - 1));

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

    map<int, map<int, int>> dist, pred;
    for (int u : conjunto_nos) {
        for (int v : conjunto_nos) {
            if (u == v) {
                dist[u][v] = 0;
                pred[u][v] = -1;
            } else {
                dist[u][v] = INFINITO;
                pred[u][v] = -1;
            }
        }
    }

    for (auto& [u, vizinhos] : adjacencia) {
        for (auto& [v, custo] : vizinhos) {
            dist[u][v] = min(dist[u][v], custo);
            pred[u][v] = u;
        }
    }

    for (int k : conjunto_nos) {
        for (int i : conjunto_nos) {
            for (int j : conjunto_nos) {
                if (dist[i][k] < INFINITO && dist[k][j] < INFINITO) {
                    if (dist[i][j] > dist[i][k] + dist[k][j]) {
                        dist[i][j] = dist[i][k] + dist[k][j];
                        pred[i][j] = pred[k][j];
                    }
                }
            }
        }
    }

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

    double caminho_medio = soma / pares;

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
    cout << "10. Intermediacao (simplificada):" << endl;
    for (auto& [no,valor] : intermediacao) {
        cout << "   No " << no << ": " << valor << endl;
    }
    cout << "11. Caminho medio: " << caminho_medio << endl;
    cout << "12. Diametro: " << diametro << endl;
}

int main() {
    vector<string> arquivos = {
        "BHW1.dat", "CBMix11.dat", "CBMix12.dat", "BHW2.dat", "CBMix19.dat", "CBMix16.dat"
    };

    for (const string& nomeArquivo : arquivos) {
        processarArquivo(nomeArquivo);
    }

    return 0;
}
