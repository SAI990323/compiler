#include <utils/show/graphShow.hpp>

int main()
{
  utils::show::graphShow graphShow;
  graphShow.load("./assets/dfa/dfa_define.txt");
  graphShow.show();
}