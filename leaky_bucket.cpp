#include <chrono>
#include <functional>
#include <iostream>

template <typename FunctionType, typename Rate> class RateLimiterClass;

template <typename ReturnType, typename Obj, typename Rate, typename... Args>
class RateLimiterClass<ReturnType (Obj::*)(Args...), Rate> {
private:
  Rate rate;
  Rate tokens;
  std::chrono::time_point<std::chrono::system_clock> last_timestamp;
  std::function<ReturnType(Args...)> func;

public:
  RateLimiterClass(Obj cl, ReturnType (Obj::*member)(Args...), Rate rate)
      : rate(rate), tokens(rate), func{[=](Args... as) mutable {
          return (cl.*member)(as...);
        }} {
    last_timestamp = std::chrono::system_clock::now();
  }

  ReturnType operator()(Args... args) {
    std::chrono::time_point<std::chrono::system_clock> timestamp =
        std::chrono::system_clock::now();
    auto duration = timestamp - last_timestamp;
    last_timestamp = timestamp;

    // If we haven't been invoked in awhile, add an appropriate number of tokens
    // to the bucket
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    tokens += static_cast<Rate>(seconds.count() * rate);

    if (tokens > rate) {
      tokens =
          rate; // ensure that we never have more than rate tokens in the bucket
    }

    if (tokens < (Rate)1) {
      // drop call
      return ReturnType{};
    } else {
      tokens -= (Rate)1;
      return func(args...);
    }
  }
};

template <typename FunctionType, typename Rate, FunctionType func>
struct RateLimiterBare;
template <typename ReturnType, typename Rate, typename... Args,
          ReturnType (*func)(Args...)>
struct RateLimiterBare<ReturnType (*)(Args...), Rate, func> {
private:
  Rate rate;
  Rate tokens;
  std::chrono::time_point<std::chrono::system_clock> last_timestamp;

public:
  RateLimiterBare(Rate rate) : rate(rate), tokens(rate) {
    last_timestamp = std::chrono::system_clock::now();
  };

  ReturnType operator()(Args... args) {
    std::chrono::time_point<std::chrono::system_clock> timestamp =
        std::chrono::system_clock::now();
    auto duration = timestamp - last_timestamp;
    last_timestamp = timestamp;

    // If we haven't been invoked in awhile, add an appropriate number of tokens
    // to the bucket
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    tokens += static_cast<Rate>(seconds.count() * rate);

    if (tokens > rate) {
      tokens =
          rate; // ensure that we never have more than rate tokens in the bucket
    }

    if (tokens < (Rate)1) {
      // drop call
      return ReturnType{};
    } else {
      tokens -= (Rate)1;
      return func(args...);
    }
  }
};

#define MAKE_WRAPPER_CLASS(cl, func, rate)                                     \
  RateLimiterClass<decltype(func), decltype(rate)> { cl, func, rate }

#define MAKE_WRAPPER(func, rate)                                               \
  RateLimiterBare<decltype(&func), decltype(rate), func> { rate }

struct foo {
  long long bar(int a, long b, long long c) {
    std::cout << "bar called" << std::endl;
    return a + b + c;
  }
};

int foobar(int a) {
  std::cout << "foo called" << std::endl;
  return a;
}

int main() {
  foo f;
  float rate = 4.0;

  auto tmp = MAKE_WRAPPER(foobar, rate);
  auto tmp2 = MAKE_WRAPPER_CLASS(f, &foo::bar, rate);

  tmp(3);
  tmp(3);
  tmp(3);
  tmp(3);
  tmp(3);
  tmp(3);
  tmp(3);
  tmp2(1, 2l, 3ll);
  tmp2(1, 2l, 3ll);
  tmp2(1, 2l, 3ll);
  tmp2(1, 2l, 3ll);
  tmp2(1, 2l, 3ll);
  tmp2(1, 2l, 3ll);
  tmp2(1, 2l, 3ll);
  tmp2(1, 2l, 3ll);
  tmp2(1, 2l, 3ll);
  tmp2(1, 2l, 3ll);
  tmp2(1, 2l, 3ll);
  tmp2(1, 2l, 3ll);
  tmp2(1, 2l, 3ll);
  std::cout << tmp(3) << std::endl;
  std::cout << tmp2(1, 2l, 3ll) << std::endl;
}