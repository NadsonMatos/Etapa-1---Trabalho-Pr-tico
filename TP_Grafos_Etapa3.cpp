/*
 * Autor: Izac Moreira e Nadson Matos
 * Curso: Sistemas de Informação
 * Projeto: Trabalho Prático - Grafos - Etapa 3 (Versão de Entrega)
 * Data: 28 de junho de 2025
 * Descrição: Implementação de uma meta-heurística híbrida (ILS + VND + LNS) para
 * resolver uma variação do Problema de Roteamento de Veículos (CARP/VRP).
 * O algoritmo constrói uma solução inicial com a Heurística de Savings
 * e a refina utilizando uma busca local VND e um Iterated Local Search
 * com perturbação LNS para obter soluções de alta qualidade.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <limits>
#include <iomanip>
#include <chrono>
#include <filesystem>
#include <algorithm>
#include <random>
#include <set>
#include <numeric>

using namespace std;
using namespace chrono;
namespace fs = filesystem;

// --- PARÂMETROS GLOBAIS ---
// Agrupa todos os parâmetros configuráveis do algoritmo para fácil ajuste.
struct Parametros {
    static inline const int MAX_ITERACOES_ILS = 200;
    static inline const int MAX_ITER_SEM_MELHORA = 50;
    static inline const double TAXA_DESTRUICAO = 0.20; // Percentual de serviços a serem removidos
};

// --- ESTRUTURAS DE DADOS ---

// Constante para identificar o nó de depósito (garagem).
const int ID_DEPOSITO = 1;

// Estrutura que representa um nó (vértice) do grafo.
struct No {
    int id;
    int demanda;
};

// Estrutura que representa uma aresta ou arco do grafo.
struct Aresta {
    int origem, destino;
    int custo;
    int demanda;
    bool ehRequerida;
    bool ehDirecionada;
};

// Estrutura de um serviço requerido (nó, aresta ou arco com demanda).
struct ServicoRequerido {
    int id_servico;
    int no_origem, no_destino;
    int custo;
    int demanda;
};

// Estrutura que representa uma rota completa de um veículo.
struct Rota {
    vector<ServicoRequerido> servicos;
    long long custo_total = 0; // Usa long long para evitar estouro de inteiro
    int demanda_total = 0;
};

// --- CLASSES ---

// Classe que encapsula a solução completa, composta por um conjunto de rotas.
class Solucao {
public:
    vector<Rota> rotas;
    long long custo_total_geral = 0; // Usa long long para o custo total
    
    // Calcula o custo total da solução somando os custos de todas as rotas individuais.
    void calcularCustoTotal() {
        custo_total_geral = 0;
        for (const auto& r : rotas) { 
            custo_total_geral += r.custo_total; 
        }
    }
    
    // Exporta a solução encontrada para um arquivo de texto e imprime o resumo no console.
    void exportar(const string& nomeInstancia, long long tempo_total_ns, long long tempo_solucao_ns, const vector<int>& stats) {
        fs::create_directory("solucoes");
        string nome_arquivo_saida = "solucoes/sol-" + fs::path(nomeInstancia).stem().string() + ".txt";
        ofstream out(nome_arquivo_saida);
        
        if (!out.is_open()) { cerr << "Erro ao criar arquivo de solução: " << nome_arquivo_saida << endl; return; }
        
        calcularCustoTotal();
        out << custo_total_geral << "\n";
        out << rotas.size() << "\n";
        out << tempo_total_ns << "\n";
        out << tempo_solucao_ns << "\n";
        
        for (size_t i = 0; i < rotas.size(); ++i) {
            const Rota& r = rotas[i];
            out << "0 1 " << (i + 1) << " " << r.demanda_total << " " << r.custo_total << " " 
                << (r.servicos.size() + 2) << " (D 0,1,1)";
            
            for (const auto& s : r.servicos) { out << " (S " << s.id_servico << "," << s.no_origem << "," << s.no_destino << ")"; }
            out << " (D 0,1,1)\n";
        }
        out.close();
        
        // Exibe um resumo no console
        cout << "\n--- Resumo da Solucao para: " << nomeInstancia << " ---" << endl;
        cout << "Custo Total da Solucao: " << custo_total_geral << endl;
        cout << "Numero de Rotas Geradas: " << rotas.size() << endl;
        cout << "\n--- Estatisticas de Melhoria dos Operadores VND ---" << endl;
        cout << "Relocate (1): \t\t" << stats.at(1) << " melhorias" << endl;
        cout << "Swap (2): \t\t" << stats.at(2) << " melhorias" << endl;
        cout << "2-Opt (3): \t\t" << stats.at(3) << " melhorias" << endl;
        cout << "(2,1)-Exchange (4): \t" << stats.at(4) << " melhorias" << endl;
        cout << "\nSolucao exportada com sucesso para: " << nome_arquivo_saida << endl;
    }
};

// Classe que armazena a representação do grafo e a matriz de distâncias.
class Grafo {
public:
    map<int, map<int, int>> distancias;
    
    // Roda o algoritmo de Floyd-Warshall para pré-calcular os menores caminhos entre todos os pares de nós.
    void calcularMenoresCaminhos(const vector<Aresta>& arestas, const map<int, No>& nos) {
        set<int> nos_existentes;
        for (const auto& par : nos) { nos_existentes.insert(par.first); }
        nos_existentes.insert(ID_DEPOSITO);
        for (const auto& aresta : arestas) {
            nos_existentes.insert(aresta.origem);
            nos_existentes.insert(aresta.destino);
        }
        for (int i : nos_existentes) {
            for (int j : nos_existentes) {
                distancias[i][j] = (i == j) ? 0 : numeric_limits<int>::max();
            }
        }
        for (const auto& aresta : arestas) {
            if (distancias.at(aresta.origem).at(aresta.destino) > aresta.custo) {
                distancias[aresta.origem][aresta.destino] = aresta.custo;
            }
            if (!aresta.ehDirecionada) {
                if (distancias.at(aresta.destino).at(aresta.origem) > aresta.custo) {
                    distancias[aresta.destino][aresta.origem] = aresta.custo;
                }
            }
        }
        vector<int> nos_ids(nos_existentes.begin(), nos_existentes.end());
        for (int k : nos_ids) {
            for (int i : nos_ids) {
                for (int j : nos_ids) {
                    if (distancias.at(i).at(k) != numeric_limits<int>::max() && distancias.at(k).at(j) != numeric_limits<int>::max()) {
                        long long dist_via_k = (long long)distancias.at(i).at(k) + distancias.at(k).at(j);
                        if (dist_via_k < distancias.at(i).at(j)) { 
                            distancias[i][j] = (int)dist_via_k; 
                        }
                    }
                }
            }
        }
    }
};

// Classe responsável por ler e armazenar todos os dados de uma instância do problema.
class Instancia {
public:
    int capacidade_veiculo = 999; 
    map<int, No> nos; 
    vector<Aresta> arestas; 
    vector<ServicoRequerido> servicos_requeridos; 
    string nome_base;
    
    // Construtor que recebe o nome do arquivo e orquestra a leitura completa.
    Instancia(const string& nomeArquivo) {
        this->nome_base = fs::path(nomeArquivo).stem().string(); 
        lerDeArquivo(nomeArquivo);
    }
private:
    // Extrai a capacidade do veículo a partir do cabeçalho do arquivo.
    void extrairCapacidade(const string& nomeArquivo) { 
        ifstream arq(nomeArquivo); string linha; 
        while (getline(arq, linha)) { if (linha.find("Capacity:") != string::npos) { stringstream ss(linha); string temp; ss >> temp >> capacidade_veiculo; return; } } 
    }
    
    // Identifica todos os serviços obrigatórios (com demanda > 0).
    void identificarServicos() {
        int id = 1;
        for (const auto& [id_no, no] : nos) { 
            if (no.demanda > 0) { servicos_requeridos.push_back({id++, no.id, no.id, 0, no.demanda}); } 
        }
        for (const auto& aresta : arestas) { 
            if (aresta.ehRequerida && aresta.demanda > 0) { servicos_requeridos.push_back({id++, aresta.origem, aresta.destino, aresta.custo, aresta.demanda}); } 
        }
    }
    
    // Lê o arquivo de instância (.dat) e preenche as estruturas de dados.
    void lerDeArquivo(const string& nomeArquivo) {
        extrairCapacidade(nomeArquivo);
        ifstream arquivo(nomeArquivo); 
        if (!arquivo.is_open()) { cerr << "Erro ao abrir arquivo: " << nomeArquivo << endl; return; }
        
        string linha, secao_atual = "";
        while (getline(arquivo, linha)) {
            if (linha.empty() || linha.rfind("#", 0) == 0) continue;
            if (linha.find("ReN.") != string::npos) { secao_atual = "ReN"; continue; } 
            if (linha.find("ReE.") != string::npos) { secao_atual = "ReE"; continue; }
            if (linha.find("EDGE") != string::npos) { secao_atual = "EDGE"; continue; } 
            if (linha.find("ReA.") != string::npos) { secao_atual = "ReA"; continue; }
            if (linha.find("ARC") != string::npos) { secao_atual = "ARC"; continue; } 
            if (linha.find("END") != string::npos) { break; }
            
            stringstream ss(linha);
            if (secao_atual == "ReN" && (linha[0] == 'N')) { 
                string e; int d, c; ss >> e >> d >> c; int id = stoi(e.substr(1)); nos[id] = {id, d}; 
            } else if (secao_atual == "ReE" && linha[0] == 'E') { 
                string e; int o, d, c, dm, cs; ss >> e >> o >> d >> c >> dm >> cs; arestas.push_back({o, d, c, dm, true, false}); 
            } else if (secao_atual == "EDGE" && isdigit(linha[0])) { 
                int o, d, c; ss >> o >> d >> c; arestas.push_back({o, d, c, 0, false, false}); 
            } else if (secao_atual == "ReA" && linha[0] == 'A') { 
                string e; int o, d, c, dm, cs; ss >> e >> o >> d >> c >> dm >> cs; arestas.push_back({o, d, c, dm, true, true}); 
            } else if (secao_atual == "ARC") { 
                if (isdigit(linha[0])) { int o,d,c; ss >> o >> d >> c; arestas.push_back({o, d, c, 0, false, true}); } 
                else if (linha.rfind("NrA", 0) == 0) { string e; int o,d,c; ss >> e >> o >> d >> c; arestas.push_back({o, d, c, 0, false, true}); } 
            }
        }
        identificarServicos();
    }
};

// --- FUNÇÕES AUXILIARES E DE LÓGICA ---

// Recalcula o custo e a demanda totais de uma rota, garantindo consistência.
void recalcularCustoERota(Rota& rota, const Grafo& grafo) {
    if (rota.servicos.empty()) { rota.custo_total = 0; rota.demanda_total = 0; return; }
    long long custo = 0; int demanda = 0; int pos_atual = ID_DEPOSITO;
    for (const auto& servico : rota.servicos) {
        custo += grafo.distancias.at(pos_atual).at(servico.no_origem);
        custo += servico.custo; 
        demanda += servico.demanda; 
        pos_atual = servico.no_destino;
    }
    custo += grafo.distancias.at(pos_atual).at(ID_DEPOSITO);
    rota.custo_total = custo; rota.demanda_total = demanda;
}

// Valida a solução final, verificando todas as restrições.
bool validarSolucao(const Solucao& solucao, const Instancia& instancia) {
    set<int> servicos_atendidos; int total_demandas = 0;
    for (const auto& rota : solucao.rotas) {
        if (rota.demanda_total > instancia.capacidade_veiculo) {
            cerr << "ERRO: Rota excede capacidade (" << rota.demanda_total << " > " << instancia.capacidade_veiculo << ")" << endl;
            return false;
        }
        total_demandas += rota.demanda_total;
        for (const auto& servico : rota.servicos) {
            if (servicos_atendidos.count(servico.id_servico)) {
                cerr << "ERRO: Servico " << servico.id_servico << " atendido mais de uma vez" << endl;
                return false;
            }
            servicos_atendidos.insert(servico.id_servico);
        }
    }
    if (servicos_atendidos.size() != instancia.servicos_requeridos.size()) {
        cerr << "ERRO: Numero de servicos atendidos (" << servicos_atendidos.size() << ") diferente do requerido (" << instancia.servicos_requeridos.size() << ")" << endl;
        return false;
    }
    return true;
}

// Estrutura auxiliar para a Heurística de Savings.
struct Economia {
    int id_servico_i, id_servico_j;
    int valor;
    bool operator<(const Economia& outra) const { return valor > outra.valor; }
};

// Heurística construtiva de Clarke & Wright (Savings), focada em minimizar o número de rotas.
Solucao construirSolucaoComSavings(const Instancia& instancia, const Grafo& grafo) {
    Solucao solucao;
    const vector<ServicoRequerido>& servicos = instancia.servicos_requeridos;
    if (servicos.empty()) return solucao;
    vector<Rota> rotas_em_construcao;
    map<int, int> mapa_servico_para_rota_idx;
    for (size_t i = 0; i < servicos.size(); ++i) {
        Rota r; r.servicos.push_back(servicos[i]);
        recalcularCustoERota(r, grafo);
        rotas_em_construcao.push_back(r);
        mapa_servico_para_rota_idx[servicos[i].id_servico] = i;
    }
    vector<Economia> economias;
    for (size_t i = 0; i < servicos.size(); ++i) {
        for (size_t j = 0; j < servicos.size(); ++j) {
            if (i == j) continue;
            const auto& servico_i = servicos[i]; const auto& servico_j = servicos[j];
            int valor_economia = grafo.distancias.at(servico_i.no_destino).at(ID_DEPOSITO) +
                                 grafo.distancias.at(ID_DEPOSITO).at(servico_j.no_origem) -
                                 grafo.distancias.at(servico_i.no_destino).at(servico_j.no_origem);
            if (valor_economia > 0) {
                economias.push_back({servico_i.id_servico, servico_j.id_servico, valor_economia});
            }
        }
    }
    sort(economias.begin(), economias.end());
    for (const auto& economia : economias) {
        int idx_rota_i = mapa_servico_para_rota_idx.at(economia.id_servico_i);
        int idx_rota_j = mapa_servico_para_rota_idx.at(economia.id_servico_j);
        if (idx_rota_i == idx_rota_j) continue;
        Rota& rota_i = rotas_em_construcao[idx_rota_i];
        Rota& rota_j = rotas_em_construcao[idx_rota_j];
        if (rota_i.servicos.empty() || rota_j.servicos.empty()) continue;
        if (rota_i.servicos.back().id_servico != economia.id_servico_i || rota_j.servicos.front().id_servico != economia.id_servico_j) continue;
        if (rota_i.demanda_total + rota_j.demanda_total > instancia.capacidade_veiculo) continue;
        rota_i.servicos.insert(rota_i.servicos.end(), rota_j.servicos.begin(), rota_j.servicos.end());
        for (const auto& servico_movido : rota_j.servicos) {
             mapa_servico_para_rota_idx[servico_movido.id_servico] = idx_rota_i;
        }
        rota_j.servicos.clear();
        rota_j.demanda_total = 0;
        rota_j.custo_total = 0;
        recalcularCustoERota(rota_i, grafo);
    }
    for (const auto& r : rotas_em_construcao) {
        if (!r.servicos.empty()) { solucao.rotas.push_back(r); }
    }
    solucao.calcularCustoTotal();
    return solucao;
}

// Busca Local com Descida em Vizinhança Variável (VND). Explora sistematicamente múltiplos tipos de movimento.
void buscaLocalVND(Solucao& solucao, const Grafo& grafo, int capacidade_veiculo, vector<int>& melhorias_por_vizinhanca) {
    vector<int> vizinhancas = {1, 2, 3, 4}; // 1:Relocate, 2:Swap, 3:2-Opt, 4:(2,1)-Exchange
    size_t k = 0;
    while (k < vizinhancas.size()) {
        bool melhora_encontrada = false;
        switch (vizinhancas[k]) {
        case 1: { // VIZINHANÇA 1: RELOCATE (INTER-ROTAS)
            for (size_t i = 0; i < solucao.rotas.size() && !melhora_encontrada; ++i) {
                for (size_t l = 0; l < solucao.rotas[i].servicos.size() && !melhora_encontrada; ++l) {
                    ServicoRequerido servico_movido = solucao.rotas[i].servicos[l];
                    for (size_t j = 0; j < solucao.rotas.size() && !melhora_encontrada; ++j) {
                        if (i == j || solucao.rotas[j].demanda_total + servico_movido.demanda > capacidade_veiculo) continue;
                        long long custo_original = solucao.rotas[i].custo_total + solucao.rotas[j].custo_total;
                        for (size_t m = 0; m <= solucao.rotas[j].servicos.size(); ++m) {
                            Rota temp_i = solucao.rotas[i]; Rota temp_j = solucao.rotas[j];
                            temp_i.servicos.erase(temp_i.servicos.begin() + l);
                            temp_j.servicos.insert(temp_j.servicos.begin() + m, servico_movido);
                            recalcularCustoERota(temp_i, grafo); recalcularCustoERota(temp_j, grafo);
                            if (temp_i.custo_total + temp_j.custo_total < custo_original) {
                                solucao.rotas[i] = temp_i; solucao.rotas[j] = temp_j;
                                melhora_encontrada = true; break;
                            }
                        }
                    }
                }
            }
            break;
        }
        case 2: { // VIZINHANÇA 2: SWAP (1,1)
            for (size_t i = 0; i < solucao.rotas.size() && !melhora_encontrada; ++i) {
                for (size_t j = i + 1; j < solucao.rotas.size() && !melhora_encontrada; ++j) {
                    for (size_t l = 0; l < solucao.rotas[i].servicos.size() && !melhora_encontrada; ++l) {
                        for (size_t m = 0; m < solucao.rotas[j].servicos.size(); ++m) {
                            if (solucao.rotas[i].demanda_total - solucao.rotas[i].servicos[l].demanda + solucao.rotas[j].servicos[m].demanda > capacidade_veiculo ||
                                solucao.rotas[j].demanda_total - solucao.rotas[j].servicos[m].demanda + solucao.rotas[i].servicos[l].demanda > capacidade_veiculo) continue;
                            long long custo_original = solucao.rotas[i].custo_total + solucao.rotas[j].custo_total;
                            swap(solucao.rotas[i].servicos[l], solucao.rotas[j].servicos[m]);
                            recalcularCustoERota(solucao.rotas[i], grafo); recalcularCustoERota(solucao.rotas[j], grafo);
                            if (solucao.rotas[i].custo_total + solucao.rotas[j].custo_total < custo_original) {
                                melhora_encontrada = true; break;
                            }
                            swap(solucao.rotas[i].servicos[l], solucao.rotas[j].servicos[m]);
                            recalcularCustoERota(solucao.rotas[i], grafo); recalcularCustoERota(solucao.rotas[j], grafo);
                        }
                    }
                }
            }
            break;
        }
        case 3: { // VIZINHANÇA 3: 2-OPT (INTRA-ROTA)
            for (size_t r = 0; r < solucao.rotas.size() && !melhora_encontrada; ++r) {
                auto& servicos = solucao.rotas[r].servicos;
                if (servicos.size() < 2) continue;
                long long custo_rota_original = solucao.rotas[r].custo_total;
                for (size_t i = 0; i < servicos.size() - 1 && !melhora_encontrada; ++i) {
                    for (size_t j = i + 1; j < servicos.size(); ++j) {
                        reverse(servicos.begin() + i + 1, servicos.begin() + j + 1);
                        recalcularCustoERota(solucao.rotas[r], grafo);
                        if (solucao.rotas[r].custo_total < custo_rota_original) {
                            melhora_encontrada = true; break;
                        }
                        reverse(servicos.begin() + i + 1, servicos.begin() + j + 1);
                        recalcularCustoERota(solucao.rotas[r], grafo);
                    }
                }
            }
            break;
        }
        case 4: { // VIZINHANÇA 4: (2,1)-EXCHANGE (INTER-ROTA)
            for (size_t i = 0; i < solucao.rotas.size() && !melhora_encontrada; ++i) {
                if (solucao.rotas[i].servicos.size() < 2) continue;
                for (size_t l = 0; l < solucao.rotas[i].servicos.size() - 1 && !melhora_encontrada; ++l) {
                    ServicoRequerido s1 = solucao.rotas[i].servicos[l], s2 = solucao.rotas[i].servicos[l+1];
                    int demanda_par = s1.demanda + s2.demanda;
                    for (size_t j = 0; j < solucao.rotas.size() && !melhora_encontrada; ++j) {
                        if (i == j || solucao.rotas[j].demanda_total + demanda_par > capacidade_veiculo) continue;
                        long long custo_original = solucao.rotas[i].custo_total + solucao.rotas[j].custo_total;
                        for (size_t m = 0; m <= solucao.rotas[j].servicos.size(); ++m) {
                            Rota temp_i = solucao.rotas[i]; Rota temp_j = solucao.rotas[j];
                            temp_i.servicos.erase(temp_i.servicos.begin() + l, temp_i.servicos.begin() + l + 2);
                            temp_j.servicos.insert(temp_j.servicos.begin() + m, {s1, s2});
                            recalcularCustoERota(temp_i, grafo); recalcularCustoERota(temp_j, grafo);
                            if (temp_i.custo_total + temp_j.custo_total < custo_original) {
                                solucao.rotas[i] = temp_i; solucao.rotas[j] = temp_j;
                                melhora_encontrada = true; break;
                            }
                        }
                    }
                }
            }
            break;
        }
        }
        if (melhora_encontrada) {
            melhorias_por_vizinhanca[vizinhancas[k]]++;
            k = 0;
        } else {
            k++;
        }
    }
    solucao.rotas.erase(remove_if(solucao.rotas.begin(), solucao.rotas.end(), [](const Rota& r){ return r.servicos.empty(); }), solucao.rotas.end());
    solucao.calcularCustoTotal();
}

// Perturbação do tipo Large Neighborhood Search (LNS).
void perturbarComLNS(Solucao& solucao, const Grafo& grafo, mt19937& gen, int capacidade_veiculo) {
    if (solucao.rotas.empty()) return;

    vector<ServicoRequerido> todos_servicos;
    for(const auto& rota : solucao.rotas) {
        todos_servicos.insert(todos_servicos.end(), rota.servicos.begin(), rota.servicos.end());
    }
    if (todos_servicos.empty()) return;

    int num_a_remover = floor(todos_servicos.size() * Parametros::TAXA_DESTRUICAO);
    if (num_a_remover == 0 && !todos_servicos.empty()) num_a_remover = 1;

    vector<ServicoRequerido> servicos_removidos;
    set<int> ids_removidos;
    shuffle(todos_servicos.begin(), todos_servicos.end(), gen);
    for(int i=0; i<num_a_remover; ++i) {
        servicos_removidos.push_back(todos_servicos[i]);
        ids_removidos.insert(todos_servicos[i].id_servico);
    }
    
    for(auto& rota : solucao.rotas) {
        rota.servicos.erase(remove_if(rota.servicos.begin(), rota.servicos.end(), 
            [&](const ServicoRequerido& s){ return ids_removidos.count(s.id_servico); }),
            rota.servicos.end());
    }

    for(const auto& servico_a_inserir : servicos_removidos) {
        long long melhor_custo_insercao = numeric_limits<long long>::max();
        int melhor_rota_idx = -1;
        int melhor_pos_idx = -1;
        for (size_t i = 0; i < solucao.rotas.size(); ++i) {
            if (solucao.rotas[i].demanda_total + servico_a_inserir.demanda <= capacidade_veiculo) {
                for (size_t j = 0; j <= solucao.rotas[i].servicos.size(); ++j) {
                    long long no_anterior = (j == 0) ? ID_DEPOSITO : solucao.rotas[i].servicos[j-1].no_destino;
                    long long no_posterior = (j == solucao.rotas[i].servicos.size()) ? ID_DEPOSITO : solucao.rotas[i].servicos[j].no_origem;
                    long long delta = (grafo.distancias.at(no_anterior).at(servico_a_inserir.no_origem) + servico_a_inserir.custo + grafo.distancias.at(servico_a_inserir.no_destino).at(no_posterior)) - grafo.distancias.at(no_anterior).at(no_posterior);
                    if (delta < melhor_custo_insercao) {
                        melhor_custo_insercao = delta;
                        melhor_rota_idx = i;
                        melhor_pos_idx = j;
                    }
                }
            }
        }
        if (melhor_rota_idx != -1) {
            solucao.rotas[melhor_rota_idx].servicos.insert(solucao.rotas[melhor_rota_idx].servicos.begin() + melhor_pos_idx, servico_a_inserir);
            recalcularCustoERota(solucao.rotas[melhor_rota_idx], grafo);
        } else {
            Rota nova_rota;
            nova_rota.servicos.push_back(servico_a_inserir);
            recalcularCustoERota(nova_rota, grafo);
            solucao.rotas.push_back(nova_rota);
        }
    }
    
    solucao.rotas.erase(remove_if(solucao.rotas.begin(), solucao.rotas.end(), [](const Rota& r){ return r.servicos.empty(); }), solucao.rotas.end());
    solucao.calcularCustoTotal();
}

// Orquestra todo o processo de resolução para uma única instância.
void processarInstancia(const string& nomeArquivo) {
    cout << "\n==================================================" << endl;
    cout << "Processando instancia: " << nomeArquivo << endl;
    
    auto inicio_total = high_resolution_clock::now();
    Instancia instancia(nomeArquivo);
    Grafo grafo; grafo.calcularMenoresCaminhos(instancia.arestas, instancia.nos);
    auto inicio_solucao = high_resolution_clock::now();
    random_device rd; mt19937 gen(rd());
    vector<int> melhorias_por_vizinhanca(5, 0);

    // 1. Construção da solução inicial
    Solucao melhor_solucao_geral = construirSolucaoComSavings(instancia, grafo);
    
    // 2. Otimização inicial com busca local
    buscaLocalVND(melhor_solucao_geral, grafo, instancia.capacidade_veiculo, melhorias_por_vizinhanca);
    
    Solucao solucao_base_para_perturbacao = melhor_solucao_geral;
    int iter_sem_melhora = 0;
    
    // 3. Refinamento com Iterated Local Search
    for (int i = 0; i < Parametros::MAX_ITERACOES_ILS && iter_sem_melhora < Parametros::MAX_ITER_SEM_MELHORA; ++i) {
        Solucao solucao_de_trabalho = solucao_base_para_perturbacao;
        
        perturbarComLNS(solucao_de_trabalho, grafo, gen, instancia.capacidade_veiculo);
        buscaLocalVND(solucao_de_trabalho, grafo, instancia.capacidade_veiculo, melhorias_por_vizinhanca);
        
        if (solucao_de_trabalho.custo_total_geral < melhor_solucao_geral.custo_total_geral) {
            melhor_solucao_geral = solucao_de_trabalho;
        }
        
        if (solucao_de_trabalho.custo_total_geral < solucao_base_para_perturbacao.custo_total_geral) {
            solucao_base_para_perturbacao = solucao_de_trabalho;
            iter_sem_melhora = 0;
        } else {
            iter_sem_melhora++;
        }
    }
    
    auto fim_solucao = high_resolution_clock::now();
    if (!validarSolucao(melhor_solucao_geral, instancia)) {
        // A função validarSolucao já imprime o erro específico no cerr.
    }
    long long tempo_total_ns = duration_cast<nanoseconds>(fim_solucao - inicio_total).count();
    long long tempo_solucao_ns = duration_cast<nanoseconds>(fim_solucao - inicio_solucao).count();
    melhor_solucao_geral.exportar(nomeArquivo, tempo_total_ns, tempo_solucao_ns, melhorias_por_vizinhanca);
}

// Função principal que inicia o programa.
int main() {
    // Itera sobre todos os arquivos com extensão .dat na pasta atual.
    for (const auto& entry : fs::directory_iterator(".")) {
        if (entry.path().extension() == ".dat") {
            processarInstancia(entry.path().filename().string());
        }
    }
    cout << "\n==================================================" << endl;
    cout << "Processamento de todas as instancias concluido." << endl;
    return 0;
}