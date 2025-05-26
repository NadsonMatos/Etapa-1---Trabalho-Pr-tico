/*
 * Autor: Izac Moreira e Nadson Matos
 * Curso: Sistemas de Informação
 * Projeto: Trabalho Prático - Grafos -  Etapa 2
 * Data: 26 de maio de 2025
 * Descrição: Este programa lê arquivos de instâncias no formato .dat relacionados a problemas de roteamento,
 *            identifica os serviços obrigatórios (com demanda), aplica o algoritmo de Floyd-Warshall para
 *            obter os menores caminhos entre nós, constrói rotas utilizando uma heurística gulosa respeitando
 *            a capacidade dos veículos, e exporta os resultados em um arquivo de solução no formato especificado.Ex:“sol-BHW1.dat”.
 */
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
namespace fs = filesystem; //Apelido para facilitar o uso de funcoes de arquivos

//Capacidade padrao, sobrescrita ao ler arquivo
int CAPACIDADE_VEICULO = 999;

//Estrutura que representa um no (vertice) do grafo
struct No {
    int id;
    int demanda;
    bool requerido; //Indica se o no eh um ponto de servico obrigatorio
};

//Estrutura que representa aresta ou arco do grafo
struct Aresta {
    int origem, destino;
    int custo, demanda;
    bool requerido; //Se representa uma aresta obrigatoria
    bool direcionada; //Se eh uma aresta direcionada (arco)
};

//Estrutura de um servico requerido (no, aresta ou arco com demanda)
struct ServicoRequerido {
    int id, origem, destino, custo, demanda;
    bool visitado; //Para controle durante a construcao das rotas
};

//Estrutura que representa uma rota completa de um veiculo com os sservicos acumulados
struct Rota {
    vector<ServicoRequerido> servicos;
    int custo_total = 0, demanda_total = 0;
};

//Algoritmo de Floyd-Warshall(Aproveitado da etapa 1)  para o calculo de menores distancias entre todos os pares de nos
map<int, map<int, int>> floydWarshall(const vector<Aresta>& arestas) {
    map<int, map<int, int>> dist;
    for (const auto& a : arestas) {
        dist[a.origem][a.origem] = 0;
        dist[a.destino][a.destino] = 0;
        dist[a.origem][a.destino] = a.custo;
        if (!a.direcionada)
            dist[a.destino][a.origem] = a.custo;
    }
    for (const auto& [k, _] : dist)
        for (const auto& [i, _1] : dist)
            for (const auto& [j, _2] : dist)
                if (dist[i].count(k) && dist[k].count(j)) {
                    int via_k = dist[i][k] + dist[k][j];
                    if (!dist[i].count(j) || via_k < dist[i][j])
                        dist[i][j] = via_k;
                }
    return dist;
}

//Aqui e extraida a capacidade do veiculo a partir do arquivo de entrada
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

//Leitura do arquivo de entrada e construcao das estruturas
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

//Identificacao dos servicos requeridos onde constroi a lista de tarefas que precisam ser atendidas (nos e arestas com demanda > 0).
vector<ServicoRequerido> identificarServicos(const vector<Aresta>& arestas, const map<int, No>& nos) {
    vector<ServicoRequerido> servicos;
    //Atribui um ID unico para cada servico.
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

//Neste trecho e utilizada uma heuristica gulosa para construir rotas
vector<Rota> construirRotas(vector<ServicoRequerido>& servicos, const vector<Aresta>& arestas) {
    vector<Rota> rotas;
    auto dist = floydWarshall(arestas);
    int atual = 0; // deposito

    while (true) {
        Rota rota;
        int carga = 0;
        while (true) {
            int melhor_idx = -1;
            double melhor_criterio = numeric_limits<double>::max();

            for (size_t i = 0; i < servicos.size(); ++i) {
                if (servicos[i].visitado || servicos[i].demanda + carga > CAPACIDADE_VEICULO) continue;

                int desloc = dist[atual][servicos[i].origem];
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
            rota.custo_total += s.custo + dist[atual][s.origem];
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

//Exporta a solucao encontrada para arquivo

void exportarSolucao(const string& nomeInstancia, const vector<Rota>& rotas, long long clocks_execucao, long long clocks_solucao) {
    fs::create_directory("solucoes"); //Cria pasta solucoes se ainda nao existir
    string nome = "solucoes/sol-" + nomeInstancia;
    ofstream out(nome.c_str());
    if (!out.is_open()) return;
    int custo_total = 0;
    for (const auto& r : rotas) custo_total += r.custo_total;
    //Escreve:Custo total da solucao; numero de rotas; Tempo total e tempo apenas da construcao da solucao; Detalhes de cada rota: demanda, custo, sequencia de servicos.
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
    //Exibe um resumo no console
    cout << "Resumo dos custos por rota (" << nomeInstancia << "):\n";
    for (size_t i = 0; i < rotas.size(); i++) {
        cout << "  Rota " << (i + 1) << ": custo = " << rotas[i].custo_total << ", demanda = " << rotas[i].demanda_total << "\n";
    }
    cout << "Custo total: " << custo_total << "\n";
}
//Funcao de processamento de uma instancia
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