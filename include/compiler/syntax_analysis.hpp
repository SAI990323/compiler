#ifndef COMPILER_SYNTAX_ANALYSIS_HPP
#define COMPILER_SYNTAX_ANALYSIS_HPP

#include <iostream>
#include <stack>
#include <unordered_set>
#include <unordered_map>

#include <compiler/syntax.hpp>

namespace compiler
{
  class LL1_syntax_analyser_t
  {
  public:
    LL1_syntax_analyser_t(const syntax_t& syntax, const symbol_t& start_symbol)
    : _is_valid(true), _syntax(syntax), _start_symbol(start_symbol)
    {
      _build_first_set();
      _build_follow_set();
      _build_predict_table();
      _check_validation();
    }

    auto get_first_set(symbol_t symbol)
    {
      auto first_set = _first_set[symbol];
      if (symbol == epsilon_symbol()) {
        first_set.insert(epsilon_symbol());
      }
      return first_set;
    }

    template <typename ForwardIterator>
    auto get_first_set(
        ForwardIterator it_begin, ForwardIterator it_end)
    {
      symbol_t epsilon = epsilon_symbol();
      std::unordered_set<symbol_t> first_set;
      // X -> X_1 X_2 ... X_n
      bool flag_all_contain_epsilon = true;
      for (auto it = it_begin; it < it_end; ++it) {
        auto first_set_of_symbol = get_first_set(*it);
        for (auto&& terminate_symbol: first_set_of_symbol) {
          if (terminate_symbol != epsilon) {
            first_set.insert(terminate_symbol);
          }
        }
        if (first_set_of_symbol.count(epsilon) == 0) {
          flag_all_contain_epsilon = false;
          break;
        }
      }
      if (flag_all_contain_epsilon) {
        first_set.insert(epsilon);
      }
      return first_set;
    }

    auto get_follow_set(symbol_t symbol)
    {
      return _follow_set[symbol];
    }

    auto get_predict_table()
    {
      return _predict_table;
    }

    auto terminate_symbols()
    {
      return _syntax.terminate_symbols();
    }

    auto non_terminate_symbols()
    {
      return _syntax.non_terminate_symbols();
    }

    auto rules(symbol_t symbol)
    {
      return _syntax.rules(symbol);
    }

  private:
    void _build_first_set()
    {
      symbol_t epsilon = epsilon_symbol();

      // X is a ternimate symbol
      for (auto&& X: terminate_symbols()) {
        if (X == epsilon) continue;
        _first_set[X].insert(X);
      }

      // X is a non-terminate symbol
      for (bool flag_first = true, flag_added = false; 
          flag_first || flag_added; flag_first = false) {
        flag_added = false;
        for (auto&& X: non_terminate_symbols()) {
          for (auto&& rule: rules(X)) {
            if (rule.is_epsilon()) {
              // X -> epsilon, add epsilon into FIRST[X]
              auto&& [_, inserted] = _first_set[X].insert(epsilon);
              flag_added |= inserted;
            } else {
              // X -> Y_1 Y_2 ... Y_k (k >= 1)
              for (auto&& Yi: rule.rule_symbols) {
                // FIRST[Y_1] ... FIRST[Y_{i-1}] all contain epsilon, add
                // all symbols in FIRST[Y_i] into FIRST[X]
                for (auto&& exist_symbol: _first_set[Yi]) {
                  auto&& [_, inserted] = _first_set[X].insert(exist_symbol);
                  if (!inserted) {
                    break;
                  } else {
                    flag_added = true;
                  }
                }
                // FIRST[Y_i] doesn't contain epsilon
                if (_first_set[Yi].count(epsilon) == 0) {
                  break;
                }
              }
            }
          }
        }
        // loop till no symbol added into FIRST set
      }
    }

