
#include <bitset>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utils/io/smart_ifstream.hpp>
#include <vector>

namespace utils {

namespace show {

class graphShow {
public:
    void load(std::string filename) {
        using ifstream = utils::io::smart_ifstream;
        ifstream in(filename);
        std::size_t num_states, num_finalize_states;
        in >> num_states >> num_finalize_states;
        total_points = num_states;
        for (int i = 0; i < num_states; i++)
            belong[i] = i;
        while (num_finalize_states--) {
            std::size_t idx_state;
            std::string token_id;
            in >> idx_state >> token_id;
            std::cout << idx_state << " " << token_id << "\n";
            finalizePoints[idx_state] = token_id;
        }

        for (int i = 0; i < num_states; i++) {
            if (finalizePoints.count(i)) {
                points[i] = finalizePoints[i] + "_" + std::to_string(i);
            } else {
                points[i] = "node_" + std::to_string(i);
            }
            if (i > 0)
                belong[i] = i;
        }

        for (int i = 1; i < num_states; i++) {
            if (finalizePoints.count(i)) {
                for (int j = i + 1; j < num_states; j++) {
                    if (finalizePoints.count(j)) {
                            if (finalizePoints[i].compare(finalizePoints[j]) == 0) {
                                belong[j] = belong[i];
                                std :: cout << finalizePoints[i] << " " << i << " " <<  finalizePoints[j] << " " << j << "\n";
                            }
                        }
                }
                finalizePoints[i] = finalizePoints[i] + "_" + std::to_string(i);
            }
            
        }

        {
            std::size_t idx_state_in, idx_state_out;
            std::string parameter;
            while (in >> idx_state_in >> idx_state_out >> parameter) {
                if (find(idx_state_in) != find(idx_state_out) && (idx_state_in != 0 && idx_state_out != 0)) {
                    belong[find(std::max(idx_state_in, idx_state_out))] = find(std::min(idx_state_in, idx_state_out));
                }
                edges[std::make_pair(idx_state_in, idx_state_out)] = parameter;
                link_edge[idx_state_in].emplace_back(std::make_pair(idx_state_out, parameter));
                if (parameter.compare("null") != 0) {
                    symbols.insert(parameter);
                }
            }
        }
    }

    void paint(int idx, int block) {
        std::ofstream out_stream;

        out_stream.open("../samples/output/output_" + std::to_string(idx) + ".dot");

        out_stream << "digraph g {\n";
        out_stream << "\tnode [shape = doublecircle];\n";
        for (auto point : finalizePoints) {
            if (belong[point.first] == block) {
                out_stream << "\t" << point.second << ";\n";
            }
        }
        out_stream << "\tnode [shape = circle];\n";
        for (auto edge : edges) {
            if (belong[edge.first.first] != block || belong[edge.first.second] != block) {
                continue;
            }
            out_stream << "\t" << points[edge.first.first] << " -> " << points[edge.first.second] << " [label = \"";
            int len = edge.second.length();
            for (int idx = 0; idx < len; idx++) {
                if (edge.second[idx] == '\\' || edge.second[idx] == '"') {
                    out_stream << '\\' << edge.second[idx];
                } else {
                    out_stream << edge.second[idx];
                }
            }
            out_stream << "\", fontsize = 16]\n";
        }
        out_stream << "}\n";
    }

    void show() {
        for (int idx = 1; idx < total_points; idx++) {
            if (belong[idx] == idx) {
                belong[0] = idx;
                paint(++graph_cnt, idx);
                std::string command = "dot -Tpng ../samples/output/output_" + std::to_string(graph_cnt) + ".dot -o ../samples/output/output_" +
                                      std::to_string(graph_cnt) + ".png";
                system(command.c_str());
            }
        }
    }

