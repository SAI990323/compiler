#ifndef COMPILER_SYNTAX_HPP
#define COMPILER_SYNTAX_HPP

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace compiler
{
  struct symbol_t
  {
    const bool is_terminate;
    const std::string symbol_id;
    const std::string content;
    symbol_t(const std::string& symbol_id, const std::string& content);
  };

  struct production_rule_t
  {
    const symbol_t symbol;
    std::vector<symbol_t> rule_symbols;

    production_rule_t();
    
    template <typename ForwardIterator>
    production_rule_t(const std::string& symbol_id,
        const ForwardIterator& it_begin, const ForwardIterator& it_end);

    bool is_epsilon() const;
  };

}

template <>
struct std::hash<compiler::symbol_t>
{
  std::size_t operator()(
      const compiler::symbol_t& symbol) const
  {
    return std::hash<std::string>()(symbol.symbol_id);
  }
};

template <>
struct std::hash<std::pair<int, std::string>>
{
  std::size_t operator()(const std::pair<int, std::string>& p) const
  {
    return std::hash<std::string>()(p.second + std::to_string(p.first));
  }
};

template <>
struct std::equal_to<std::pair<int, std::string>>
{
  bool operator()(const std::pair<int, std::string> &p, const std::pair<int, std::string> &q) const
  {
    return p.first == q.first && p.second == q.second;
  }
};

template <>
struct std::equal_to<compiler::symbol_t>
{
  bool operator()(
      const compiler::symbol_t& symbol_lhs, 
      const compiler::symbol_t& symbol_rhs) const
  {
    return symbol_lhs.symbol_id == symbol_rhs.symbol_id;
  }
};

template <>
struct std::hash<compiler::production_rule_t>
{
  std::size_t operator()(
      const compiler::production_rule_t& rule) const
  {
    std::size_t result = std::hash<compiler::symbol_t>()(rule.symbol);
    for (auto&& rule_symbol: rule.rule_symbols) {
      result |= std::hash<compiler::symbol_t>()(rule_symbol);
    }
    return result;
  }
};

template <>
struct std::equal_to<compiler::production_rule_t>
{
  bool operator()(
      const compiler::production_rule_t& rule_lhs, 
      const compiler::production_rule_t& rule_rhs) const
  {
    if (rule_lhs.symbol.symbol_id == rule_rhs.symbol.symbol_id
        && rule_lhs.rule_symbols.size() == rule_rhs.rule_symbols.size())
    {
      for (std::size_t idx = 0; idx < rule_lhs.rule_symbols.size(); ++idx) {
        if (rule_lhs.rule_symbols[idx].symbol_id != rule_rhs.rule_symbols[idx].symbol_id) {
          return false;
        }
      }
      return true;
    } else {
      return false;
    }
  }
};

namespace compiler
{
  // struct symbol_t
  symbol_t::symbol_t(const std::string& symbol_id, const std::string& content = "")
  : is_terminate(!isupper(symbol_id[0])),
    symbol_id(symbol_id),
    content(symbol_id[0] == '"' ? symbol_id.substr(1, symbol_id.length() - 2) : content)
  { }

  bool operator==(const symbol_t& symbol_lhs, const symbol_t& symbol_rhs)
  {
    return symbol_lhs.symbol_id == symbol_rhs.symbol_id;
  }

  bool operator!=(const symbol_t& symbol_lhs, const symbol_t& symbol_rhs)
  {
    return !(symbol_lhs == symbol_rhs);
  }

  std::ostream& operator<<(std::ostream& out_stream, const symbol_t& symbol)
  {
    return out_stream << symbol.symbol_id;
  }

  symbol_t epsilon_symbol()
  {
    static symbol_t epsilon("epsilon");
    return epsilon;
  }

  symbol_t delimiter_symbol()
  {
    static symbol_t delimiter("$");
    return delimiter;
  }

  // struct production_rule_t
  template <typename ForwardIterator>
  production_rule_t::production_rule_t(
      const std::string& symbol_id,
      const ForwardIterator& it_begin, const ForwardIterator& it_end)
  : symbol(symbol_id)
  {
    for (auto it = it_begin; it < it_end; ++it) {
      rule_symbols.emplace_back(*it);
    }
  }

  bool production_rule_t::is_epsilon() const
  {
    return rule_symbols.size() == 1 && rule_symbols[0] == epsilon_symbol();
  }

  std::ostream& operator<<(
      std::ostream& out_stream, const production_rule_t& rule)
  {
    out_stream << rule.symbol << " ->";
    for (auto&& rule_symbol: rule.rule_symbols) {
      out_stream << " " << rule_symbol;
    }
    return out_stream;
  }

  // class syntax_t
  class syntax_t
  {
  public:
    template <typename ForwardIterator>
    syntax_t(const ForwardIterator& it_begin, const ForwardIterator& it_end)
    {
      // used for checking validation
      std::unordered_set<symbol_t> symbols, symbols_with_rule;

      for (auto it = it_begin; it < it_end; ++it) {
        symbols_with_rule.insert(it->symbol);
        _non_terminate_symbols.insert(it->symbol);
        for (auto&& rule_symbol: it->rule_symbols) {
          if (!rule_symbol.is_terminate) {
            _non_terminate_symbols.insert(rule_symbol);
            symbols.insert(rule_symbol);
          } else {
            _terminate_symbols.insert(rule_symbol);
          }
        }
        bnfs[it->symbol].push_back(*it);
      }

      // check validation
      for (auto&& symbol: symbols) {
        if (symbols_with_rule.count(symbol) == 0) {
          is_valid = false;
          break;
        }
      }
    }

    std::vector<symbol_t> terminate_symbols()
    {
      std::vector<symbol_t> symbols { 
          _terminate_symbols.begin(), _terminate_symbols.end() };
      return symbols;
    }

    std::vector<symbol_t> non_terminate_symbols()
    {
      std::vector<symbol_t> symbols { 
          _non_terminate_symbols.begin(), _non_terminate_symbols.end() };
      return symbols;
    }

    std::vector<production_rule_t> rules(symbol_t symbol)
    {
      std::vector<production_rule_t> production_rules(bnfs[symbol]);
      return production_rules;
    }

    explicit operator bool() const
    {
      return is_valid;
    }

  private:
    bool is_valid = true;
    std::unordered_set<symbol_t> _terminate_symbols;
    std::unordered_set<symbol_t> _non_terminate_symbols;
    std::unordered_map<
        symbol_t,
        std::vector<production_rule_t>
    > bnfs;
  };

} // namespace compiler

#endif // COMPILER_SYNTAX_HPP