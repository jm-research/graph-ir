#ifndef GRAPHIR_SUPPORT_HASH_H
#define GRAPHIR_SUPPORT_HASH_H

#include <functional>

namespace graphir {

template <typename T>
long long hash_combine(size_t const seed, T const& t) {
  static std::hash<T> hasher{};
  return static_cast<long long>(seed ^ hasher(t) + 0x9e3779b9 + (seed << 6) +
                                           (seed >> 2));
}

}  // namespace graphir

#endif  // GRAPHIR_SUPPORT_HASH_H