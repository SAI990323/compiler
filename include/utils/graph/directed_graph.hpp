#ifndef UTILS_GRAPH_DIRECTED_GRAPH_HPP
#define UTILS_GRAPH_DIRECTED_GRAPH_HPP

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <vector>

namespace utils {

  namespace graph {

    template <typename vertex_property_t, typename edge_property_t>
    class directed_graph_t
    {
    public:
      using vertex_property_pointer_t = std::shared_ptr<vertex_property_t>;
      using edge_property_pointer_t = std::shared_ptr<edge_property_t>;

      using edge_description_t = std::tuple<vertex_property_pointer_t, vertex_property_pointer_t, edge_property_pointer_t>;

    private:

    public:
      directed_graph_t()
      { }

      ~directed_graph_t()
      { }

      bool exist_vertex(vertex_property_pointer_t vertex_property)
      {
        return m_vertex_properties.count(vertex_property) > 0;
      }

      bool add_vertex(vertex_property_pointer_t vertex_property)
      {
        if (exist_vertex(vertex_property)) {
          return false;
        }
        m_vertex_properties.insert(vertex_property);
        return true;
      }

      bool remove_vertex(vertex_property_pointer_t vertex_property)
      {
        if (exist_vertex(vertex_property)) {
          for (auto&& [in, out, edge]: in_edge_descriptions(vertex_property)) {
            remove_edge(edge);
          }
          for (auto&& [in, out, edge]: out_edge_descriptions(vertex_property)) {
            remove_edge(edge);
          }
          m_in_edge_map.erase(vertex_property);
          m_out_edge_map.erase(vertex_property);
          m_edge_property_map.erase(vertex_property);
          m_vertex_properties.erase(vertex_property);
          return true;
        }
        return false;
      }

      std::size_t num_vertices()
      {
        return m_vertex_properties.size();
      }

      std::unordered_set<vertex_property_pointer_t> vertices()
      {
        std::unordered_set<vertex_property_pointer_t> vertices(m_vertex_properties.begin(), m_vertex_properties.end());
        return vertices;
      }

      bool exist_edge_with_property(edge_property_pointer_t edge_property)
      {
        return m_edge_properties.count(edge_property) > 0;
      }

      bool exist_edge_with_endpoints(vertex_property_pointer_t vertex_in, vertex_property_pointer_t vertex_out)
      {
        return m_edge_property_map.count(vertex_in) > 0 && m_edge_property_map[vertex_in].count(vertex_out) > 0;
      }

      bool add_edge(vertex_property_pointer_t vertex_in, vertex_property_pointer_t vertex_out, edge_property_pointer_t edge_property)
      {
        if (!exist_edge_with_property(edge_property) && !exist_edge_with_endpoints(vertex_in, vertex_out)) {
          m_edge_properties.insert(edge_property);

          m_endpoints_map[edge_property] = std::make_pair(vertex_in, vertex_out);
          m_edge_property_map[vertex_in][vertex_out] = edge_property;

          m_in_edge_map[vertex_out].insert(vertex_in);
          m_out_edge_map[vertex_in].insert(vertex_out);
          return true;
        }
        return false;
      }

      bool remove_edge(edge_property_pointer_t edge_property)
      {
        if (exist_edge_with_property(edge_property)) {
          auto& [in, out] = m_endpoints_map[edge_property];
          m_edge_property_map[in].erase(out);
          m_in_edge_map[out].erase(in);
          m_out_edge_map[in].erase(out);
          m_edge_properties.erase(edge_property);
          return true;
        }
        return false;
      }

      bool remove_edge(vertex_property_pointer_t vertex_in, vertex_property_pointer_t vertex_out)
      {
        if (exist_edge_with_endpoints(vertex_in, vertex_out)) {
          return remove_edge(m_edge_property_map[vertex_in][vertex_out]);
        }
        return false;
      }

      std::size_t num_edges()
      {
        return m_edge_properties.size();
      }

    private:
      edge_description_t null_edge_description()
      {
        static auto null_edge_description = std::make_tuple(vertex_property_pointer_t(), vertex_property_pointer_t(), edge_property_pointer_t());
        return null_edge_description;
      }

    public:
      std::pair<edge_description_t, bool> edge_description(vertex_property_pointer_t vertex_in, vertex_property_pointer_t vertex_out)
      {
        if (exist_edge_with_endpoints(vertex_in, vertex_out)) {
          return std::make_pair(
              std::make_tuple(vertex_in, vertex_out, m_edge_property_map[vertex_in][vertex_out]), 
              true
          );
        }
        return std::make_pair(null_edge_description(), false);
      }

      std::pair<edge_description_t, bool> edge_description(edge_property_pointer_t edge_property)
      {
        if (exist_edge_with_property(edge_property)) {
          auto endpoints = m_endpoints_map[edge_property];
          return std::make_pair(
              std::make_tuple(endpoints.first, endpoints.second, edge_property), 
              true
          );
        }
        return std::make_pair(null_edge_description(), false);
      }

      std::vector<edge_description_t> edge_descriptions()
      {
        std::vector<edge_description_t> edge_descriptions;
        for (auto it0: m_edge_property_map) {
          for (auto it1: it0.second) {
            edge_descriptions.emplace_back(it0.first, it1.first, it1.second);
          }
        }
        return edge_descriptions;
      }

      std::size_t in_degree(vertex_property_pointer_t vertex_out)
      {
        return m_in_edge_map[vertex_out].size();
      }

      std::vector<edge_description_t> in_edge_descriptions(vertex_property_pointer_t vertex_out)
      {
        std::vector<edge_description_t> edge_descriptions;
        for (auto& vertex_in: m_in_edge_map[vertex_out]) {
          edge_descriptions.push_back(edge_description(vertex_in, vertex_out).first);
        }
        return edge_descriptions;
      }

      std::size_t out_degree(vertex_property_pointer_t vertex_in)
      {
        return m_out_edge_map[vertex_in].size();
      }

      std::vector<edge_description_t> out_edge_descriptions(vertex_property_pointer_t vertex_in)
      {
        std::vector<edge_description_t> edge_descriptions;
        for (auto& vertex_out: m_out_edge_map[vertex_in]) {
          edge_descriptions.push_back(edge_description(vertex_in, vertex_out).first);
        }
        return edge_descriptions;
      }

      std::vector<edge_description_t> edges()
      {
        std::vector<edge_description_t> edges;
        for (auto& edge_property: m_edge_properties) {
          edges.push_back(edge_description(edge_property).first);
        }
        return edges;
      }

    private:
      std::unordered_set<vertex_property_pointer_t> m_vertex_properties;
      std::unordered_set<edge_property_pointer_t> m_edge_properties;

      std::unordered_map<
          edge_property_pointer_t,
          std::pair<vertex_property_pointer_t, vertex_property_pointer_t>
      > m_endpoints_map;
      std::unordered_map<
          vertex_property_pointer_t, 
          std::unordered_map<vertex_property_pointer_t, edge_property_pointer_t>
      > m_edge_property_map;

      std::unordered_map<
          vertex_property_pointer_t, 
          std::unordered_set<vertex_property_pointer_t>
      > m_in_edge_map;
      std::unordered_map<
          vertex_property_pointer_t, 
          std::unordered_set<vertex_property_pointer_t>
      > m_out_edge_map;
    };

  } // namespace graph

} // namespace utils

#endif // UTILS_GRAPH_DIRECTED_GRAPH_HPP