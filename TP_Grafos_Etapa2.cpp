#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <limits>
#include <iomanip>
#include <chrono>
#include <stack>
#include <filesystem>

using namespace std;
using namespace chrono;
namespace fs = filesystem;

int CAPACIDADE_VEICULO = 999; // será lido do arquivo

struct No {
    int id;
    int demanda;
    bool requerido;
};

struct Aresta {
    int origem, destino;
    int custo, demanda;
    bool requerido;
    bool direcionada;
};

struct ServicoRequerido {
    int id, origem, destino, custo, demanda;
    bool visitado;
};

struct Rota {
    vector<ServicoRequerido> servicos;
    int custo_total = 0, demanda_total = 0;
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

void lerArquivo(const string& nomeArquivo, map<int, No>& nos, vector<Aresta>& arestas) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) return;
    string linha;
    while (getline(arquivo, linha)) {
        if (linha.find("ReN.") != string::npos) break;
    }
    while (getline(arquivo, linha)) {
        if (linha.empty() || linha.find("ReE.") != string::npos) break;
        if (linha[0] == 'N') {
            string etiqueta;
            int demanda, custo;
            stringstream ss(linha);
            ss >> etiqueta >> demanda >> custo;
            int id = atoi(etiqueta.substr(1).c_str());
            nos[id] = {id, demanda, true};
        }
    }
    while (getline(arquivo, linha)) {
        if (linha.empty() || linha.find("EDGE") != string::npos) break;
        if (linha[0] == 'E') {
            string etiqueta;
            int o, d, custo, dem, cs;
            stringstream ss(linha);
            ss >> etiqueta >> o >> d >> custo >> dem >> cs;
            arestas.push_back({o, d, custo, dem, true, false});
        }
    }
    while (getline(arquivo, linha)) {
        if (linha.empty() || linha.find("ReA.") != string::npos) break;
        if (isdigit(linha[0])) {
            int o, d, custo;
            stringstream ss(linha);
            ss >> o >> d >> custo;
            arestas.push_back({o, d, custo, 0, false, false});
        }
    }
    while (getline(arquivo, linha)) {
        if (linha.empty() || linha.find("ARC") != string::npos) break;
        if (linha[0] == 'A') {
            string etiqueta;
            int o, d, custo, dem, cs;
            stringstream ss(linha);
            ss >> etiqueta >> o >> d >> custo >> dem >> cs;
            arestas.push_back({o, d, custo, dem, true, true});
        }
    }
    while (getline(arquivo, linha)) {
        if (linha.empty()) break;
        if (linha.substr(0, 3) == "NrA") {
            string etiqueta;
            int o, d, custo;
            stringstream ss(linha);
            ss >> etiqueta >> o >> d >> custo;
            arestas.push_back({o, d, custo, 0, false, true});
        }
    }
}

vector<ServicoRequerido> identificarServicos(const vector<Aresta>& arestas, const map<int, No>& nos) {
    vector<ServicoRequerido> servicos;
    int id = 1;
    for (const auto& [_, no] : nos) {
        if (no.requerido && no.demanda > 0) {
            servicos.push_back({id, no.id, no.id, 0, no.demanda, false});
            id++;
        }
    }
    for (const auto& a : arestas) {
        if (a.requerido && a.demanda > 0) {
            servicos.push_back({id, a.origem, a.destino, a.custo, a.demanda, false});
            id++;
        }
    }
    return servicos;
}

vector<Rota> construirRotas(vector<ServicoRequerido>& servicos, const vector<Aresta>& arestas) {
    vector<Rota> rotas;
    int atual = 0; // depósito
    while (true) {
        Rota rota;
        int carga = 0;
        while (true) {
            int melhor_idx = -1;
            double melhor_criterio = numeric_limits<double>::max();

            for (size_t i = 0; i < servicos.size(); ++i) {
                if (servicos[i].visitado || servicos[i].demanda + carga > CAPACIDADE_VEICULO) continue;

                int desloc = servicos[i].custo;
                for (const auto& a : arestas) {
                    if ((a.origem == atual && a.destino == servicos[i].origem) ||
                        (!a.direcionada && a.destino == atual && a.origem == servicos[i].origem)) {
                        desloc = a.custo;
                        break;
                    }
                }

                double criterio = static_cast<double>(desloc + servicos[i].custo) / (1 + servicos[i].demanda);

                if (criterio < melhor_criterio) {
                    melhor_criterio = criterio;
                    melhor_idx = i;
                }
            }

            if (melhor_idx == -1) break;

            ServicoRequerido& s = servicos[melhor_idx];
            s.visitado = true;
            rota.servicos.push_back(s);
            rota.demanda_total += s.demanda;
            rota.custo_total += s.custo;
            carga += s.demanda;
            atual = s.destino;
        }

        if (!rota.servicos.empty()) rotas.push_back(rota);

        bool todosVisitados = true;
        for (const auto& s : servicos) {
            if (!s.visitado) {
                todosVisitados = false;
                break;
            }
        }
        if (todosVisitados) break;
    }
    return rotas;
}

void exportarSolucao(const string& nomeInstancia, const vector<Rota>& rotas, long long clocks_execucao, long long clocks_solucao) {
    fs::create_directory("solucoes");
    string nome = "solucoes/sol-" + nomeInstancia;
    ofstream out(nome.c_str());
    if (!out.is_open()) return;
    int custo_total = 0;
    for (const auto& r : rotas) custo_total += r.custo_total;

    out << custo_total << "\n";
    out << rotas.size() << "\n";
    out << clocks_execucao << "\n";
    out << clocks_solucao << "\n";

    for (size_t i = 0; i < rotas.size(); i++) {
        const Rota& r = rotas[i];
        out << "0 1 " << (i + 1) << " " << r.demanda_total << " " << r.custo_total << " " << (r.servicos.size() + 2) << " (D 0,1,1)";
        for (const auto& s : r.servicos) {
            out << " (S " << s.id << "," << s.origem << "," << s.destino << ")";
        }
        out << " (D 0,1,1)\n";
    }

    cout << "Resumo dos custos por rota (" << nomeInstancia << "):\n";
    for (size_t i = 0; i < rotas.size(); i++) {
        cout << "  Rota " << (i + 1) << ": custo = " << rotas[i].custo_total << ", demanda = " << rotas[i].demanda_total << "\n";
    }
    cout << "Custo total: " << custo_total << "\n";
}

void processarArquivo(const string& nomeArquivo) {
    auto inicio_total = high_resolution_clock::now();

    CAPACIDADE_VEICULO = extrairCapacidade(nomeArquivo);
    map<int, No> nos;
    vector<Aresta> arestas;
    lerArquivo(nomeArquivo, nos, arestas);
    vector<ServicoRequerido> servicos = identificarServicos(arestas, nos);

    auto inicio_solucao = high_resolution_clock::now();
    vector<Rota> rotas = construirRotas(servicos, arestas);
    auto fim = high_resolution_clock::now();

    long long clocks_total = duration_cast<nanoseconds>(fim - inicio_total).count();
    long long clocks_solucao = duration_cast<nanoseconds>(fim - inicio_solucao).count();

    exportarSolucao(nomeArquivo, rotas, clocks_total, clocks_solucao);
}

int main() {
    for (const auto& entry : fs::directory_iterator(".")) {
        if (entry.path().extension() == ".dat") {
            processarArquivo(entry.path().filename().string());
        }
    }
    return 0;
}