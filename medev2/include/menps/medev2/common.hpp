
#pragma once

#include <menps/mefdn/lang.hpp>
#include <menps/medev2/config.h>

#define MEDEV2_EXPAND_PARAMS_1(i, X, XL, t0, a0)       XL(i, t0, a0)
#define MEDEV2_EXPAND_PARAMS_2(i, X, XL, t0, a0, ...)  X(i, t0, a0) MEDEV2_EXPAND_PARAMS_1(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_EXPAND_PARAMS_3(i, X, XL, t0, a0, ...)  X(i, t0, a0) MEDEV2_EXPAND_PARAMS_2(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_EXPAND_PARAMS_4(i, X, XL, t0, a0, ...)  X(i, t0, a0) MEDEV2_EXPAND_PARAMS_3(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_EXPAND_PARAMS_5(i, X, XL, t0, a0, ...)  X(i, t0, a0) MEDEV2_EXPAND_PARAMS_4(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_EXPAND_PARAMS_6(i, X, XL, t0, a0, ...)  X(i, t0, a0) MEDEV2_EXPAND_PARAMS_5(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_EXPAND_PARAMS_7(i, X, XL, t0, a0, ...)  X(i, t0, a0) MEDEV2_EXPAND_PARAMS_6(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_EXPAND_PARAMS_8(i, X, XL, t0, a0, ...)  X(i, t0, a0) MEDEV2_EXPAND_PARAMS_7(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_EXPAND_PARAMS_9(i, X, XL, t0, a0, ...)  X(i, t0, a0) MEDEV2_EXPAND_PARAMS_8(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_EXPAND_PARAMS_10(i, X, XL, t0, a0, ...) X(i, t0, a0) MEDEV2_EXPAND_PARAMS_9(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_EXPAND_PARAMS_11(i, X, XL, t0, a0, ...) X(i, t0, a0) MEDEV2_EXPAND_PARAMS_10(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_EXPAND_PARAMS_12(i, X, XL, t0, a0, ...) X(i, t0, a0) MEDEV2_EXPAND_PARAMS_11(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_EXPAND_PARAMS_13(i, X, XL, t0, a0, ...) X(i, t0, a0) MEDEV2_EXPAND_PARAMS_12(i+1, X, XL, __VA_ARGS__)

#define MEDEV2_EXPAND_PARAMS(X, XL, num, ...) \
    MEDEV2_EXPAND_PARAMS_ ## num(0, X, XL, __VA_ARGS__)


#define MEDEV2_EXPAND_PARAMS_TO_DECL_AUX(i, t, a)   t a;

#define MEDEV2_EXPAND_PARAMS_TO_DECL(num, ...) \
    MEDEV2_EXPAND_PARAMS( \
        MEDEV2_EXPAND_PARAMS_TO_DECL_AUX, \
        MEDEV2_EXPAND_PARAMS_TO_DECL_AUX, \
        num, __VA_ARGS__)


#define MEDEV2_EXPAND_PARAMS_TO_PARAMS_AUX(i, t, a)         t a,
#define MEDEV2_EXPAND_PARAMS_TO_PARAMS_AUX_LAST(i, t, a)    t a

#define MEDEV2_EXPAND_PARAMS_TO_PARAMS(num, ...) \
    MEDEV2_EXPAND_PARAMS( \
        MEDEV2_EXPAND_PARAMS_TO_PARAMS_AUX, \
        MEDEV2_EXPAND_PARAMS_TO_PARAMS_AUX_LAST, \
        num, __VA_ARGS__)


#define MEDEV2_EXPAND_PARAMS_TO_ARGS_AUX(i, t, a)       a,
#define MEDEV2_EXPAND_PARAMS_TO_ARGS_AUX_LAST(i, t, a)  a

#define MEDEV2_EXPAND_PARAMS_TO_ARGS(num, ...) \
    MEDEV2_EXPAND_PARAMS( \
        MEDEV2_EXPAND_PARAMS_TO_ARGS_AUX, \
        MEDEV2_EXPAND_PARAMS_TO_ARGS_AUX_LAST, \
        num, __VA_ARGS__)


#define MEDEV2_EXPAND_PARAMS_TO_P_DOT_ARGS_AUX(i, t, a)         p.a,
#define MEDEV2_EXPAND_PARAMS_TO_P_DOT_ARGS_AUX_LAST(i, t, a)    p.a

#define MEDEV2_EXPAND_PARAMS_TO_P_DOT_ARGS(num, ...) \
    MEDEV2_EXPAND_PARAMS( \
        MEDEV2_EXPAND_PARAMS_TO_P_DOT_ARGS_AUX, \
        MEDEV2_EXPAND_PARAMS_TO_P_DOT_ARGS_AUX_LAST, \
        num, __VA_ARGS__)


#define MEDEV2_EXPAND_PARAMS_TO_LOG_FMT_AUX(i, t, a)    #a ":{}\t"

#define MEDEV2_EXPAND_PARAMS_TO_LOG_FMT(num, ...) \
    MEDEV2_EXPAND_PARAMS( \
        MEDEV2_EXPAND_PARAMS_TO_LOG_FMT_AUX, \
        MEDEV2_EXPAND_PARAMS_TO_LOG_FMT_AUX, \
        num, __VA_ARGS__)


#define MEDEV2_EXPAND_PARAMS_TO_LOG_P_DOT_ARGS_AUX(i, t, a)        ::menps::mefdn::show_param((p.a)),
#define MEDEV2_EXPAND_PARAMS_TO_LOG_P_DOT_ARGS_AUX_LAST(i, t, a)   ::menps::mefdn::show_param((p.a))

#define MEDEV2_EXPAND_PARAMS_TO_LOG_P_DOT_ARGS(num, ...) \
    MEDEV2_EXPAND_PARAMS( \
        MEDEV2_EXPAND_PARAMS_TO_LOG_P_DOT_ARGS_AUX, \
        MEDEV2_EXPAND_PARAMS_TO_LOG_P_DOT_ARGS_AUX_LAST, \
        num, __VA_ARGS__)

