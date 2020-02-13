
#pragma once

#include <memory>
#include <utility>
#include <type_traits>
#include <tuple>
#include <exception>
#include <atomic>
#include <mutex>
#include <thread>
#include <functional>
#include <iterator>
#include <array>

// Macros

#if !defined(NDEBUG) && !defined(CMPTH_DEBUG)
    #define CMPTH_DEBUG
#endif

#if __cplusplus < 201103L
    #error "ComposableThreads requires C++11 or above."
#endif

#if __cplusplus >= 201402L
    #define CMPTH_USE_CXX14 1
#else
    #define CMPTH_USE_CXX14 0
#endif

#if __cplusplus >= 201703L
    #define CMPTH_USE_CXX17 1
#else
    #define CMPTH_USE_CXX17 0
#endif

#define CMPTH_PP_CAT(a, b)          CMPTH_PP_CAT_HELPER(a, b)
#define CMPTH_PP_CAT_HELPER(a, b)   a ## b

#define CMPTH_DEFINE_DERIVED(P) \
    private: \
        using derived_type = typename P::derived_type; \
        \
        /*constexpr*/ derived_type& derived() noexcept { \
            return static_cast<derived_type&>(*this); \
        } \
        constexpr const derived_type& derived() const noexcept { \
            return static_cast<const derived_type&>(*this); \
        }

#define CMPTH_UNREACHABLE() __builtin_unreachable()

#define CMPTH_CACHE_LINE_SIZE   64

#define CMPTH_LIKELY(x)     __builtin_expect(!!(x), 1)
#define CMPTH_UNLIKELY(x)   __builtin_expect(!!(x), 0)

#define CMPTH_NODISCARD     __attribute__((warn_unused_result))
#define CMPTH_MAYBE_UNUSED  __attribute__((unused))

#define CMPTH_NOINLINE      __attribute__((noinline))

