#ifndef TT_H
#define TT_H

#include "kvfifo.h"
#include <cassert>

namespace ttt {
  bool b = false;
class mv {
public:
  mv() = default;
  mv([[maybe_unused]] mv &&m) {
    if (b) {
      throw std::runtime_error{"move"};
    }
  }
  mv([[maybe_unused]] const mv &m) {
    if (b) {
      throw std::runtime_error{"copy"};
    }
  }

  mv &operator=([[maybe_unused]] mv &&m) {
    if (b) {
      throw std::runtime_error{"move assignment"};
    }
  }
  mv &operator=([[maybe_unused]] const mv &m) {
    if (b) {
      throw std::runtime_error{"copy assignment"};
    }
  }
};

void tt_main() {
  kvfifo<int, mv> q{};
  for (size_t i = 0; i < 10; i++) {
    q.push(i, {});
  }

  b = true;
  q.move_to_back(5);
}

} // namespace ttt

#endif