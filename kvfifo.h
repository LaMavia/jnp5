#ifndef KVFIFO_H
#define KVFIFO_H

#include <list>
#include <map>
#include <memory>
#include <stdexcept>

#define RETHROW(X)                                                             \
  try {                                                                        \
    X;                                                                         \
  } catch (...) {                                                              \
    throw;                                                                     \
  }

template <typename K, typename V> class kvfifo {
private:
  using list_ptr_t = typename std::list<std::pair<K, V>>::iterator;
  using map_t = std::map<K, std::list<list_ptr_t>>;
  using list_t = std::list<std::pair<K, V>>;

  // map of lists of pointers to values of the same key
  std::shared_ptr<map_t> A;
  // list of pairs <Key, Value>
  std::shared_ptr<list_t> B;
  bool must_copy;

  inline void copy() {
    if (must_copy || !A.unique() || !B.unique()) {
      kvfifo new_this{};
      for (const auto &[key, val] : *B)
        new_this.push(key, val);
      *this = new_this;
    }
  }

public:
  class k_iterator {
  private:
    typename map_t::const_iterator it;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = const K;
    using difference_type = ptrdiff_t;
    using pointer = const K *;
    using reference = const K &;

    inline k_iterator() = default;
    inline k_iterator(K it) : it(it) {}
    inline k_iterator(const k_iterator &other) : it(other.it) {}
    inline k_iterator(typename map_t::const_iterator &&it) : it(it) {}

    inline k_iterator &operator++() noexcept {
      ++it;
      return *this;
    }

    inline k_iterator operator++(int) noexcept {
      auto prev = *this;
      ++*this;
      return prev;
    }

    inline k_iterator &operator--() noexcept {
      --it;
      return *this;
    }

    inline k_iterator operator--(int) noexcept {
      auto prev = *this;
      --*this;
      return prev;
    }

    inline bool operator==(const k_iterator &other) const noexcept {
      return it == other.it;
    }

    inline bool operator!=(const k_iterator &other) const noexcept {
      return !this->operator==(other);
    }

    inline k_iterator &operator=(const k_iterator &other) noexcept = default;

    inline reference operator*() const noexcept { return (*it).first; }
    inline pointer operator->() const noexcept { return &(it->first); }
  };

  inline kvfifo()
      : A(std::make_shared<map_t>()), B(std::make_shared<list_t>()),
        must_copy(false) {}
  inline kvfifo(const kvfifo &other)
      : A(other.A), B(other.B), must_copy(other.must_copy) {
    if (must_copy) {
      RETHROW({ copy(); });
    }
  }
  inline kvfifo(kvfifo &&other) : must_copy(other.must_copy) {
    A.swap(other.A);
    B.swap(other.B);

    if (must_copy) {
      RETHROW(copy());
    }
  };

  inline kvfifo &operator=(kvfifo other) {
    A.swap(other.A);
    B.swap(other.B);
    must_copy = other.must_copy;

    if (must_copy) {
      RETHROW(copy());
    }

    return *this;
  };

  inline void push(const K &key, const V &val) {
    RETHROW(copy());

    bool do_pop_back = false;

    try {
      B->emplace_back(key, val);
      do_pop_back = true;
      (*A)[key].emplace_back(std::prev(B->end()));
    } catch (...) {
      if (do_pop_back) {
        B->pop_back();
        if (A->contains(key) && (*A)[key].empty())
          A->erase(key);
      }
      throw;
    }
  }

  inline void pop() {
    if (B->empty())
      throw std::invalid_argument("kvfifo: empty");

    RETHROW(copy());

    auto key = B->front().first;
    B->pop_front();

    auto &bucket = (*A)[key];
    bucket.pop_front();
    if (bucket.empty())
      A->erase(key);
  }

  inline void pop(const K &key) {
    if (!A->contains(key))
      throw std::invalid_argument("kvfifo: key not found");

    RETHROW(copy());

    auto &bucket = (*A)[key];
    B->erase(bucket.front());
    bucket.pop_front();
  }

  inline void move_to_back(const K &key) {
    if (!A->contains(key))
      throw std::invalid_argument("kvfifo: key not found");

    RETHROW(copy());

    auto &bucket = (*A)[key];
    for (auto it : bucket)
      B->splice(B->end(), *B, it);
  }

  inline std::pair<const K &, V &> front() {
    if (B->empty())
      throw std::invalid_argument("kvfifo: empty");

    RETHROW(copy());

    auto &[key, val] = B->front();
    must_copy = true;
    return {key, val};
  }

  inline std::pair<const K &, const V &> front() const {
    if (B->empty())
      throw std::invalid_argument("kvfifo: empty");

    auto &[key, val] = B->front();
    return {key, val};
  }
  inline std::pair<const K &, V &> back() {
    if (B->empty())
      throw std::invalid_argument("kvfifo: empty");

    RETHROW(copy());

    auto &[key, val] = B->back();
    must_copy = true;
    return {key, val};
  }

  inline std::pair<const K &, const V &> back() const {
    if (B->empty())
      throw std::invalid_argument("kvfifo: empty");

    auto &[key, val] = B->back();
    return {key, val};
  }

  inline std::pair<const K &, V &> first(const K &key) {
    if (!A->contains(key))
      throw std::invalid_argument("kvfifo: key not found");

    RETHROW(copy());

    auto &it = (*A)[key].front();
    auto &val = it->second;
    must_copy = true;
    return {key, val};
  }

  inline std::pair<const K &, const V &> first(const K &key) const {
    if (!A->contains(key))
      throw std::invalid_argument("kvfifo: key not found");

    auto &it = A->find(key)->second.front();
    auto &val = it->second;
    return {key, val};
  }

  inline std::pair<const K &, V &> last(const K &key) {
    if (!A->contains(key))
      throw std::invalid_argument("kvfifo: key not found");

    RETHROW(copy());

    auto &it = (*A)[key].back();
    auto &val = it->second;
    must_copy = true;
    return {key, val};
  }

  inline std::pair<const K &, const V &> last(const K &key) const {
    if (!A->contains(key))
      throw std::invalid_argument("kvfifo: key not found");

    auto &it = A->find(key)->second.back();
    auto &val = it->second;
    return {key, val};
  }

  inline size_t size() const noexcept { return B->size(); }

  inline bool empty() const noexcept { return B->empty(); }

  inline size_t count(const K &key) const noexcept {
    return A->contains(key) ? A->find(key)->second.size() : 0;
  };

  inline void clear() {
    RETHROW({
      copy();
      A->clear();
      B->clear();
    });
  }

  inline k_iterator k_begin() const noexcept { return {A->begin()}; }
  inline k_iterator k_end() const noexcept { return {A->end()}; }
};

#endif // KVFIFO_H
