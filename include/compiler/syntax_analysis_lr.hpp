#ifndef COMPILER_SYNTAX_ANALYSIS_LR_HPP
#define COMPILER_SYNTAX_ANALYSIS_LR_HPP

#include <iostream>
#include <stack>
#include <unordered_set>
#include <unordered_map>

#include <compiler/syntax.hpp>

namespace compiler
{
class LR_syntax_analyser_t
{
public:
  LR_syntax_analyser_t(const syntax_t &syntax, const symbol_t &start_symbol)
    : _is_valid(true), _syntax(syntax), _start_symbol(start_symbol)
  {
    
    _build_first_set();
    get_all_rules();
    get_item();
  }

  auto get_first_set(symbol_t symbol)
  {
    auto first_set = _first_set[symbol];
    if (symbol == epsilon_symbol())
    {
      first_set.insert(epsilon_symbol());
    }
    if (symbol == delimiter_symbol()) {
      first_set.insert(delimiter_symbol());
    }
    return first_set;
  }

  auto actions()
  {
    return ACTION;
  }

  template <typename ForwardIterator>
  auto get_first_set(
      ForwardIterator it_begin, ForwardIterator it_end)
  {
    symbol_t epsilon = epsilon_symbol();
    std::unordered_set<symbol_t> first_set;
    // X -> X_1 X_2 ... X_n
    bool flag_all_contain_epsilon = true;
    for (auto it = it_begin; it < it_end; ++it)
    {
      auto first_set_of_symbol = get_first_set(*it);
      for (auto &&terminate_symbol : first_set_of_symbol)
      {
        if (terminate_symbol != epsilon)
        {
          first_set.insert(terminate_symbol);
        }
      }
      if (first_set_of_symbol.count(epsilon) == 0)
      {
        flag_all_contain_epsilon = false;
        break;
      }
    }
    if (flag_all_contain_epsilon)
    {
      first_set.insert(epsilon);
    }
    return first_set;
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
    for (auto &&X : terminate_symbols())
    {
      if (X == epsilon)
        continue;
      _first_set[X].insert(X);
    }

    // X is a non-terminate symbol
    for (bool flag_first = true, flag_added = false;
         flag_first || flag_added; flag_first = false)
    {
      flag_added = false;
      for (auto &&X : non_terminate_symbols())
      {
        for (auto &&rule : rules(X))
        {
          if (rule.is_epsilon())
          {
            // X -> epsilon, add epsilon into FIRST[X]
            auto &&[_, inserted] = _first_set[X].insert(epsilon);
            flag_added |= inserted;
          }
          else
          {
            // X -> Y_1 Y_2 ... Y_k (k >= 1)
            for (auto &&Yi : rule.rule_symbols)
            {
              // FIRST[Y_1] ... FIRST[Y_{i-1}] all contain epsilon, add
              // all symbols in FIRST[Y_i] into FIRST[X]
              for (auto &&exist_symbol : _first_set[Yi])
              {
                auto &&[_, inserted] = _first_set[X].insert(exist_symbol);
                if (!inserted)
                {
                  break;
                }
                else
                {
                  flag_added = true;
                }
              }
              // FIRST[Y_i] doesn't contain epsilon
              if (_first_set[Yi].count(epsilon) == 0)
              {
                break;
              }
            }
          }
        }
      }
      // loop till no symbol added into FIRST set
    }
  }

  void get_all_rules()
  {
    int cnt = 0, tot = 0;
    auto _non_terminate_symbols = non_terminate_symbols();
    for (auto &&symbol : _non_terminate_symbols)
    {
      auto rule = rules(symbol);
      for (auto &&ru : rule)
      {
        id2rule.insert({++cnt, ru});
        rule2id.insert({ru, cnt});
      }
    }
  }

  auto get_closure(int number)
  {
    for (bool flag = true; flag;)
    {
      flag = false;
      for (auto p : project[number])
      {
        bool flag = false;
        int rule_id = p.first;
        int idx = p.second.first;
        if (idx == id2rule[rule_id].rule_symbols.size()) 
          break;
        symbol_t next = symbol_t(p.second.second);
        std::vector<symbol_t> vec;
        if (id2rule[rule_id].rule_symbols[idx].is_terminate)
          continue;
        vec.emplace_back(id2rule[rule_id].rule_symbols[idx]);
        vec.emplace_back(next);
        auto first_set = get_first_set(vec.begin(), vec.end());
        for (auto &&ru : rules(id2rule[rule_id].rule_symbols[idx]))
        {
          auto y = rule2id[ru];
          for (auto &&symbol : first_set)
            _project[number].push_back({y, {0, symbol.symbol_id}});
          flag = true;
        }
      }
      for (auto p : _project[number])
      {
        project[number].push_back(p);
      }
      _project[number].clear();
    }
    return project[number];
  }

