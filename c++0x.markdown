
# Target Compilers

- GCC 4.4.3
    - https://gcc.gnu.org/gcc-4.4/cxx0x_status.html
- Clang 3.1
    - http://clang.llvm.org/cxx_status.html
- Intel C++ Compiler 14.0
    - https://software.intel.com/en-us/articles/c0x-features-supported-by-intel-c-compiler
- PGI Compiler 2015

# Available Features in C++11

## Language

- `auto`, `decltype`
- Initializer lists
    - Be careful to use
    - Bugs in GCC 4.x
        - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55922
        - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53361#c6
- Variadic templates
    - Be careful to use
    - Bugs in GCC 4.x
        - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=45236
        - https://gcc.gnu.org/ml/gcc-bugs/2011-05/msg01489.html
- rvalue references
    - std::move was not supported in GCC 4.3
- `extern template`
- Defaulted and deleted functions
    - Move constructors/assignments cannot be defaulted in GCC 4.4
- Strongly-typed enums

### ICE in GCC 4.4

- `decltype`
    - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=47289

    template <typename T>
    T r(T x) { return x; }
    
    template <typename F, typename... T>
    inline auto f(F&& f, T&&... x) -> decltype(r(f)(x...)) { return f(x...); }
    
    int g(int);
    
    decltype(f(&g, 123)) x;

### Bug in GCC

- `using`
    - Fixed in GCC 4.7

    template <typename D>
    struct A { typedef int x; };
    
    template <typename D>
    struct B : A<B<D>> {
        using typename A<B<D>>::x;
        x f() { return 1; }
    };

## Library

- `<type_traits>`

# Unavailable Language Features in C++11

## Language

- Template aliases
- Inheriting constructors
- Delegating constructors
- `nullptr`
- Alignment support
- Explicit virtual overrides
- Template arguments with internal linkage
    - DR 1155

## Library


# Coding Rules

- Uniform initialization syntax
    - Zero argument
        - Always use {}
    - One reference
        - Use () for the old versions of GCC
    - One argument, not reference
        - Use () if we expect normal constructors
        - Use {} if we expect initializer_list<>
    - Multiple arguments
        - Use () if we expect normal constructors
        - Use {} if we expect initializer_list<>
    - Omitting return type
        - Use if the constructed type is aggregate

