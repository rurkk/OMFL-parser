#include "lib/parser.h"
#include <iostream>

int main(int, char**) {
  std::string data = R"(
        key1 = [1, true, 3.14, "ITMO", [1, 2, 3], ["a", "b", 28]])";

  std::filesystem::path path = "C:\\Users\\raulm\\labwork-6-rurkk\\bin\\fin";
  const auto root = omfl::parse(path);

  std::cout << root.Get("key1").AsInt() << ' ';
  std::cout << root.Get("asd")[0].AsString() << ' ';
  std::cout << root.Get("asd")[1].AsInt() << ' ';
  std::cout << root.Get("asd")[2][0].AsBool() << ' ';

  return 0;
}