namespace cmpth {

namespace fdn {

enum class byte : unsigned char {}; // added in C++17

using std::size_t;
using std::ptrdiff_t;
using std::int32_t;
using std::int64_t;
using std::uint32_t;
using std::uint64_t;
using std::uintptr_t;
using std::intptr_t;

using std::move;
using std::forward;
using std::swap;

using std::adopt_lock;
using std::adopt_lock_t;

using std::unique_ptr;

using std::terminate;

using std::memory_order_acq_rel;
using std::memory_order_release;
using std::memory_order_acquire;
using std::memory_order_relaxed;

using std::unique_lock;
using std::lock_guard;

using std::begin;
using std::end;

using std::ref;
using std::reference_wrapper;

using std::array;

// <type_traits>

// "identity" metafunction
// Proposed in P0887R1

template <typename T>
struct type_identity { using type = T; };

template <typename T>
using type_identity_t = typename type_identity<T>::type;


template <typename T>
using decay_t = typename std::decay<T>::type;

template <typename T>
using remove_reference_t = typename std::remove_reference<T>::type;

template <typename T>
using remove_extent_t = typename std::remove_extent<T>::type;

using std::is_trivially_destructible;
using std::is_integral;
using std::is_pointer;

template <bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <typename T>
using make_signed_t = typename std::make_signed<T>::type;


template <typename T, T... Is>
struct integer_sequence {
    using value_type = T;
    static constexpr fdn::size_t size() noexcept {
        return sizeof...(Is);
    }
};

template <fdn::size_t... Is>
using index_sequence = integer_sequence<fdn::size_t, Is...>;

namespace detail {

// See also: https://blog.galowicz.de/2016/06/24/integer_sequences_at_compile_time/

template <typename T, fdn::size_t... Is>
struct make_int_seq;

template <typename T, fdn::size_t I, fdn::size_t... Is>
struct make_int_seq<T, I, Is...>
    : make_int_seq<T, (I-1), (I-1), Is...> { };

template <typename T, fdn::size_t... Is>
struct make_int_seq<T, 0, Is...> {
    using type = integer_sequence<T, static_cast<T>(Is)...>;
};

} // namespace detail

template <typename T, T N>
using make_integer_sequence =
    typename detail::make_int_seq<T, static_cast<fdn::size_t>(N)>::type;

template <fdn::size_t N>
using make_index_sequence = make_integer_sequence<fdn::size_t, N>;


// <functional>

// invoke()
// See also: https://stackoverflow.com/questions/32918679/in-c-11-how-to-invoke-an-arbitrary-callable-object

#if CMPTH_USE_CXX17
using std::invoke;

#else
template <typename Func, typename... Args>
inline constexpr auto invoke(Func func, Args&&... args)
    -> decltype(fdn::ref(func)(fdn::forward<Args>(args)...))
{
    return fdn::ref(func)(fdn::forward<Args>(args)...);
}
#endif

// <tuple>

using std::tuple;
using std::make_tuple;
using std::forward_as_tuple;
using std::tuple_size;
using std::tuple_cat;
using std::get;

#if CMPTH_USE_CXX17
using std::apply;

#else
namespace detail {

template <typename Func, typename Tuple, fdn::size_t... Is>
inline constexpr auto apply_impl(Func&& func, Tuple&& t, fdn::index_sequence<Is...> /*unused*/)
    -> decltype(
        fdn::invoke(
            fdn::forward<Func>(func)
        ,   fdn::get<Is>(fdn::forward<Tuple>(t))...
        )
    )
{
    return fdn::invoke(
        fdn::forward<Func>(func)
    ,   fdn::get<Is>(fdn::forward<Tuple>(t))...
    );
}

} // namespace detail

template <typename Func, typename Tuple>
inline constexpr auto apply(Func&& func, Tuple&& t)
    -> decltype(
        detail::apply_impl(
            fdn::forward<Func>(func)
        ,   fdn::forward<Tuple>(t)
        ,   fdn::make_index_sequence<
                fdn::tuple_size<fdn::remove_reference_t<Tuple>>::value
            >{}
        )
    )
{
    return detail::apply_impl(
        fdn::forward<Func>(func)
    ,   fdn::forward<Tuple>(t)
    ,   fdn::make_index_sequence<
            fdn::tuple_size<fdn::remove_reference_t<Tuple>>::value
        >{}
    );
}
#endif


// Added functions

inline void* over_aligned_alloc(const fdn::size_t size, const fdn::size_t alignment) {
    void* ret = nullptr;
    if (posix_memalign(&ret, alignment, size) != 0) {
        throw std::bad_alloc{};
    }
    return ret;
}

inline void over_aligned_free(void* const ptr) {
    free(ptr);
}


template <typename T, typename... Args>
inline T* over_aligned_new(Args&&... args) {
    const auto p = over_aligned_alloc(sizeof(T), alignof(T));
    return new (p) T(fdn::forward<Args>(args)...);
}

template <typename T>
inline void over_aligned_delete(T* const ptr) {
    ptr->~T();
    over_aligned_free(ptr);
}


template <typename T>
inline T* over_aligned_new_array(const fdn::size_t num) {
    const auto alignment = alignof(T);
    T* p = nullptr;
    if (fdn::is_trivially_destructible<T>::value) {
        p = static_cast<T*>(
            over_aligned_alloc(sizeof(T)*num, alignment)
        );
    }
    else {
        // Allocate an extra memory to store the size.
        const auto p_ret = over_aligned_alloc(sizeof(T)*num + alignment, alignment);
        const auto p_size = static_cast<fdn::size_t*>(p_ret);
        *p_size = num;
        
        p = reinterpret_cast<T*>(
            static_cast<fdn::byte*>(p_ret) + alignment
        );
    }
    
    for (fdn::size_t i = 0; i < num; ++i) {
        new (&p[i]) T();
    }
    return p;
}

template <typename T>
inline void over_aligned_delete_array(T* const ptr) {
    const auto alignment = alignof(T);
    if (fdn::is_trivially_destructible<T>::value) {
        // We don't need to call the destructor.
        over_aligned_free(ptr);
    }
    else {
        const auto size_ptr =
            reinterpret_cast<fdn::size_t*>(
                reinterpret_cast<fdn::byte*>(ptr) - alignment
            );
        
        const auto size = *size_ptr;
        
        for (fdn::size_t i = 0; i < size; ++i) {
            ptr[i].~T();
        }
        
        over_aligned_free(size_ptr);
    }
}


template <typename T>
struct oa_delete {
    void operator() (T* const ptr) const {
        over_aligned_delete(ptr);
    }
};

template <typename T>
struct oa_delete<T []> {
    void operator() (T* const ptr) const {
        over_aligned_delete_array(ptr);
    }
};


template <typename T>
using oa_unique_ptr = unique_ptr<T, oa_delete<T>>;


template <typename T>
struct make_unique_helper { using single_type = T; };

template <typename T>
struct make_unique_helper<T []> { using array_type = T []; };

template <typename T, fdn::size_t N>
struct make_unique_helper<T [N]> { struct invalid_type; };


template <typename T, typename... Args>
inline unique_ptr<typename make_unique_helper<T>::single_type>
make_unique(Args&&... args)
{
    return unique_ptr<T>( new T(fdn::forward<Args>(args)...) );
}

template <typename T, typename... Args>
inline unique_ptr<typename make_unique_helper<T>::array_type>
make_unique(const fdn::size_t size)
{
    using element_type = remove_extent_t<T>;
    return unique_ptr<element_type []>(
        new element_type[size]() // value-initialization
    );
}

template <typename T, typename... Args>
inline unique_ptr<typename make_unique_helper<T>::invalid_type>
make_unique(Args&&...) = delete;

// Non-standard function to allocate an array without initialization
template <typename T>
inline unique_ptr<typename make_unique_helper<T>::array_type>
make_unique_uninitialized(const fdn::size_t size)
{
    using element_type = remove_extent_t<T>;
    
    return unique_ptr<T>(
        // No value-initialization here.
        new element_type[size]
    );
}


template <typename T, typename... Args>
inline oa_unique_ptr<typename make_unique_helper<T>::single_type>
make_oa_unique(Args&&... args)
{
    return oa_unique_ptr<typename make_unique_helper<T>::single_type>{
        over_aligned_new<T>(fdn::forward<Args>(args)...)
    };
}

template <typename T, typename... Args>
inline oa_unique_ptr<typename make_unique_helper<T>::array_type>
make_oa_unique(const fdn::size_t size)
{
    return oa_unique_ptr<typename make_unique_helper<T>::array_type>{
        over_aligned_new_array<remove_extent_t<T>>(size)
    };
}

template <typename T, typename... Args>
inline oa_unique_ptr<typename make_unique_helper<T>::invalid_type>
make_oa_unique(Args&&...) = delete;


inline constexpr fdn::size_t calc_padding(const fdn::size_t data_size) {
    return CMPTH_CACHE_LINE_SIZE - data_size % CMPTH_CACHE_LINE_SIZE;
}

template <typename T>
inline constexpr T roundup_divide(const T x, const T y) noexcept {
    static_assert(fdn::is_integral<T>::value, "T must be integer");
    return (x + y - 1) / y;
}


// See also: offsetof() macro in <stddef.h>

template <typename T, typename M>
inline constexpr fdn::ptrdiff_t get_offset_of(M T::* const mem)
{
    return reinterpret_cast<fdn::ptrdiff_t>(
        &(static_cast<T*>(nullptr)->*mem)
    );
}

// See also: container_of() macro in Linux kernel

template <typename T, typename M>
inline constexpr T* get_container_of(M* const p, M T::* const mem)
{
    return reinterpret_cast<T*>(
        reinterpret_cast<fdn::uintptr_t>(p) - get_offset_of(mem)
    );
}


inline void* align_call_stack(
    const fdn::size_t   alignment
,   const fdn::size_t   size
,   void*&              ptr
,   fdn::size_t&        space
) {
    using iptr_t = fdn::intptr_t;
    const auto salign = static_cast<iptr_t>(alignment);
    const auto ssize = static_cast<iptr_t>(size);
    const auto sptr = reinterpret_cast<iptr_t>(ptr);
    
    // Calculate the maximum aligned position less than (ptr-size).
    const auto aligned = (sptr - ssize) & -salign;
    
    const auto diff =
        static_cast<fdn::size_t>(sptr - aligned);
    
    if (CMPTH_UNLIKELY(diff > space)) {
        return nullptr;
    }
    
    space -= diff;
    const auto ret = reinterpret_cast<void*>(aligned);
    ptr = ret;
    return ret;
}


// Define as specified in C++17 (or Parallelism TS).

namespace execution {

struct sequenced_policy { };

struct parallel_policy { };

struct parallel_unsequenced_policy { };

} // namespace execution


// <exception>

using std::exception;


template <typename T, typename U>
inline constexpr typename fdn::enable_if_t<
    ! (fdn::is_pointer<T>::value || fdn::is_pointer<U>::value)
,   T
>
force_integer_cast(const U& value) noexcept {
    return static_cast<T>(value);
}

template <typename T, typename U>
inline constexpr typename fdn::enable_if_t<
    (fdn::is_pointer<T>::value || fdn::is_pointer<U>::value)
,   T
>
force_integer_cast(const U& value) noexcept {
    return reinterpret_cast<T>(value);
}

} // namespace fdn

} // namespace cmpth