    std::bitset<100> add_transfer(std::set<int>& dfa_set, std::string symbol) {
        while (true) {
            bool flag = false;
            static std::vector<int> v = {};
            for (auto point : dfa_set) {
                for (auto edge_pair : link_edge[point]) {
                    if (edge_pair.second.compare(symbol) == 0) {
                        v.emplace_back(edge_pair.first);
                    }
                }
            }
            for (auto point : v) {
                if (dfa_set.find(point) == dfa_set.end()) {
                    flag = true;
                    dfa_set.insert(point);
                }
            }
            if (!flag)
                break;
        }
        std::bitset<100> set;
        for (auto p : dfa_set) {
            set.set(p);
        }
        return set;
    }

    void nfa_to_dfa() {
        cnt = 0;
        dfa_set[0].insert(0);
        std::bitset<100> set = add_transfer(dfa_set[0], "null");
        if (dfa_points.find(set) == dfa_points.end()) {
            dfa_points[set] = cnt++;
            for (const auto point : dfa_set[0]) {
                if (finalizePoints.find(point) != finalizePoints.end()) {
                    finalizeDFAPoints[0] = "node_" + std::to_string(0);
                    break;
                }
            }
            dfaPoints[0] = "node_" + std::to_string(0);
        }
        std::queue<int> Q;
        Q.push(0);
        while (!Q.empty()) {
            int now = Q.front();
            Q.pop();
            std::set<int> points;
            points.clear();
            for (auto s : symbols) {
                points.clear();
                for (auto in : dfa_set[now]) {
                    for (auto p : link_edge[in]) {
                        if (p.second.compare(s) == 0) {
                            points.insert(p.first);
                        }
                    }
                }
                std::bitset<100> ans = add_transfer(points, "null");
                if (dfa_points.find(ans) == dfa_points.end()) {
                    for (auto point : points) {
                        if (finalizePoints.find(point) != finalizePoints.end()) {
                            finalizeDFAPoints[cnt] = "node_" + std::to_string(cnt);
                        }
                    }
                    dfaPoints[cnt] = "node_" + std::to_string(cnt);
                    dfa_points[ans] = cnt++;
                    dfa_set[cnt - 1] = points;
                    Q.push(cnt - 1);
                }
                if (points.size() > 0)
                    dfa_edges[std::make_pair(now, dfa_points[ans])] = s;
            }
        }
    }

    void dfa_show() {
        std::ofstream out_stream;

        out_stream.open("../samples/output/output_dfa.dot");

        out_stream << "digraph g {\n";
        out_stream << "\tnode [shape = doublecircle];\n";
        for (auto point : finalizeDFAPoints) {
            out_stream << "\t" << point.second << ";\n";
        }
        out_stream << "\tnode [shape = circle];\n";
        for (auto edge : dfa_edges) {
            out_stream << "\t" << dfaPoints[edge.first.first] << " -> " << dfaPoints[edge.first.second] << " [label = \"";
            int len = edge.second.length();
            for (int idx = 0; idx < len; idx++) {
                if (edge.second[idx] == '/' || edge.second[idx] == '"') {
                    out_stream << "/" << edge.second[idx];
                } else {
                    out_stream << edge.second[idx];
                }
            }
            out_stream << "\", fontsize = 8]\n";
        }
        out_stream << "}\n";
        out_stream.close();
        std::string command = "dot -Tpng ../samples/output/output_dfa.dot -o ../samples/output/output_dfa.png";
        system(command.c_str());
    }

private:
    std::map<int, std::string> finalizePoints, finalizeDFAPoints;
    std::map<int, std::string> points, dfaPoints;
    std::map<std::pair<int, int>, std::string> edges, dfa_edges;
    std::unordered_map<int, std::vector<std::pair<int, std::string>>> link_edge;
    std::unordered_map<int, std::set<int>> dfa_set;
    std::unordered_map<std::bitset<100>, int> dfa_points;
    std::unordered_map<int, int> belong;
    std::set<std::string> symbols;
    int total_points;
    int cnt;
    int graph_cnt = 0;

    int find(int x) { return belong[x] == x ? x : belong[x] = find(belong[x]); }
};
} // namespace show
} // namespace utils