    void _build_follow_set()
    {
      symbol_t epsilon = epsilon_symbol(), delimiter = delimiter_symbol();

      // insert delimiter into follow set of start symbol
      _follow_set[_start_symbol].insert(delimiter);

      //
      for (bool flag_first = true, flag_added = false;
          flag_first || flag_added; flag_first = false)
      {
        flag_added = false;
        for (auto&& A: non_terminate_symbols()) {
          for (auto&& rule: rules(A)) {
            // A -> epsilon, bypass such condition
            if (rule.is_epsilon()) continue;
            auto& symbols = rule.rule_symbols;
            // A -> a B b (b may be epsilon)
            for (auto it_B = symbols.begin(), it_end = symbols.end(); 
                it_B < it_end; ++it_B) {
              // obtain FIRST[b]
              auto first_set_of_beta = get_first_set(it_B + 1, it_end);
              // add all symbols except epsilon into FOLLOW[B]
              for (auto&& exist_symbol: first_set_of_beta) {
                if (exist_symbol != epsilon) {
                  auto&& [_, inserted] = _follow_set[*it_B].insert(exist_symbol);
                  flag_added |= inserted;
                }
              }
              // if FIRST[b] contains epsilon, add FOLLOW[A] into FOLLOW[B]
              if (first_set_of_beta.count(epsilon) > 0) {
                for (auto&& exist_symbol: _follow_set[A]) {
                  auto&& [_, inserted] = _follow_set[*it_B].insert(exist_symbol);
                  flag_added |= inserted;
                }
              }
            }
          }
        }
        // loop until no symbol added into FOLLOW set
      }
    }

    void _build_predict_table()
    {
      symbol_t epsilon = epsilon_symbol(), delimiter = delimiter_symbol();

      for (auto&& A: non_terminate_symbols()) {
        for (auto&& alpha: rules(A)) {
          // A -> alpha
          auto first_set_of_alpha = get_first_set(
              alpha.rule_symbols.begin(), alpha.rule_symbols.end());
          // for all terminate symbol a in FIRST[alpha], insert A->alpha 
          // into M[A, a]
          for (auto&& a: first_set_of_alpha) {
            if (a == epsilon) continue;
            _predict_table[A][a].insert(alpha);
          }
          // if epsilon exists in FIRST[alpha], for each terminate symbol b
          // in FOLLOW[A], insert A->alpha into M[A, b]
          if (first_set_of_alpha.count(epsilon) > 0) {
            for (auto&& b: get_follow_set(A)) {
              _predict_table[A][b].insert(alpha);
            }
            // if epsilon exists in FIRST[alpha], and $ exists in FOLLOW[A],
            // insert A->alpha into M[A, $]
            if (get_follow_set(A).count(delimiter) > 0) {
              _predict_table[A][delimiter].insert(alpha);
            }
          }
        }
      }
    }

    void _check_validation()
    {
      for (auto&& [_, items]: get_predict_table()) {
        for (auto&& [_, rule_set]: items) {
          if (rule_set.size() > 1) {
            _is_valid = false;
            break;
          }
        }
        if (!_is_valid) break;
      }
    }

  public:
    explicit operator bool() const
    {
      return _is_valid;
    }

    template <typename ForwardIterator>
    void analysis(
        const ForwardIterator& it_begin, const ForwardIterator& it_end)
    {
      std::stack<symbol_t> symbols;
      symbols.push(_start_symbol);

      auto match_or_output = [&](symbol_t terminate_symbol) {
        while (!symbols.empty()) {
          auto top = symbols.top();
          symbols.pop();
          if (top == terminate_symbol) {
            std::cout << "[matched] " << terminate_symbol << "\n";
            break;
          } else {
            auto rule = *(_predict_table[top][terminate_symbol].begin());
            std::cout << rule << "\n";
            if (!rule.is_epsilon()) {
              for (auto reverse_it = rule.rule_symbols.rbegin(); 
                  reverse_it < rule.rule_symbols.rend(); ++reverse_it)
              {
                symbols.push(*reverse_it);
              }
            }
          }
        }
      };

      for (auto it = it_begin; it < it_end; ++it) {
        match_or_output(*it);
      }
      match_or_output(delimiter_symbol());
    }

  private:
    bool _is_valid;
    syntax_t _syntax;
    symbol_t _start_symbol;

    std::unordered_map<symbol_t, std::unordered_set<symbol_t>> _first_set;
    std::unordered_map<symbol_t, std::unordered_set<symbol_t>> _follow_set;

    std::unordered_map<
        symbol_t, 
        std::unordered_map<
            symbol_t, 
            std::unordered_set<production_rule_t>
        >
    > _predict_table;
  };
} // namespace compiler

#endif // COMPILER_SYNTAX_ANALYSIS_HPP