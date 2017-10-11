
#pragma once

#include "integral_constant.hpp"

namespace mgbase {

// primary template
template <typename>
struct is_function : false_type { };
 
// specialization for regular functions
template <typename Result , typename... Args>
struct is_function<Result (Args...)> : true_type {};
 
// specialization for variadic functions such as printf
template <typename Result , typename... Args>
struct is_function<Result (Args..., ...)> : true_type {};
 
// specialization for function types that have cv-qualifiers
template <typename Result , typename... Args>
struct is_function<Result (Args...) const> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args...) volatile> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args...) const volatile> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args..., ...) const> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args..., ...) volatile> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args..., ...) const volatile> : true_type {};

#ifdef MGBASE_CXX11_REF_QUALIFIERS_SUPPORTED
 
// specialization for function types that have ref-qualifiers
template <typename Result , typename... Args>
struct is_function<Result (Args...) &> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args...) const &> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args...) volatile &> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args...) const volatile &> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args..., ...) &> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args..., ...) const &> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args..., ...) volatile &> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args..., ...) const volatile &> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args...) &&> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args...) const &&> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args...) volatile &&> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args...) const volatile &&> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args..., ...) &&> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args..., ...) const &&> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args..., ...) volatile &&> : true_type {};
template <typename Result , typename... Args>
struct is_function<Result (Args..., ...) const volatile &&> : true_type {};

#endif

} // namespace mgbase

