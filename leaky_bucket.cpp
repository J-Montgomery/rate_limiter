#include <functional>
#include <iostream>

template <typename FunctionType> class Rate_Limiter;

template <typename ReturnValue, typename Obj, typename... Args>
class Rate_Limiter<ReturnValue (Obj::*)(Args...)> {
private:
  std::function<ReturnValue(Args...)> func;

public:
  Rate_Limiter(Obj cl, ReturnValue (Obj::*member)(Args...))
      : func{[=](Args... as) mutable { return (cl.*member)(as...); }} {}

  ReturnValue operator()(Args... args) const { return func(args...); }
};

template <typename FunctionType, FunctionType func> struct RateLimiterClass;
template <typename ReturnType, typename Obj, typename... Args,
          ReturnType (*func)(Args...)>
struct RateLimiterClass<ReturnType (*)(Args...), func> {
  bool enabled{true};
  ReturnType operator()(Args... args) {
    if (enabled) {
      return func(args...);
    } else {
      return ReturnType{};
    }
  }
};
#define MAKE_WRAPPER(cl, func)                                                 \
  RateLimiterClass<decltype(&func), func> {}

template <typename FunctionType, FunctionType func> struct RateLimiterBare;
template <typename ReturnType, typename... Args, ReturnType (*func)(Args...)>
struct RateLimiterBare<ReturnType (*)(Args...), func> {
  bool enabled{true};
  ReturnType operator()(Args... args) {
    if (enabled) {
      return func(args...);
    } else {
      return ReturnType{};
    }
  }
};
#define MAKE_WRAPPER(func)                                                     \
  RateLimiterBare<decltype(&func), func> {}

struct foo {
  long long bar(int a, long b, long long c) { return a + b + c; }
};

int foobar(int a) { return a; }

int main() {
  foo f;

  Rate_Limiter<decltype(&foo::bar)> wfb{f, &foo::bar};
  auto tmp = MAKE_WRAPPER(foobar);

  std::cout << wfb(1, 2l, 3ll) << std::endl;
  std::cout << tmp(3) << std::endl;
}