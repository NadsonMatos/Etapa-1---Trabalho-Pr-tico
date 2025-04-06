#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <set>
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
        cerr << "Erro ao abrir o arquivo!" << endl;
        exit(1);
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
int main() {
    map<int, No> nos;
    vector<Aresta> arestas;
    lerArquivo("BHW1.dat", nos, arestas);

    
    set<int> conjunto_nos;
    int num_nos_requeridos = 0;
    int num_arestas_requeridas = 0;
    int num_arcos_requeridos = 0;
    int num_arestas = 0;
    int num_arcos = 0;
    map<int, vector<pair<int, int>>> adjacencia;

    for (size_t i = 0; i < arestas.size(); ++i) {
        Aresta& a = arestas[i];
        conjunto_nos.insert(a.origem);
        conjunto_nos.insert(a.destino);
        adjacencia[a.origem].push_back(make_pair(a.destino, a.custo));
        if (!a.direcionada) adjacencia[a.destino].push_back(make_pair(a.origem, a.custo));
        if (a.direcionada) {
            num_arcos++;
            if (a.requerido) num_arcos_requeridos++;
        } else {
            num_arestas++;
            if (a.requerido) num_arestas_requeridas++;
        }
    }

    for (map<int, No>::iterator it = nos.begin(); it != nos.end(); ++it) {
        conjunto_nos.insert(it->first);
        if (it->second.requerido) num_nos_requeridos++;
    }

    int V = conjunto_nos.size();

    cout << fixed << setprecision(4);
    cout << "1. Quantidade de nos: " << V << endl;
    cout << "2. Quantidade de arestas: " << num_arestas << endl;
    cout << "3. Quantidade de arcos: " << num_arcos << endl;
    return 0;
}