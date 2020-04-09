#include <iostream>

#include <utils/io/smart_ifstream.hpp>

int main(int argc, char* argv[]) {
  using ifstream = utils::io::smart_ifstream;
  ifstream in_stream(argv[1]);
  std::size_t num_states, num_finalize_states;
  in_stream >> num_states >> num_finalize_states;
  std::cout << num_states << " " << num_finalize_states << "\n";
  while (num_finalize_states--) {
    std::size_t idx_state;
    std::string token_id;
    in_stream >> idx_state >> token_id;
    std::cout << idx_state << " " << token_id << "\n";
  }
  {
    std::size_t idx_state_in, idx_state_out;
    std::string parameter;
    while (in_stream >> idx_state_in >> idx_state_out >> parameter) {
      std::cout << idx_state_in << " -> " << idx_state_out << " " << parameter << "\n";
    }
  }
}