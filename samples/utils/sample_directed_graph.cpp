#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <algorithm>

#include <utils/graph/directed_graph.hpp>
#include <utils/io/smart_ifstream.hpp>

struct Vertex {
  std::size_t m_idx;
  bool is_finalize = false;
  std::string m_token_name;

  Vertex(std::size_t idx)
  : m_idx(idx)
  { }

  void set_finalize(const std::string& token_name)
  {
    is_finalize = true;
    m_token_name = token_name;
  }
};

struct Edge {
  std::string m_parameter;

  Edge(const std::string& parameter)
  : m_parameter(parameter)
  { }
};

using Graph = typename utils::graph::directed_graph_t<Vertex, Edge>;

bool operator<(const Graph::edge_description_t& lhs, const Graph::edge_description_t& rhs)
{
  return std::get<0>(lhs)->m_idx == std::get<0>(rhs)->m_idx
       ? std::get<1>(lhs)->m_idx < std::get<1>(rhs)->m_idx
       : std::get<0>(lhs)->m_idx < std::get<0>(rhs)->m_idx;
}

int main(int argc, char* argv[])
{
  using ifstream = utils::io::smart_ifstream;
  ifstream in_stream(argv[1]);

  // construct graph
  auto graph = std::make_shared<Graph>();

  std::size_t num_vertices, num_finalize_vertices;
  in_stream >> num_vertices >> num_finalize_vertices;

  std::vector<std::shared_ptr<Vertex>> vertices;
  for (std::size_t idx = 0; idx < num_vertices; ++idx) {
    auto vertex = std::make_shared<Vertex>(idx);
    vertices.push_back(vertex);
    graph->add_vertex(vertex);
  }

  while (num_finalize_vertices--) {
    std::size_t idx_vertex;
    std::string token_name;
    in_stream >> idx_vertex >> token_name;

    vertices[idx_vertex]->set_finalize(token_name);
  }

  {
    std::size_t idx_vertex_in, idx_vertex_out;
    std::string parameter;
    while (in_stream >> idx_vertex_in >> idx_vertex_out >> parameter) {
      auto edge = std::make_shared<Edge>(parameter);
      graph->add_edge(vertices[idx_vertex_in], vertices[idx_vertex_out], edge);
    }
  }

  // vertices
  std::cout << "Graph vertices count: " << graph->num_vertices() << "\n";
  std::cout << "Vertices:\n";
  for (auto &vertex: graph->vertices()) {
    std::cout << "\t<" << vertex->m_idx << (vertex->is_finalize ? " " + vertex->m_token_name : "") << ">\n";
  }
  std::cout << "\n\n";

  // edges
  std::cout << "Graph edges count: " << graph->num_edges() << "\n";
  std::cout << "Edges:\n";
  auto edges = graph->edge_descriptions();
  for (auto&& [in, out, edge]: edges) {
    std::cout << "\t<" << in->m_idx << " -> " << out->m_idx << " " << edge->m_parameter << ">\n";
  }
  std::cout << "\n";

  // sort edges
  std::cout << "Sorted edges:\n";
  std::vector<Graph::edge_description_t> vec(edges.begin(), edges.end());
  std::sort(vec.begin(), vec.end());
  for (auto&& [in, out, edge]: vec) {
    std::cout << "\t<" << in->m_idx << " -> " << out->m_idx << " " << edge->m_parameter << ">\n";
  }
  std::cout << "\n";

  // remove vertex
  // graph->remove_vertex(vertices[0]);

  // remove edge
  // graph->remove_edge(vertices[77], vertices[78]);

  // adjacency edges
  for (std::size_t idx = 0; idx < num_vertices; ++idx) {
    std::cout << "Vertex #" << idx 
              << ", indegree: " << graph->in_degree(vertices[idx]) 
              << ", outdegree: " << graph->out_degree(vertices[idx]) 
              << "\n";
    // in edges
    std::cout << "\tin edges:\n";
    for (auto&& [in, out, edge]: graph->in_edge_descriptions(vertices[idx])) {
      if (vertices[idx] != out) {
        std::cout << "\t\tERROR\n";
      } else {
        std::cout << "\t\t<" << in->m_idx << " -> " << out->m_idx << " " << edge->m_parameter << ">\n";
      }
    }
    std::cout << "\n";

    // out edges
    std::cout << "\tout edges:\n";
    for (auto&& [in, out, edge]: graph->out_edge_descriptions(vertices[idx])) {
      if (vertices[idx] != in) {
        std::cout << "\t\tERROR<" << in->m_idx << " -> " << out->m_idx << " " << edge->m_parameter << ">\n";
      } else {
        std::cout << "\t\t<" << in->m_idx << " -> " << out->m_idx << " " << edge->m_parameter << ">\n";
      }
    }
    std::cout << "\n";
    std::cout << "\n";
  }

  return 0;
}