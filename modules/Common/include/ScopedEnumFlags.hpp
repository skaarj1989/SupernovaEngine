#pragma once

#include <type_traits>

/*
enum class TestFlags { None = 0, A = 1 << 0, B = 1 << 1, };
template <> struct has_flags<TestFlags> : std::true_type {};
*/

// The most common operations:
//
// TestFlags flags{TestFlags::A};
// flags |= TestFlags::B;                   // Add a flag.
// flags |= (TestFlags::A | TestFlags::B);  // Add multiple flags.
// flags &= ~TestFlags::A;                  // Remove a flag.
// flags &= ~(TestFlags::A | TestFlags::B); // Remove multiple flags.
//
// bool(flags & TestFlags::A);              // Check if a flag is set.
// flags == (TestFlags::A | TestFlags::B);  // Check the exact set of flags.

template <typename T> struct has_flags : std::false_type {};
template <typename T> constexpr auto has_flags_v = has_flags<T>::value;

template <typename T>
concept ScopedEnumWithFlags = std::is_scoped_enum_v<T> && has_flags_v<T>;

template <ScopedEnumWithFlags T> [[nodiscard]] constexpr auto operator~(T a) {
  return static_cast<T>(~static_cast<std::underlying_type_t<T>>(a));
}

template <ScopedEnumWithFlags T>
[[nodiscard]] constexpr auto operator|(T lhs, T rhs) {
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) |
                        static_cast<std::underlying_type_t<T>>(rhs));
}
template <ScopedEnumWithFlags T> inline T &operator|=(T &lhs, T rhs) {
  return lhs = lhs | rhs;
}

template <ScopedEnumWithFlags T>
[[nodiscard]] constexpr auto operator&(T lhs, T rhs) {
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) &
                        static_cast<std::underlying_type_t<T>>(rhs));
}
template <ScopedEnumWithFlags T> inline T &operator&=(T &lhs, T rhs) {
  return lhs = lhs & rhs;
}
