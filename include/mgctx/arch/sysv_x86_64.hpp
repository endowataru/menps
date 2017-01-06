
#pragma once

#include <mgctx/common.hpp>
#include <mgbase/logger.hpp>

// Implementation on System V x86-64 ABI

namespace mgctx {
namespace untyped {

namespace detail {

template <void (*Func)(transfer_t)>
MGBASE_NORETURN
inline void on_start(void* const sp, void* data)
{
    asm volatile (
        // A new context is restored.
        
        // Set the stack pointer and the stack base pointer.
        "movq   %%rdi, %%rsp\n\t"
        "movq   %%rdi, %%rbp\n\t"
        
    :   // Output constraints
        "+S" (data) /* RSI */
            // Because RBP is modified,
            // it must be reloaded from the register.
        
    :   // Input constraints
        "D"(sp) /* RDI */
        
    :   // No clobbers
    );
    
    Func({ data });
    
    abort();
}

} // namespace detail

template <void (*Func)(transfer_t)>
inline context_t make_context(
    void* const             sp
,   const mgbase::size_t    size
)
{
    typedef mgbase::int64_t    i64;
    
    const i64 mask = ~0xF;
    
    const auto ctx =
        reinterpret_cast<i64*>(
            (reinterpret_cast<i64>(sp) & mask) - 16
        );
    
    const auto new_ip =
        reinterpret_cast<i64>(
            &detail::on_start<Func>
        );
    
    // Assign the label.
    *ctx = new_ip;
    
    return { reinterpret_cast<context_frame*>(ctx) };
}

template <transfer_t (*Func)(context_t, void*)>
inline transfer_t save_context(
    void* const     sp
,   mgbase::size_t  //size
,   void* const     arg
)
{
    typedef mgbase::uintptr_t   uip;
    
    const auto mask = ~uip{0xF};
    
    // Align the stack pointer.
    auto new_sp = reinterpret_cast<void*>(
        reinterpret_cast<uip>(sp) & mask
    );
    
    void* result;
    
    // Note: 128-byte red zone is also (implicitly) saved.
    
    asm volatile (
        // RDI = new_sp;
        // RSI = arg;
        
        // Save RBP at the location which is not in the red zone.
        "movq   %%rbp, -0x88(%%rsp)\n\t"
        
        // Save RSP to a callee-saved register (RBP).
        "movq   %%rsp, %%rbp\n\t"
        
        // Move to a new call stack.
        "movq   %%rdi, %%rsp\n\t"
        
        // Calculate the address of the saved context.
        "leaq   -0x90(%%rbp), %%rdi\n\t"
        
        // Save the old context's RIP using RAX.
        //  There is no special meaning to use RAX.
        //  Both callee- or caller-saved registers are OK.
        "leaq   1f(%%rip), %%rax\n\t"
        "movq   %%rax, (%%rdi)\n\t"
        
        // Place the return address.
        "leaq   2f(%%rip), %%rdx\n\t"
        "movq   %%rdx, (%%rsp)\n\t"
        
        // Call the user-defined function here.
        //  (RAX, RDX) = func(RDI, RSI);
        "jmp    %P[func]\n\t"
        // "P" removes dollar ($) for an immediate.
        //  - https://gcc.gnu.org/ml/gcc-help/2010-08/msg00102.html
        //  - http://stackoverflow.com/questions/3467180/direct-call-using-gccs-inline-assembly
        
    "1:\n\t"
        // Saved context is restored.
        
        // Restore RBP from RDI.
        "leaq   0x90(%%rdi), %%rbp\n\t"
        
        // Get an user-defined result from RSI.
        "movq   %%rsi, %%rax\n\t"
        
    "2:\n\t"
        // Continuation starts here.
        
        // Restore RSP.
        "movq   %%rbp, %%rsp\n\t"
        
        // Restore RBP.
        "movq   -0x88(%%rbp), %%rbp\n\t"
        
    :   // Output constraints
        // Input: Restored RSP. / Output: Overwritten (but discarded).
        "+D" (new_sp) /* RDI */ ,
        // Output: User-defined value from Func or the previous context.
        "=a" (result) /* RAX */
        
    :   // Input constraints
        "S" (arg) /* RSI */ ,
        [func] "i" (Func)
        
    :   "cc", "memory",
        // Caller-saved registers
        "%rcx", "%rdx", "%r8", "%r9", "%r10", "%r11",
        // Callee-saved registers except for EBP
        "%rbx", "%r12", "%r13", "%r14", "%15"
        
        // Not listed in clobbers:
        //  RSI, RDI : Inputs.
        //  RAX : Returned value.
        //  RSP & RBP: Saved & restored.
    );
    
    return { result };
}

MGBASE_NORETURN
inline void restore_context(const context_t ctx, const transfer_t data)
{
    asm volatile (
        // RDI and RSI are passed to the resumed context.
        
        // Both arguments should be passed as an ordinary function parameters
        // because the handler of make_context has function prologue.
        
        "movq   (%%rdi), %%rax\n\t"
        
        // Jump to a continuation or a new context.
        "jmp    *%%rax\n\t"
    
    :   // No output constraints
        
    :   // Input constraints
        "D" (ctx) /* RDI */,
        "S" (data.p0) /* RSI */
        
    :   // No clobbers
    );
    
    MGBASE_UNREACHABLE();
}

} // namespace untyped
} // namespace mgctx

