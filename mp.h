#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace mp {

/// Contains a list of types
template<class... T>
struct list {};

/// Identity metafunction
template<class T>
using id = T;

/// Bool metatype
template<bool V>
using bool_mv = std::integral_constant<bool, V>;

/// True metavalue
using true_ = bool_mv<true>;

/// False metavalue
using false_ = bool_mv<false>;

/// Size metatype
template<std::size_t V>
using size_mv = std::integral_constant<std::size_t, V>;

/// Casts a metavalue from one type to another
template<class VT, class V>
using mv_cast = std::integral_constant<VT, static_cast<VT>(V::value)>;

/// Lifts a function to be a metafunction
template <auto F>
struct lift {
    template <class... T>
    static constexpr auto value = F(T::value...);

    template <class... T>
    using type = std::integral_constant<decltype(F(T::value...)), value<T...>>;
};

/// Constant metavalue metafunction
template<class V>
struct constant_fn {
    template<class...>
    using type = V;
};

/// True metafunction
template<class...>
using true_fn = true_;

/// False metafunction
template<class...>
using false_fn = false_;

namespace detail {

template<class S>
struct sequence_to_list_impl;

template<template<class T, T...> class ST, class IT, IT... I>
struct sequence_to_list_impl<ST<IT, I...>> {
    using type = list<std::integral_constant<IT, I>...>;
};

} // namespace detail

/// Convert a value sequence to a metavalue sequence
template<class S>
using sequence_to_list = typename detail::sequence_to_list_impl<S>::type;

namespace detail {

template<class L>
struct list_to_sequence_impl;

template<template<class...> class LT, class T1, class... T>
struct list_to_sequence_impl<LT<T1, T...>> {
    using element_type = typename T1::value_type;
    static_assert((true && ... && std::is_same_v<typename T::value_type, element_type>));
    using type = std::integer_sequence<element_type, T1::value, T::value...>;
};

template<template<class...> class LT>
struct list_to_sequence_impl<LT<>> {
    using type = std::integer_sequence<size_t>;
};

} // namespace detail

/// Convert a metavalue sequence to a value sequence
template<class L>
using list_to_sequence = typename detail::list_to_sequence_impl<L>::type;

namespace detail {

template<template<class...> class F, class L>
struct invoke_impl;

template<template<class...> class F, template<class...> class LT, class... T>
struct invoke_impl<F, LT<T...>> {
    using type = F<T...>;
};

} // namespace detail

/// Invokes metafunction F where the arguments are the members of list L
template<template<class...> class F, class L>
using invoke = typename detail::invoke_impl<F, L>::type;

template<template<class...> class F, class... A>
struct bind {
    template<class... T>
    struct type : F<A..., T...> {};
};

/// Metafunction that returns the number of arguments it has
template<class... T>
using arg_count = size_mv<sizeof...(T)>;

/// Metafunction that returns the size of list L
template<class L> 
using length = invoke<arg_count, L>;

/// Metafunction that returns the size of list L as a value
template<class L> 
constexpr size_t length_v = length<L>::value;

namespace detail {

template<class... L>
struct concat_impl;

template<>
struct concat_impl<> {
    using type = list<>;
};

template<class L>
struct concat_impl<L> {
    using type = L;
};

template<template<class...> class LT, class... T1, class... T2, class... Ls>
struct concat_impl<LT<T1...>, LT<T2...>, Ls...> {
    using type = typename concat_impl<LT<T1..., T2...>, Ls...>::type;
};

template<template<class...> class LT, class... T1, class... T2, class... T3, class... T4, class... T5, class... Ls>
struct concat_impl<LT<T1...>, LT<T2...>, LT<T3...>, LT<T4...>, LT<T5...>, Ls...> {
    using type = typename concat_impl<LT<T1..., T2..., T3..., T4..., T5...>, Ls...>::type;
};

} // namespace detail

/// Concatenate lists together
template<class... L>
using concat = typename detail::concat_impl<L...>::type;

namespace detail {

template<template<class...> class F, class L>
struct map_impl;

template<template<class...> class F, template<class...> class LT, class... T>
struct map_impl<F, LT<T...>> {
    using type = LT<F<T>...>;
};

} // namespace detail

/// Metafunction that applies each element of list L to metafunction F
template<template<class...> class F, class L>
using map = typename detail::map_impl<F, L>::type;

namespace detail {

template<class L> struct carcdr_impl;

template<template<class...> class LT, class T1, class... T> 
struct carcdr_impl<LT<T1, T...>> {
    using car = T1;
    using cdr = LT<T...>;
};

} // namespace detail

/// Metafunction that returns the first type of list L
template<class L>
using car = typename detail::carcdr_impl<L>::car;

/// Metafunction that returns all of L except the first type
template<class L>
using cdr = typename detail::carcdr_impl<L>::cdr;

/// Metafunction that returns the I-th type of L
template<std::size_t I, class L>
using get = typename std::tuple_element<I, invoke<std::tuple, L>>::type;

namespace detail {

template <class L, class V>
struct count_impl;

template <template<class...> class LT, class... Ts, class T>
struct count_impl<LT<Ts...>, T> {
    using type = size_mv<(0 + ... + (std::is_same_v<Ts, T> ? 1 : 0))>;
};

} // namespace detail

/// Metafunction that returns the number of occurances of T in list L
template <class L, class T>
using count = typename detail::count_impl<L, T>::type;

/// Metafunction that returns the number of occurances of T in list L
template <class L, class T>
constexpr std::size_t count_v = count<L, T>::value;

namespace detail {

template <class L, class V>
struct contains_impl;

template <template<class...> class LT, class... Ts, class T>
struct contains_impl<LT<Ts...>, T> {
    static constexpr bool value = (false || ... || std::is_same_v<Ts, T>);
};

} // namespace detail

/// Metafunction that returns true if list L contains type T
template <class L, class... T>
constexpr bool contains_v = (true && ... && detail::contains_impl<L, T>::value);

/// Metafunction that returns true if list L contains type T
template <class L, class... T>
using contains = bool_mv<contains_v<L, T...>>;

/// Metafunction that returns true if all elements of list L are type T
template <class L, class T>
constexpr bool all_are_v = length<L>::value == count<L, T>::value;

/// Metafunction that returns true if all elements of list L are type T
template <class L, class T>
using all_are = bool_mv<all_are_v<L, T>>;

namespace detail {

template <class V>
constexpr bool value_equals(const typename V::value_type& v) {
    return v == V::value;
}

}

/// Metafunction that returns true if all elements of list L are type T
template <class L, class V>
constexpr bool all_values_are_v = all_are_v<map<lift<detail::value_equals<V>>::template type, L>, true_>;

/// Metafunction that returns true if all elements of list L are type T
template <class L, class V>
using all_values_are = bool_mv<all_values_are_v<L, V>>;

namespace detail {

template<typename V>
using to_bool_mv = bool_mv<static_cast<bool>(V::value)>;

} // namespace detail

/// Metafunction that returns true if predicate F returns true for all elements of list L
template <class L, template<class> class F>
constexpr bool all_of_v = all_values_are_v<map<F, L>, true_>;

/// Metafunction that returns true if predicate F returns true for all elements of list L
template <class L, template<class> class F>
using all_of = bool_mv<all_of_v<L, F>>;

/// Metafunction that returns true if all of the elements in list L are unique
template <class L>
constexpr bool is_a_set_v = all_values_are_v<map<bind<count, L>::template type, L>, size_mv<1>>;

/// Metafunction that returns true if all of the elements in list L are unique
template <class L>
using is_a_set = bool_mv<is_a_set_v<L>>;

} // namespace mp
