#ifndef KVFIFO_H
#define KVFIFO_H

#include <cstddef>
#include <list>
#include <map>
#include <stdexcept>
#include <utility>

template <typename K, typename V> class kvfifo {
private:
  using x = std::list<std::pair<K, V>>::iterator;

  class k_iterator {
    std::map<K, std::list<x>>::const_iterator it;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = const K;
    using difference_type = ptrdiff_t;
    using pointer = const K *;
    using reference = const K &;

    inline k_iterator() = default;
    inline k_iterator(K it) : it(it) {}
    inline k_iterator(const k_iterator &other) : it(other.it) {}
    inline k_iterator(std::map<K, std::list<x>>::const_iterator &&it)
        : it(it) {}

    inline k_iterator &operator++() {
      ++it;
      return *this;
    }

    inline k_iterator operator++(int) {
      auto prev = *this;
      ++it;
      return prev;
    }

    inline k_iterator &operator--() {
      --it;
      return *this;
    }

    inline k_iterator operator--(int) {
      auto prev = *this;
      --it;
      return prev;
    }

    inline bool operator==(const k_iterator &other) const {
      return it == other.it;
    }

    inline bool operator!=(const k_iterator &other) const {
      return it != other.it;
    }

    inline k_iterator &operator=(const k_iterator &other) = default;

    inline reference operator*() { return (*it).first; }
    inline pointer operator->() { return &(it->first); }
  };

  std::map<K, std::list<x>> A;
  std::list<std::pair<K, V>> B;
  bool is_a_copy;

  inline void copy() {
    if (is_a_copy) {
      kvfifo new_this{};
      for (const auto &[key, val] : B) {
        new_this.push(key, val);
      }

      *this = new_this;
    }
  }

public:
  inline kvfifo() : A({}), B({}), is_a_copy(false) {}
  inline kvfifo(kvfifo const &other)
      : A(other.A), B(other.B), is_a_copy(true) {}
  inline kvfifo(kvfifo &&other) {
    A.swap(other.A);
    B.swap(other.B);
    is_a_copy = other.is_a_copy;
  };

  inline kvfifo &operator=(kvfifo other) {
    A.swap(other.A);
    B.swap(other.B);
    is_a_copy = other.is_a_copy;

    return *this;
  };

  inline void push(K const &key, V const &val) {
    copy();

    B.emplace_back(key, val);
    A[key].push_back(--B.end());
  }

  inline void pop() {
    if (B.empty()) {
      throw std::invalid_argument("");
    }

    copy();

    auto key = B.front().first;
    B.pop_front();

    auto &bucket = A[key];
    bucket.pop_front();
    if (bucket.empty()) {
      A.erase(key);
    }
  }

  inline void pop(K const &key) {
    if (!A.contains(key)) {
      throw std::invalid_argument("");
    }

    copy();

    auto &bucket = A[key];
    B.erase(bucket.front());
    bucket.pop_front();
  }

  inline void move_to_back(K const &key) {
    if (!A.contains(key)) {
      throw std::invalid_argument("");
    }

    copy();

    auto &bucket = A[key];
    for (auto &it : bucket) {
      B.push_back(*it);
      B.erase(it);
      it = --B.end();
    }
  }

  inline std::pair<K const &, V &> front() {
    if (B.empty()) {
      throw std::invalid_argument("");
    }

    copy();

    auto &[key, val] = B.front();
    return {key, val};
  }

  inline std::pair<K const &, V const &> front() const {
    if (B.empty()) {
      throw std::invalid_argument("");
    }

    auto &[key, val] = B.front();
    return {key, val};
  }
  inline std::pair<K const &, V &> back() {
    if (B.empty()) {
      throw std::invalid_argument("");
    }

    copy();

    auto &[key, val] = B.back();
    return {key, val};
  }

  inline std::pair<K const &, V const &> back() const {
    if (B.empty()) {
      throw std::invalid_argument("");
    }

    auto &[key, val] = B.back();
    return {key, val};
  }

  inline std::pair<K const &, V &> first(K const &key) {
    if (!A.contains(key)) {
      throw std::invalid_argument("");
    }

    copy();

    auto &it = A[key].front();
    auto &val = it->second;
    return {key, val};
  }

  inline std::pair<K const &, V const &> first(K const &key) const {
    if (!A.contains(key)) {
      throw std::invalid_argument("");
    }

    auto &it = A.find(key)->second.front();
    auto &val = it->second;
    return {key, val};
  }

  inline std::pair<K const &, V &> last(K const &key) {
    if (!A.contains(key)) {
      throw std::invalid_argument("");
    }

    copy();

    auto &it = A[key].back();
    auto &val = it->second;
    return {key, val};
  }

  inline std::pair<K const &, V const &> last(K const &key) const {
    if (!A.contains(key)) {
      throw std::invalid_argument("");
    }

    auto &it = A.find(key)->second.back();
    auto &val = it->second;
    return {key, val};
  }

  inline size_t size() const noexcept { return B.size(); }

  inline bool empty() const noexcept { return B.empty(); }

  inline size_t count(K const &key) const noexcept {
    return A.contains(key) ? A.find(key)->second.size() : 0;
  };

  inline void clear() noexcept {
    copy();
    A.clear();
    B.clear();
  }

  inline k_iterator k_begin() const noexcept { return {A.begin()}; }
  inline k_iterator k_end() const noexcept { return {A.end()}; }
};

#endif /* KVFIFO_H */