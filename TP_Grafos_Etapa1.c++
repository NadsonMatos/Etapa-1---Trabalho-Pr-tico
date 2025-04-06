#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

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

    arquivo.close();
}
int main() {
    map<int, No> nos;
    vector<Aresta> arestas;

    lerArquivo("BHW1.dat", nos, arestas);

    cout << "Total de nos: " << nos.size() << endl;
    cout << "Total de arestas: " << arestas.size() << endl;

    return 0;
}