  void get_item()
  {
    int block = 0 ,tot = 0;
    int start = rule2id[rules(_start_symbol).front()];
    project[block].push_back({start, {0, delimiter_symbol().symbol_id}});
    get_closure(block);
    for (; block <= tot; block++)
    {
      get_closure(block);
      for (auto x : terminate_symbols())
      {
        for (auto y : project[block])
        {
          int rule_id = y.first;
          int idx = y.second.first;
          symbol_t next = symbol_t(y.second.second);
          int sz = id2rule[rule_id].rule_symbols.size();
          if (idx == sz)
          {
            // action
            ACTION[std::make_pair(block, next.symbol_id)] = std::make_pair("r", rule_id);

          } else {
            // yiru
            ACTION[std::make_pair(block, next.symbol_id)] = std::make_pair("s", tot + 1);
            project[++tot].push_back({rule_id, {idx + 1, y.second.second}});
          } 
        }
      }
      for (auto x : non_terminate_symbols())
      {
        for (auto y : project[block])
        {
          int rule_id = y.first;
          int idx = y.second.first;
          symbol_t next = symbol_t(y.second.second);
          int sz = id2rule[rule_id].rule_symbols.size();
          if (idx == sz) {
            // action
            ACTION[std::make_pair(block, next.symbol_id)] = std::make_pair("r", rule_id);
          } else {
            // yiru
            GOTO[std::make_pair(block, x.symbol_id)] = tot + 1;
            project[++tot].push_back({rule_id, {idx + 1, y.second.second}});
          }
        }
      }
    }
  }

public:
  explicit operator bool() const
  {
    return _is_valid;
  }

  template <typename ForwardIterator>
  void analysis(
      const ForwardIterator &it_begin, const ForwardIterator &it_end)
  {
    std::stack<symbol_t> symbols;
    std::stack<int> condition;
    symbols.push(delimiter_symbol());
    condition.push(0);
    

    auto match_or_output = [&](symbol_t terminate_symbol) {  
      while (!symbols.empty())
      {
        int id = condition.top();
        if (ACTION.count(std::make_pair(id, terminate_symbol.symbol_id)) == 0)
        {
          std :: cout << "error" << "\n";
        }
        else {
          auto op = ACTION[std::make_pair(id, terminate_symbol.symbol_id)];
          if (op.first[0] == 's') {
            std :: cout << "s" << " " << op.second << "\n";
            condition.push(op.second);
            symbols.push(terminate_symbol);
          }
          else {
            std::cout << "r" << " "  << op.second << "\n";
            condition.pop();
            int rule_id = op.second;
            for(int i = 0;i < id2rule[rule_id].rule_symbols.size();i++) symbols.pop();
            symbols.push(id2rule[rule_id].symbol);
            if (GOTO.count(std::make_pair(condition.top(), symbols.top().symbol_id)) == 0)
            {
              std::cout << "error" << "\n";
            }
            else {
              condition.push(GOTO[std::make_pair(condition.top(), symbols.top().symbol_id)]);
            }
          }
        }
      }
    };

    for (auto it = it_begin; it < it_end; ++it)
    {
      match_or_output(*it);
    }
    match_or_output(delimiter_symbol());
  }

private:
  bool _is_valid;
  syntax_t _syntax;
  symbol_t _start_symbol;

  std::unordered_map<symbol_t, std::unordered_set<symbol_t>> _first_set;
  std::unordered_map<int, production_rule_t> id2rule;
  std::unordered_map<production_rule_t, int> rule2id;
  std::unordered_map<int, std::vector<std::pair<int, std::pair<int, std::string>>>> project, _project;
  std::unordered_map<std::pair<int, std::string>, std::pair<std::string, int>> ACTION;
  std::unordered_map<std::pair<int, std::string>, int> GOTO;
};
} // namespace compiler

#endif // COMPILER_SYNTAX_ANALYSIS_HPP