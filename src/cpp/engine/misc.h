// Copyright (c) 2015, Tamas Csala

#ifndef ENGINE_MISC_H
#define ENGINE_MISC_H

#include <memory>

namespace engine {

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>{new T(std::forward<Args>(args)...)};
}

template<typename T>
std::unique_ptr<T> make_unique() {
  return std::unique_ptr<T>{new T{}};
}

template<typename T>
constexpr T sqr(const T& x) {
  return x*x;
}

template<typename T>
constexpr T cube(const T& x) {
  return x*x*x;
}

template<typename T>
constexpr T clamp(const T& x, const T& min, const T& max) {
  // assert(min < max);
  if (x < min) { return min; }
  if (x > max) { return max; }
  return x;
}

constexpr double kEpsilon = 1e-5;

}  // namespace engine

#endif
