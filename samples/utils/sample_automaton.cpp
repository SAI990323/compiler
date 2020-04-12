#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <stack>
#include <bitset>

#include <utils/automaton/automaton.hpp>
#include <utils/io/smart_ifstream.hpp>

struct State : utils::automaton::state_property_base
{
  std::size_t m_idx;
  bool m_is_finalize;
  std::string m_token_name;

  State(std::size_t idx)
  : m_idx(idx), m_is_finalize(false)
  { }

  void set_finalize(const std::string& token_name)
  {
    m_is_finalize = true;
    m_token_name = token_name;
  }

  bool is_finalize() override
  {
    return m_is_finalize;
  }
};

char translate_escape_character(char escape_character)
{
  static std::unordered_map<char, char> translation_map = {
    { 'b', ' ' }, { 't','\t' }, { 'n','\n' }, { '[', '[' }, { ']', ']' }, 
    { '.', '.' }, {'\\','\\' },
  };
  if (translation_map.count(escape_character) > 0) {
    return translation_map[escape_character];
  } else {
    return '\0';
  }
}

std::bitset<256> character_set_all()
{
  std::bitset<256> character_set = 0;
  character_set.set();
  return character_set;
}

std::bitset<256> parse_character_set_without_reverse(const std::string& pattern)
{
  std::bitset<256> character_set = 0;
  for (std::size_t idx = 0, length = pattern.length(); idx < length; ++idx) {
    if (pattern[idx] == '\\') {
      character_set.set(translate_escape_character(pattern[idx + 1]));
      ++idx;
    } else if (isdigit(pattern[idx]) && pattern[idx + 1] == '-' && idx + 2 < length && isdigit(pattern[idx + 2])) {
      for (auto ch = pattern[idx]; ch <= pattern[idx + 2]; ++ch) {
        character_set.set(ch);
      }
      idx += 2;
    } else if (isalpha(pattern[idx]) && pattern[idx + 1] == '-' && idx + 2 < length && isalpha(pattern[idx + 2])) {
      for (auto ch = pattern[idx]; ch <= pattern[idx + 2]; ++ch) {
        character_set.set(ch);
      }
      idx += 2;
    } else {
      character_set.set(pattern[idx]);
    }
  }
  return character_set;
}

std::bitset<256> parse_character_set(const std::string& pattern)
{
  std::bitset<256> character_set = 0;
  if (pattern[0] == '^') {
    if (pattern.length() > 1) {
      character_set = ~parse_character_set_without_reverse(pattern.substr(1));
    }
  } else {
    character_set = parse_character_set_without_reverse(pattern);
  }
  return character_set;
}

struct Transition : utils::automaton::transition_property_base<char>
{
  bool m_is_epsilon = false;
  std::bitset<256> m_character_set = 0;

  Transition(const std::string& parameter)
  {
    if (parameter == "epsilon") {
      // epsilon transition
      m_is_epsilon = true;
    } else if (parameter.length() == 1) {
      // single character
      if (parameter[0] == '.') {
        m_character_set = character_set_all();
      } else {
        m_character_set.set(parameter[0]);
      }
    } else if (parameter.length() == 2 && parameter[0] == '\\') {
      // escape character
      m_character_set.set(translate_escape_character(parameter[1]));
    } else if (parameter[0] == '[' && parameter.back() == ']') {
      // character range
      m_character_set = parse_character_set(parameter.substr(1, parameter.length() - 2));
    }
  }

  bool is_epsilon() override
  {
    return m_is_epsilon;
  }

  bool accept(char value) override
  {
    return m_character_set[value];
  }
};

using Automaton = utils::automaton::automaton_t<State, Transition>;

std::string smart_character_output(char c)
{
  if (c == ' ') {
    return "\\b";
  } else if (c == '\t') {
    return "\\t";
  } else if (c == '\n') {
    return "\\n";
  } else {
    return std::string { c };
  }
}

void smart_token_output(
    const std::string& token_id, 
    const std::string& accepted_string, 
    std::ostream& out_stream)
{
  out_stream << "< " << token_id << " ,\t";
  for (auto& ch: accepted_string) {
    out_stream << smart_character_output(ch);
  }
  out_stream << " >\n";
}

void scan(
    std::shared_ptr<Automaton> dfa, 
    const std::string& input_content,
    std::ostream& out_stream)
{
  for (std::size_t i_start = 0, i_offset;
      i_start < input_content.length();
      i_start += i_offset)
  {
    std::stack<typename Automaton::vertex_property_pointer_t> stack;
    stack.push(dfa->start_state());
    i_offset = 1;
    while (i_start + i_offset <= input_content.length() && !stack.empty()) {
      auto current_state = stack.top();
      bool found = false;
      for (auto&& [state_in, state_out, transition]:
          dfa->out_edge_descriptions(current_state)) 
      {
        if (transition->accept(input_content[i_start + i_offset - 1])) {
          found = true;
          stack.push(state_out);
          break;
        }
      }
      if (found) {
        i_offset += 1;
      } else {
        i_offset -= 1;
        break;
      }
    }
    while (i_offset > 0 && !stack.top()->is_finalize()) {
      i_offset -= 1;
      stack.pop();
    }
    if (i_offset == 0) {
      smart_token_output(
          "INVALID", 
          input_content.substr(i_start, 1), 
          out_stream);
      i_start += 1;
    } else {
      auto stopped_state = stack.top();
      if (stopped_state->m_token_name != "BLANK") {
        smart_token_output(
            stopped_state->m_token_name, 
            input_content.substr(i_start, i_offset), 
            out_stream);
      }
    }
  }
}

int main(int argc, char* argv[])
{
  using ifstream = utils::io::smart_ifstream;
  ifstream dfa_in_stream(argv[1]);

  // construct automaton
  std::vector<typename Automaton::vertex_property_pointer_t> states;
  states.push_back(std::make_shared<State>(0));
  
  std::size_t num_states, num_finalize_states;
  dfa_in_stream >> num_states >> num_finalize_states;

  auto dfa = std::make_shared<Automaton>(states[0]);

  for (std::size_t idx = 1; idx < num_states; ++idx) {
    auto state = std::make_shared<State>(idx);
    states.push_back(state);
    dfa->add_vertex(state);
  }

  while (num_finalize_states--) {
    std::size_t idx_state;
    std::string token_name;
    dfa_in_stream >> idx_state >> token_name;

    states[idx_state]->set_finalize(token_name);
  }

  {
    std::size_t idx_state_in, idx_state_out;
    std::string parameter;
    while (dfa_in_stream >> idx_state_in >> idx_state_out >> parameter) {
      auto transition = std::make_shared<Transition>(parameter);
      dfa->add_edge(states[idx_state_in], states[idx_state_out], transition);
    }
  }

  // scan
  std::ifstream code_in_stream(argv[2]);
  std::string input_content {
      std::istreambuf_iterator<char>(code_in_stream), 
      std::istreambuf_iterator<char>() 
  };
  scan(dfa, input_content, std::cout);

  return 0;
}