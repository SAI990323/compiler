#ifndef UTILS_AUTOMATON_AUTOMATON_HPP
#define UTILS_AUTOMATON_AUTOMATON_HPP

#include <memory>
#include <utility>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <utils/graph/directed_graph.hpp>

namespace utils {

  namespace automaton {

    struct state_property_base
    {
      virtual bool is_finalize()
      { }
    };

    template <typename T>
    struct transition_property_base
    {
      virtual bool is_epsilon()
      { }

      virtual bool accept(T value)
      { }
    };

    template <typename state_property_t, typename transition_property_t>
    class automaton_t : public graph::directed_graph_t<state_property_t, transition_property_t>
    {
    private:
      using base_graph_t = graph::directed_graph_t<state_property_t, transition_property_t>;

    public:
      automaton_t(
          typename base_graph_t::vertex_property_pointer_t state_property)
      : m_start_state(state_property)
      {
        this->add_vertex(state_property);
      }

      typename base_graph_t::vertex_property_pointer_t start_state()
      {
        return m_start_state;
      }

    private:
      typename base_graph_t::vertex_property_pointer_t m_start_state;
    };

  } // namespace automaton

} // namespace utils

#endif // UTILS_AUTOMATON_HPP