#ifndef GRAPHIR_SUPPORT_STLEXTRAS_H
#define GRAPHIR_SUPPORT_STLEXTRAS_H

#include <algorithm>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace graphir {

template <class T>
struct unique_ptr_unwrapper {
  T* operator()(std::unique_ptr<T>& up) const { return up.get(); }
};

template <typename ContainerTy>
auto adl_begin(ContainerTy&& container)
    -> decltype(std::begin(std::forward<ContainerTy>(container))) {
  return std::begin(std::forward<ContainerTy>(container));
}

template <typename ContainerTy>
auto adl_end(ContainerTy&& container)
    -> decltype(std::end(std::forward<ContainerTy>(container))) {
  return std::end(std::forward<ContainerTy>(container));
}

template <typename R, typename T>
auto find(R&& range, const T& val) -> decltype(adl_begin(range)) {
  return std::find(adl_begin(range), adl_end(range), val);
}

template <typename R, typename UnaryPredicate>
auto find_if(R&& range, UnaryPredicate p) -> decltype(adl_begin(range)) {
  return std::find_if(adl_begin(range), adl_end(range), p);
}

template <typename R, typename UnaryPredicate>
auto find_if_not(R&& range, UnaryPredicate p) -> decltype(adl_begin(range)) {
  return std::find_if_not(adl_begin(range), adl_end(range), p);
}

template <class T, class... Args>
typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T>
typename std::enable_if<std::is_array<T>::value && std::extent<T>::value == 0,
                        std::unique_ptr<T>>::type
make_unique(size_t n) {
  return std::unique_ptr<T>(new typename std::remove_extent<T>::type[n]());
}

template <class T, class... Args>
typename std::enable_if<std::extent<T>::value != 0>::type make_unique(
    Args&&...) = delete;

template <typename Fn>
class function_ref;

template <typename Ret, typename... Params>
class function_ref<Ret(Params...)> {
  Ret (*callback)(intptr_t callable, Params... params) = nullptr;
  intptr_t callable;

  template <typename Callable>
  static Ret callback_fn(intptr_t callable, Params... params) {
    return (*reinterpret_cast<Callable*>(callable))(
        std::forward<Params>(params)...);
  }

 public:
  function_ref() = default;
  function_ref(std::nullptr_t) {}

  template <typename Callable>
  function_ref(Callable&& callable,
               typename std::enable_if<
                   !std::is_same<typename std::remove_reference<Callable>::type,
                                 function_ref>::value>::type* = nullptr)
      : callback(callback_fn<typename std::remove_reference<Callable>::type>),
        callable(reinterpret_cast<intptr_t>(&callable)) {}

  Ret operator()(Params... params) const {
    return callback(callable, std::forward<Params>(params)...);
  }

  operator bool() const { return callback; }
};

}  // namespace graphir

#endif  // GRAPHIR_SUPPORT_STLEXTRAS_H