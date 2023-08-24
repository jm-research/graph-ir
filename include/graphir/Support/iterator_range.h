#ifndef GRAPHIR_SUPPORT_ITERATOR_RANGE_H
#define GRAPHIR_SUPPORT_ITERATOR_RANGE_H

#include <iterator>
#include <utility>

#include "graphir/Support/STLExtras.h"

namespace graphir {

template <typename IteratorT>
class iterator_range {
  IteratorT begin_iterator, end_iterator;

 public:
  template <typename Container>
  iterator_range(Container&& c)
      : begin_iterator(c.begin()), end_iterator(c.end()) {}
  iterator_range(IteratorT begin_iterator, IteratorT end_iterator)
      : begin_iterator(std::move(begin_iterator)),
        end_iterator(std::move(end_iterator)) {}

  IteratorT begin() const { return begin_iterator; }
  IteratorT end() const { return end_iterator; }
};

template <class T>
iterator_range<T> make_range(T x, T y) {
  return iterator_range<T>(std::move(x), std::move(y));
}

template <typename T>
iterator_range<T> make_range(std::pair<T, T> p) {
  return iterator_range<T>(std::move(p.first), std::move(p.second));
}

template <typename T>
iterator_range<decltype(adl_begin(std::declval<T>()))> drop_begin(T&& t,
                                                                  int n) {
  return make_range(std::next(adl_begin(t), n), adl_end(t));
}

}  // namespace graphir

#endif  // GRAPHIR_SUPPORT_ITERATOR_RANGE_H