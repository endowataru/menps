
#pragma once

#if (__cplusplus >= 201103L)
    #define MGBASE_EXPLICIT_OPERATOR    explicit
    #define MGBASE_NOEXCEPT             noexcept
    #define MGBASE_OVERRIDE             override
    #define MGBASE_NULLPTR              nullptr
#else
    #define MGBASE_EXPLICIT_OPERATOR
    #define MGBASE_NOEXCEPT             throw()
    #define MGBASE_OVERRIDE
    #define MBBASE_NULLPTR              0
#endif

#if (__cplusplus >= 201403L)
    #define MGBASE_CONSTEXPR_CPP14  constexpr
#else
    #define MGBASE_CONSTEXPR_CPP14
#endif

#if (defined(__sparc))
    #define MGBASE_ARCH_SPARC
#endif

#if (defined(__i386__) || defined(__x86_64__))
    #define MGBASE_ARCH_INTEL
#endif


namespace mgbase {

#if (__cplusplus >= 201103L)

class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator = (const noncopyable) = delete;

protected:
    noncopyable() noexcept = default;
};

#else

class noncopyable {
private:
    noncopyable(const noncopyable&);
    noncopyable& operator = (const noncopyable);

protected:
    noncopyable() { }
};

#endif

}


