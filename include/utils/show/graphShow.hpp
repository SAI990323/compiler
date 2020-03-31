
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utils/io/smart_ifstream.hpp>

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
            finalizePoints[idx_state] = token_id + "_" + std::to_string(idx_state);
        }

        for (int i = 0; i < num_states; i++) {
            if (finalizePoints.count(i)) {
                points[i] = finalizePoints[i];
            } else {
                points[i] = "node_" + std::to_string(i);
            }
        }

        {
            std::size_t idx_state_in, idx_state_out;
            std::string parameter;
            while (in >> idx_state_in >> idx_state_out >> parameter) {
                if (find(idx_state_in) != find(idx_state_out) && (idx_state_in != 0 && idx_state_out != 0)) {
                    belong[std::max(idx_state_in, idx_state_out)] = find(std::min(idx_state_in, idx_state_out));
                }
                edges[std::make_pair(idx_state_in, idx_state_out)] = parameter;
            }
        }
    }

    void paint(int idx) {
        std::ofstream out_stream;

        out_stream.open("../samples/output/output_" + std::to_string(idx) + ".dot");

        out_stream << "digraph g {\n";
        out_stream << "\tnode [shape = doublecircle];\n";
        for (auto point : finalizePoints) {
            if (belong[point.first] == idx) {
                out_stream << "\t" << point.second << ";\n";
            }
        }
        out_stream << "\tnode [shape = circle];\n";
        for (auto edge : edges) {
            if (belong[edge.first.first] != idx || belong[edge.first.second] != idx) {
                continue;
            }
            std::string::size_type found = edge.second.find("\"");
            if (found == std::string::npos) {
                out_stream << "\t" << points[edge.first.first] << " -> " << points[edge.first.second] << " [label = \""
                           << edge.second + "\"]\n";
            } else {
                out_stream << "\t" << points[edge.first.first] << " -> " << points[edge.first.second] << " [label = \""
                           << "\\" << edge.second + "\"]\n";
            }
        }
        out_stream << "}\n";
    }

    void show() {
        for (int idx = 1; idx < total_points; idx++) {
            if (belong[idx] == idx) {
                belong[0] = idx;
                paint(idx);
                std::string command = "dot -Tpng ../samples/output/output_" + std::to_string(idx) + ".dot -o ../samples/output/output_" + std::to_string(idx) + ".png";
                system(command.c_str());
            }
        }
    }

private:
    std::map<int, std::string> finalizePoints;
    std::map<int, std::string> points;
    std::map<std::pair<int, int>, std::string> edges;
    std::unordered_map<int, int> belong;
    int total_points;

    int find(int x) { return belong[x] == x ? x : belong[x] = find(belong[x]); }
};
} // namespace show
} // namespace utils