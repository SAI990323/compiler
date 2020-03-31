#include <utils/show/graphShow.hpp>

int main() {
    utils::show::graphShow graphShow;
    graphShow.load("./assets/dfa/nfa_define.txt");
    graphShow.nfa_to_dfa();
    graphShow.dfa_show();
}