
#pragma once

#include <mgctx/common.hpp>
#include <mgbase/logger.hpp>

// Implementation on System V x86-64 ABI

namespace mgctx {
namespace untyped {

template <void (*Func)(transfer_t)>
inline context_t make_context(
    void* const             sp
,   const mgbase::size_t    //size
)
{
    typedef mgbase::int64_t i64;
    const auto mask = ~i64{0xF};
    
    const auto ctx =
        reinterpret_cast<i64*>(
            (reinterpret_cast<i64>(sp) & mask) - 16
        );
    
    // Set the restored RBP to zero.
    // This is just for debugging.
    *ctx = 0;
    
    i64 tmp;
    
    asm volatile (
        "leaq   1f(%%rip), %[tmp]\n\t"
        "movq   %[tmp], %[label]\n\t"
        
        "jmp    2f\n\t\n\t"
        
    "1:\n\t"
        // Pass the result as a parameter.
        "movq   %%rax, %%rdi\n\t"
        
        // Call the user-defined function.
        "call   %P[func]\n\t"
        
        "call   abort\n\t"
        
    "2:\n\t"
        
    :   // Output constraints
        [label] "+m" (*(ctx+1)),
        
        [tmp] "=r" (tmp)
        
    :   // Input constraints
        [func] "i" (Func)
        
    :   "cc"
    );
    
    return { reinterpret_cast<context_frame*>(ctx) };
}

#define SWAP_CTX_RDI(saved_rsp) \
        /* Save RSP to a certain callee-saved register (R15). */ \
        "movq   %%rsp, %%" saved_rsp "\n\t" \
        \
        /* Save the continuation's RIP using RAX. */ \
        "leaq   1f(%%rip), %%rax\n\t" \
        "movq   %%rax, -0x88(%%rsp)\n\t" \
        \
        /* Load the new stack pointer. */ \
        "movq   %%rdi, %%rsp\n\t" \
        \
        /* Create the address of the saved context. */ \
        "leaq   -0x90(%%" saved_rsp "), %%rdi \n\t" \
        \
        /* Save RBP to the saved context. */ \
        "movq   %%rbp, (%%rdi)\n\t"

#define CLOBBER_REGISTERS \
        /* Caller-saved registers */ \
        "%rcx", "%rdx", "%r8", "%r9", "%r10", "%r11", \
        /* Callee-saved registers except for RBP */ \
        "%rbx", "%r12", "%r13", "%r14", "%15"
        
        // Not listed in clobbers:
        //  RSI, RDI : Inputs.
        //  RAX      : Returned value.
        //  RSP, RBP : Saved & restored.

template <transfer_t (*Func)(context_t, void*)>
inline transfer_t save_context(
    void* const     sp
,   mgbase::size_t  //size
,   void* const     arg
)
{
    typedef mgbase::int64_t i64;
    const auto mask = ~i64{0xF};
    
    // Align the stack pointer.
    auto new_sp = reinterpret_cast<void*>(
        reinterpret_cast<i64>(sp) & mask
    );
    
    void* result;
    
    // Note: 128-byte red zone is also (implicitly) saved.
    
    asm volatile (
        // RDI = new_sp;
        // RSI = arg;
        
        SWAP_CTX_RDI("r15")
        
        // RBP is still preserved.
        
        // Call the user-defined function here.
        //  (RAX, RDX) = func(RDI, RSI);
        "callq  %P[func]\n\t"
        // "P" removes dollar ($) for an immediate.
        //  - https://gcc.gnu.org/ml/gcc-help/2010-08/msg00102.html
        //  - http://stackoverflow.com/questions/3467180/direct-call-using-gccs-inline-assembly
        
        // The function ordinarily finished.
        
        // Restore RSP from R15.
        // Subtract 0x80 to counteract the next instruction.
        "leaq   -0x80(%%r15), %%rsp\n\t"
        
    "1:\n\t"
        // Continuation starts here.
        
        // RSP is (new_sp - 0x80).
        // RBP is already restored.
        
        // Fix RSP.
        "addq   $0x80, %%rsp"
        
        // result = RAX;
        
    :   // Output constraints
        // RDI | Input: Restored RSP. / Output: Overwritten (but discarded).
        "+D" (new_sp) ,
        // RAX | Output: User-defined value from Func or the previous context.
        "=a" (result)
        
    :   // Input constraints
        // RSI | Input: User-defined value
        "S" (arg),
        [func] "i" (Func)
        
    :   "cc", "memory",
        CLOBBER_REGISTERS
    );
    
    return { result };
}

template <transfer_t (*Func)(context_t, void*)>
inline transfer_t swap_context(
    context_t   ctx
,   void*       arg
)
{
    void* result;
    
    // Note: 128-byte red zone is also (implicitly) saved.
    
    asm volatile (
        // RDI = ctx;
        // RSI = arg;
        
        SWAP_CTX_RDI("r15")
        
        // Restore RBP (May be pushed by the callee again.)
        "popq   %%rbp\n\t"
        
        // Call the user-defined function here.
        //  (RAX, RDX) = func(RDI, RSI);
        "jmp    %P[func]\n\t"
        
    "1:\n\t"
        // Continuation starts here.
        
        // RSP is (new_sp - 0x80).
        // RBP is already restored.
        
        // Fix RSP.
        "addq   $0x80, %%rsp\n\t"
        
        // result = RAX;
        
    :   // Output constraints
        // RDI | Input: Restored RSP. / Output: Overwritten (but discarded).
        "+D" (ctx) ,
        // RAX | Output: User-defined value from Func or the previous context.
        "=a" (result)
        
    :   // Input constraints
        // RSI | Input: User-defined value
        "S" (arg),
        [func] "i" (Func)
        
    :   "cc", "memory",
        CLOBBER_REGISTERS
    );
    
    return { result };
}

template <transfer_t (*Func)(void*)>
MGBASE_NORETURN
inline void restore_context(const context_t ctx, void* const arg)
{
    asm volatile (
        // Restore RSP.
        "movq   %[ctx], %%rsp\n\t"
        
        // Restore RBP (May be pushed by the callee again.)
        "popq   %%rbp\n\t"
        
        // Call the user-defined function.
        // The return address is the continuation.
        "jmp    %P[func]\n\t"
        
        // RAX is passed to the continuation.
        
    :   // No output constraints
        
    :   // Input constraints
        
        // Allow any register or memory operand.
        [ctx] "g" (ctx),
        "D" (arg) /* RDI */ ,
        
        [func] "i" (Func)
        
    :   // No clobbers
    );
    
    MGBASE_UNREACHABLE();
}

#undef SWAP_CTX_RDI
#undef CLOBBER_REGISTERS

} // namespace untyped
} // namespace mgctx

