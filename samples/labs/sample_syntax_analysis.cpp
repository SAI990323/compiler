#include <iostream>
#include <string>
#include <vector>

#include <utils/io/smart_ifstream.hpp>
#include <compiler/syntax.hpp>
#include <compiler/syntax_analysis.hpp>

int main(int argc, char* argv[])
{
  using ifstream = utils::io::smart_ifstream;
  ifstream in_stream(argv[1]);
  std::vector<compiler::production_rule_t> rules;
  std::string start_symbol_id = "";
  {
    std::string s;
    std::string symbol;
    std::vector<std::string> rule;

label_symbol:
    if (in_stream >> s) {
      symbol = s;
      if (start_symbol_id == "") {
        start_symbol_id = s;
      }
      goto label_is_defined_as;
    } else {
      goto label_terminate;
    }

label_is_defined_as:
    in_stream >> s;
    goto label_rule_start;

label_rule_start:
    in_stream >> s;
    rule.push_back(s);
    goto label_rule_loop;

label_rule_loop:
    in_stream >> s;
    if (s == "|") {
      rules.emplace_back(symbol, rule.begin(), rule.end());
      rule.clear();
      goto label_rule_start;
    } else if (s == ";") {
      rules.emplace_back(symbol, rule.begin(), rule.end());
      rule.clear();
      goto label_symbol;
    } else {
      rule.push_back(s);
      goto label_rule_loop;
    }

label_terminate:
    ;
  }

  compiler::syntax_t syntax(rules.begin(), rules.end());

  std::cout << "terminate symbols:";
  for (auto&& symbol: syntax.terminate_symbols()) {
    std::cout << " " << symbol.symbol_id;
  }
  std::cout << "\n\n";

  std::cout << "non-terminate symbols (with rule):\n\n";
  for (auto&& symbol: syntax.non_terminate_symbols()) {
    std::cout << "  " << symbol.symbol_id << " ::=\n";
    bool first = true;
    for (auto&& rules: syntax.rules(symbol)) {
      std::cout << (first ? "     " : "    |");
      first = false;
      for (auto&& rule_symbol: rules.rule_symbols) {
        std::cout << " " << rule_symbol.symbol_id;
      }
      std::cout << "\n";
    }
    std::cout << "    ;\n\n";
  }
  std::cout << "\n";

  compiler::LL1_syntax_analyser_t analyser(
      syntax, compiler::symbol_t(start_symbol_id));

  for (auto&& symbol: analyser.non_terminate_symbols()) {
    std::cout << "FIRST(" << symbol << ") = { ";
    for (auto&& terminate_symbol: analyser.get_first_set(symbol)) {
      std::cout << terminate_symbol << ", ";
    }
    std::cout << "}\n";
    std::cout << "FOLLOW(" << symbol << ") = { ";
    for (auto&& terminate_symbol: analyser.get_follow_set(symbol)) {
      std::cout << terminate_symbol << ", ";
    }
    std::cout << "}\n\n";
  }

  for (auto&& [non_terminate_symbol, items]: analyser.get_predict_table()) {
    std::cout << non_terminate_symbol << "\n";
    for (auto&& [terminate_symbol, rule_set]: items) {
      std::cout << "  " << terminate_symbol << "\n";
      for (auto&& rule: rule_set) {
        std::cout << "    " << rule << "\n";
      }
    }
  }
  std::cout << "\n";

  return 0;
}