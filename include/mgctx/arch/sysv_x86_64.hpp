
#pragma once

#include <mgctx/common.hpp>
#include <mgbase/logger.hpp>
#include <cstdlib>

// Implementation on System V x86-64 ABI

namespace mgctx {

struct context_frame
{
    void*   rbp;
    void*   rip;
    void*   rsp;
};

namespace detail {

// Separated to assembly
extern "C"
void mgctx_entrypoint();

} // namespace detail

template <typename T, void (*Func)(transfer<T*>)>
inline context<T*> make_context(
    void* const             sp
,   const mgbase::size_t    /*size*/
) {
    typedef mgbase::int64_t i64;
    const auto mask = ~i64{0xF};
    
    const auto ctx =
        reinterpret_cast<i64*>(
            (reinterpret_cast<i64>(sp) & mask) - 32
        );
    
    // Set the restored RBP to zero.
    // This is just for debugging.
    *ctx = 0;
    
    // Set the entrypoint function.
    *(ctx+1) = reinterpret_cast<i64>(&detail::mgctx_entrypoint);
    
    // Save the function pointer.
    *(ctx+2) = reinterpret_cast<i64>(Func);
    
    return { reinterpret_cast<context_frame*>(ctx) };
}

#define SAVE_CONTEXT(saved_rsp) \
        /* Save RSP to a certain callee-saved register (R15). */ \
        "movq   %%rsp, %%" saved_rsp "\n\t" \
        \
        /* Align RSP to follow 16-byte stack alignment. */ \
        /* The compiler doesn't guarantee the alignment of RSP at a certain code point. */ \
        /* restore_context's handler is executed on the top of this stack. */ \
        "andq   $-0x10, %%rsp\n\t" \
        \
        /* Save the red zone. */ \
        /* Preserve the alignment by subtracting additional 8-bytes. */ \
        "subq   $0x88, %%rsp\n\t" \
        \
        /* Save the original RSP. */ \
        "pushq  %%" saved_rsp "\n\t" \
        \
        /* Save the continuation's RIP using RAX. */ \
        "leaq   1f(%%rip), %%rax\n\t" \
        "pushq  %%rax\n\t" \
        \
        /* Save RBP to the saved context. */ \
        "pushq  %%rbp\n\t" \
        \
        /* Swap the stack pointers. */ \
        "xchg   %%rdi, %%rsp\n\t"

#define CLOBBER_REGISTERS \
        /* Caller-saved registers */ \
        "%rcx", "%rdx", "%r8", "%r9", "%r10", "%r11", \
        /* Callee-saved registers except for RBP */ \
        "%r12", "%r13", "%r14", "%r15"
        
        // Not listed in clobbers:
        //  RSI, RDI : Inputs.
        //  RAX      : Returned value.
        //  RSP, RBP : Saved & restored.
        //  RBX      : Function pointer.


template <typename T, typename Arg, transfer<T*> (*Func)(context<T*>, Arg*)>
inline transfer<T*> save_context(
    void* const             sp
,   const mgbase::size_t    /*size*/
,   Arg*                    arg
) {
    typedef mgbase::int64_t i64;
    const auto mask = ~i64{0xF};
    
    // Align the stack pointer.
    auto new_sp = (reinterpret_cast<i64>(sp) & mask) - 0x8;
    
    auto func = Func;
    
    transfer<T*> result;
    
    // Note: 128-byte red zone is also (implicitly) saved.
    
    asm volatile (
        // RDI = new_sp;
        // RSI = arg;
        
        SAVE_CONTEXT("r15")
        
        // Push the old RSP to the new context's stack.
        "pushq  %%r15\n\t"
        
        // RBP is still preserved.
        
        // Call the user-defined function here.
        //  (RAX, RDX) = func(RDI, RSI);
        "call   *%[func]\n\t"
        // "P" removes dollar ($) for an immediate.
        //  - https://gcc.gnu.org/ml/gcc-help/2010-08/msg00102.html
        //  - http://stackoverflow.com/questions/3467180/direct-call-using-gccs-inline-assembly
        
        // The function ordinarily finished.
        
    "1:\n\t"
        // Continuation starts here.
        
        // RSP is pointing to the saved RSP.
        // RBP is already restored.
        
        // Restore RSP from the stack.
        // It also skips the red zone.
        "popq   %%rsp"
        
        // result = RAX;
        
    :   // Output constraints
        // RDI | Input: Restored RSP. / Output: Overwritten (but discarded).
        "+D" (new_sp),
        // RAX | Output: User-defined value from Func or the previous context.
        "=a" (result.p0),
        // RSI | Input: User-defined value. / Output: Overwritten (but discarded).
        "+S" (arg),
        // RBX | Input: User-defined function. / Output: Overwritten (but discarded).
        [func] "+b" (func)
        
    :   // No input constraints
        
    :   "cc", "memory",
        CLOBBER_REGISTERS
    );
    
    return result;
}

template <typename T, typename Arg, transfer<T*> (*Func)(context<T*>, Arg*)>
inline transfer<T*> swap_context(
    context<T*> ctx
,   Arg*        arg
) {
    auto func = Func;
    
    transfer<T*> result;
    
    // Note: 128-byte red zone is also (implicitly) saved.
    
    asm volatile (
        // RDI = ctx;
        // RSI = arg;
        
        SAVE_CONTEXT("r15")
        
        // Restore RBP (May be pushed by the callee again.)
        "popq   %%rbp\n\t"
        
        // Call the user-defined function here.
        //  (RAX, RDX) = func(RDI, RSI);
        "jmp    *%[func]\n\t"
        
    "1:\n\t"
        // Continuation starts here.
        
        // RSP is pointing to the saved RSP.
        // RBP is already restored.
        
        // Restore RSP from the stack.
        // It also skips the red zone.
        "popq   %%rsp"
        
        // result = RAX;
        
    :   // Output constraints
        // RDI | Input: Restored RSP. / Output: Overwritten (but discarded).
        "+D" (ctx.p),
        // RAX | Output: User-defined value from Func or the previous context.
        "=a" (result.p0),
        // RSI | Input: User-defined value / Output: Overwritten (but discarded).
        "+S" (arg),
        // RBX | Input: User-defined function. / Output: Overwritten (but discarded).
        [func] "+b" (func)
        
    :   // No input constraints
        
    :   "cc", "memory",
        CLOBBER_REGISTERS
    );
    
    return result;
}

template <typename T, typename Arg, transfer<T*> (*Func)(Arg*)>
MGBASE_NORETURN
inline void restore_context(
    const context<T*>   ctx
,   Arg* const          arg
) {
    MGBASE_ASSERT(reinterpret_cast<mgbase::int64_t>(ctx.p) % 0x10 == 0);
    
    asm volatile (
        // Restore RSP.
        "movq   %[ctx], %%rsp\n\t"
        
        // Restore RBP (May be pushed by the callee again.)
        "popq   %%rbp\n\t"
        
        // Call the user-defined function.
        // The return address is the continuation.
        "jmp    *%[func]\n\t"
        
        // RAX is passed to the continuation.
        
    :   // No output constraints
        
    :   // Input constraints
        
        // Allow any register or memory operand.
        [ctx] "g" (ctx.p),
        // RDI | Input: User-defined value
        "D" (arg),
        
        [func] "b" (Func)
        
    :   // No clobbers
    );
    
    MGBASE_UNREACHABLE();
}

#undef SAVE_CONTEXT
#undef CLOBBER_REGISTERS

} // namespace mgctx

