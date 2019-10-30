
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

struct x86_64_context_frame
{
    void*   rbp;
    void*   rip;
    void*   rsp;
};

template <typename T>
struct x86_64_transfer;

template <typename T>
struct x86_64_transfer<T*>
{
    T*  p0;
};

template <typename T>
struct x86_64_cond_transfer;

template <typename T>
struct x86_64_cond_transfer<T*>
{
    T*              p0;
    fdn::int64_t    flag;
};

template <typename T>
struct x86_64_context;

template <typename T>
struct x86_64_context<T*>
{
    x86_64_context_frame*   p;
};


struct x86_64_context_policy_base
{
private:
    using i64 = fdn::int64_t;
    
public:
    using context_frame = x86_64_context_frame;
    
    template <typename T>
    using transfer = x86_64_transfer<T>;
    
    template <typename T>
    using cond_transfer = x86_64_cond_transfer<T>;
    
    template <typename T>
    using context = x86_64_context<T>;
    
protected:
    using make_context_func_t = void (*)(transfer<void*>, void*);
    
    static context<void*> make_context_void(
        void* const                 sp
    ,   const fdn::size_t           /*size*/
    ,   const make_context_func_t   func
    ,   void* const                 arg1
    ) {
        const auto mask = ~i64{0xF};
        
        const auto ctx =
            reinterpret_cast<i64*>(
                (reinterpret_cast<i64>(sp) & mask) - 32
            );
        
        void* start;
        
        asm volatile (
            // Write the label pointer.
            "lea    1f(%%rip), %[start]\n\t"
            // Jump to skip the assembly for starting the context.
            "jmp    2f\n"
            
        "1:\n\t"
            // Pass the result as a parameter.
            "mov    %%rax, %%rdi\n\t"
            // Restore the function argument.
            "pop    %%rsi\n\t"
            // Restore the function pointer.
            "pop    %%rax\n\t"
            // Call the user-defined function.
            //  Func(RDI, RSI)
            "call   *%%rax\n\t"
            // Terminate because of unintended return
            // from the user-defined function.
            "call   abort@PLT\n"
            
        "2:\n\t"
            
        :   // Output constraints
            [start] "=q" (start)
            
        :   // No input constraints
            
        :   // No clobbers
        );
        
        // Set the restored RBP to zero.
        // This is just for debugging.
        *ctx = 0;
        // Set the entrypoint function.
        *(ctx+1) = reinterpret_cast<i64>(start);
        // Save the function argument.
        *(ctx+2) = reinterpret_cast<i64>(arg1);
        // Save the function pointer.
        *(ctx+3) = reinterpret_cast<i64>(func);
        
        return { reinterpret_cast<context_frame*>(ctx) };
    }
    
    #define SAVE_CONTEXT(saved_rsp) \
        /* Save RSP to a certain callee-saved register. */ \
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
    
    #define CLOBBERS \
        "cc", "memory", \
        /* Caller-saved registers */ \
        "%r10", "%r11", \
        /* Callee-saved registers */ \
        "%r12", "%r13", "%r14", "%r15"
        
        // Not listed in clobbers:
        //  RSI, RDI, RCX,
        //  RDX, R8 , R9    : Inputs.
        //  RAX             : Returned value.
        //  RSP, RBP        : Saved & restored.
        //  RBX             : Function pointer.
    
    #define FUNC_OPERAND    "*%[func]"
    
    #define OUTPUT_CONSTRAINT_FUNC(rbx) \
        /* RBX | Input: User-defined function. / Output: Overwritten (but discarded). */ \
        [func] "+b" (rbx)
    
    #define OUTPUT_CONSTRAINTS(rbx, rdi, rax, rsi, rdx, rcx, r8, r9)   \
        OUTPUT_CONSTRAINT_FUNC(rbx), \
        /* RDI | Input: RSP of the next context. / Output: Overwritten (but discarded). */ \
        "+D" (rdi), \
        /* RAX | Output: User-defined value from Func or the previous context. */ \
        "=a" (rax), \
        /* RSI | Input: 1st user-defined argument. / Output: Overwritten (but discarded). */ \
        "+S" (rsi), \
        /* RDX | Input: 2nd user-defined argument. / Output: Overwritten (but discarded). */ \
        "+d" (rdx), \
        /* RCX | Input: 3rd user-defined argument. / Output: Overwritten (but discarded). */ \
        "+c" (rcx), \
        /* R8  | Input: 4th user-defined argument. / Output: Overwritten (but discarded). */ \
        "+r" (r8), \
        /* R9  | Input: 5th user-defined argument. / Output: Overwritten (but discarded). */ \
        "+r" (r9) \
    
    using save_context_func_t =
        transfer<void*> (*)(context<void*>, void*, void*, void*, void*, void*);
    
    static transfer<void*> save_context_void(
        void* const         sp
    ,   const fdn::size_t   /*size*/
    ,   save_context_func_t func
    ,   void*               arg1 = nullptr
    ,   void*               arg2 = nullptr
    ,   void*               arg3 = nullptr
    ,   void* const         arg4 = nullptr
    ,   void* const         arg5 = nullptr
    ) {
        const auto mask = ~i64{0xF};
        
        // Align the stack pointer.
        auto new_sp = (reinterpret_cast<i64>(sp) & mask) - 0x8;
        
        transfer<void*> result;
        
        register void* arg4_r8 asm ("r8") = arg4;
        register void* arg5_r9 asm ("r9") = arg5;
        
        asm volatile (
            // RDI = new_sp; RSI = arg1; RDX = arg2; RCX = arg3;
            
            SAVE_CONTEXT("r15")
            
            // Push the old RSP to the new context's stack.
            "pushq  %%r15\n\t"
            
            // RBP is still preserved.
            
            // Call the user-defined function here.
            //  (RAX, RDX) = func(RDI, RSI, RDX, RCX, R8, R9);
            "call   " FUNC_OPERAND "\n"
            
            // The function ordinarily finished.
            
        "1:\n\t"
            // Continuation starts here.
            
            // RSP is pointing to the saved RSP.
            // RBP is already restored.
            
            // Restore RSP from the stack.
            // It also skips the red zone.
            "popq   %%rsp"
            
            // result = RAX;
            
        :   OUTPUT_CONSTRAINTS(
            // RBX | Input: User-defined function. / Output: Overwritten (but discarded).
                func ,
            // RDI | Input: Restored RSP. / Output: Overwritten (but discarded).
                new_sp ,
            // RAX | Output: User-defined value from Func or the previous context.
                result.p0 ,
            // RSI | Input: 1st user-defined argument. / Output: Overwritten (but discarded).
                arg1 ,
            // RDX | Input: 2nd user-defined argument. / Output: Overwritten (but discarded).
                arg2 ,
            // RCX | Input: 3rd user-defined argument. / Output: Overwritten (but discarded).
                arg3 ,
            // R8  | Input: 4th user-defined argument. / Output: Overwritten (but discarded).
                arg4_r8 ,
            // R9  | Input: 5th user-defined argument. / Output: Overwritten (but discarded).
                arg5_r9
            )
        :   // No input constraints
        :   CLOBBERS
        );
        
        return result;
    }
    
    using swap_context_func_t = save_context_func_t;
    
    static transfer<void*> swap_context_void(
        context<void*>  ctx
    ,   swap_context_func_t func
    ,   void*           arg1 = nullptr
    ,   void*           arg2 = nullptr
    ,   void*           arg3 = nullptr
    ,   void* const     arg4 = nullptr
    ,   void* const     arg5 = nullptr
    ) {
        transfer<void*> result;
        
        register void* arg4_r8 asm ("r8") = arg4;
        register void* arg5_r9 asm ("r9") = arg5;
        
        asm volatile (
            // RDI = ctx; RSI = arg1; RDX = arg2; RCX = arg3;
            
            SAVE_CONTEXT("r15")
            
            // Restore RBP (May be pushed by the callee again.)
            "popq   %%rbp\n\t"
            
            // Call the user-defined function here.
            //  (RAX, RDX) = func(RDI, RSI, RDX, RCX, R8, R9);
            "jmp    " FUNC_OPERAND "\n"
            
        "1:\n\t"
            // Continuation starts here.
            
            // RSP is pointing to the saved RSP.
            // RBP is already restored.
            
            // Restore RSP from the stack.
            // It also skips the red zone.
            "popq   %%rsp"
            
            // result = RAX;
            
        :   OUTPUT_CONSTRAINTS(
            // RBX | Input: User-defined function. / Output: Overwritten (but discarded).
                func ,
            // RDI | Input: Restored RSP. / Output: Overwritten (but discarded).
                ctx.p ,
            // RAX | Output: User-defined value from Func or the previous context.
                result.p0 ,
            // RSI | Input: 1st user-defined argument. / Output: Overwritten (but discarded).
                arg1 ,
            // RDX | Input: 2nd user-defined argument. / Output: Overwritten (but discarded).
                arg2 ,
            // RCX | Input: 3rd user-defined argument. / Output: Overwritten (but discarded).
                arg3 ,
            // R8  | Input: 4th user-defined argument. / Output: Overwritten (but discarded).
                arg4_r8 ,
            // R9  | Input: 5th user-defined argument. / Output: Overwritten (but discarded).
                arg5_r9
            )
        :   // No input constraints
        :   CLOBBERS
        );
        
        return result;
    }
    
    using cond_swap_context_func_t =
        cond_transfer<void*> (*)(context<void*>, void*, void*, void*, void*, void*);
    
    static transfer<void*> cond_swap_context_void(
        context<void*>  ctx
    ,   cond_swap_context_func_t func
    ,   void*           arg1 = nullptr
    ,   void*           arg2 = nullptr
    ,   void*           arg3 = nullptr
    ,   void* const     arg4 = nullptr
    ,   void* const     arg5 = nullptr
    ) {
        transfer<void*> result;
        
        register void* arg4_r8 asm ("r8") = arg4;
        register void* arg5_r9 asm ("r9") = arg5;
        
        asm volatile (
            // RDI = ctx; RSI = arg1; RDX = arg2; RCX = arg3;
            
            SAVE_CONTEXT("r15")
            
            // Copy the saved context to prepare for the recovery of "2:".
            "movq   %%rdi, %%r14\n\t"
            // Load RBP from the context.
            "movq   (%%rsp), %%rbp\n\t"
            // Call the user-defined function here.
            //  (RAX, RDX) = func(RDI, RSI, RDX, RCX, R8, R9);
            "call   " FUNC_OPERAND "\n\t" // = push %rip; jmp FUNC_OPERAND
            
            // Jump to "2:" if (RDX == 0).
            "testq  %%rdx, %%rdx\n\t"
            "je     2f\n\t"
            
            // Adjust RSP (the same value of RBP is loaded before).
            "popq   %%rbp\n\t"
            // Jump to the saved RIP.
            "ret\n"
            
        "2:\n\t"
            // Recover to the original context.
            
            // Restore the RSP.
            "mov    %%r14, %%rsp\n\t"
            // Restore the original RBP.
            "popq   %%rbp\n\t"
            // Skip the saved RIP.
            "add    $8, %%rsp\n"
            
        "1:\n\t"
            // Continuation starts here.
            
            // RSP is pointing to the saved RSP.
            // RBP is already restored.
            
            // Restore RSP from the stack.
            // It also skips the red zone.
            "popq   %%rsp"
            
            // result = RAX;
            
        :   OUTPUT_CONSTRAINTS(
            // RBX | Input: User-defined function. / Output: Overwritten (but discarded).
                func ,
            // RDI | Input: Restored RSP. / Output: Overwritten (but discarded).
                ctx.p ,
            // RAX | Output: User-defined value from Func or the previous context.
                result.p0 ,
            // RSI | Input: 1st user-defined argument. / Output: Overwritten (but discarded).
                arg1 ,
            // RDX | Input: 2nd user-defined argument. / Output: Overwritten (but discarded).
                arg2 ,
            // RCX | Input: 3rd user-defined argument. / Output: Overwritten (but discarded).
                arg3 ,
            // R8  | Input: 4th user-defined argument. / Output: Overwritten (but discarded).
                arg4_r8 ,
            // R9  | Input: 5th user-defined argument. / Output: Overwritten (but discarded).
                arg5_r9
            )
        :   // No input constraints
        :   CLOBBERS
        );
        
        return result;
    }
    
    using restore_context_func_t =
        transfer<void*> (*)(void*, void*, void*, void*, void*, void*);
    
    [[noreturn]]
    static void restore_context_void(
        const context<void*>    ctx
    ,   restore_context_func_t  func
    ,   void* const             arg0 = nullptr
    ,   void* const             arg1 = nullptr
    ,   void* const             arg2 = nullptr
    ,   void* const             arg3 = nullptr
    ,   void* const             arg4 = nullptr
    ,   void* const             arg5 = nullptr
    ) {
        register void* arg4_r8 asm ("r8") = arg4;
        register void* arg5_r9 asm ("r9") = arg5;
        
        asm volatile (
            // Restore RSP.
            "movq   %[ctx], %%rsp\n\t"
            
            // Restore RBP (May be pushed by the callee again.)
            "popq   %%rbp\n\t"
            
            // Call the user-defined function.
            // The return address is the continuation.
            "jmp    " FUNC_OPERAND "\n\t"
            
            // RAX is passed to the continuation.
            
        :   OUTPUT_CONSTRAINT_FUNC(
            // RBX | Input: User-defined function. / Output: Overwritten (but discarded).
                func
            )
            
        :   // Input constraints
            
            // Allow any register or memory operand.
            [ctx] "g" (ctx.p),
            // RDI | Input: 0th user-defined argument.
            "D" (arg0),
            // RSI | Input: 1st user-defined argument.
            "S" (arg1),
            // RDX | Input: 2nd user-defined argument.
            "d" (arg2),
            // RCX | Input: 3rd user-defined argument.
            "c" (arg3),
            // R8  | Input: 4th user-defined argument.
            "r" (arg4_r8),
            // R9  | Input: 5th user-defined argument.
            "r" (arg5_r9)
            
        :   // "cc" is added without any strong reasons now.
            "cc",
            // Necessary to flush out the registers to the memory.
            // There was a bug in which *arg was not written to the stack.
            "memory"
        );
        
        CMPTH_UNREACHABLE();
    }
    
    #undef SAVE_CONTEXT
    #undef CLOBBERS
    #undef FUNC_OPERAND
    #undef OUTPUT_CONSTRAINT_FUNC
    #undef OUTPUT_CONSTRAINTS
};


template <typename P>
struct x86_64_context_policy
    : x86_64_context_policy_base
{
private:
    using i64 = fdn::int64_t;
    
    template <typename Func, typename T, typename A1>
    [[noreturn]]
    static void on_make_context(const transfer<T*> tr, A1* const arg1)
    {
        Func{}(tr, arg1);
    }
    
public:
    template <typename Func, typename T, typename A1>
    static context<T*> make_context(
        void* const         sp
    ,   const fdn::size_t   size
    ,   A1* const           arg1
    ) {
        const auto func =
            reinterpret_cast<make_context_func_t>(
                &x86_64_context_policy::on_make_context<Func, T, A1>
            );
        
        const auto ctx =
            x86_64_context_policy_base::make_context_void(
                sp, size, func, arg1
            );
        
        return { ctx.p };
    }
    
private:
    template <typename Func, typename T, typename... Args>
    static transfer<T*> on_save(const context<T*> ctx, Args* const ... args) {
        return Func{}(ctx, args...);
    }
    
    template <typename Func, typename T,
        typename A1, typename A2, typename A3, typename A4,
        typename... RArgs>
    static transfer<T*> on_save_many(
        const context<T*>   ctx
    ,   A1* const           arg1
    ,   A2* const           arg2
    ,   A3* const           arg3
    ,   A4* const           arg4
    ,   const fdn::tuple<RArgs* ...>* const rest_args
    ) {
        return fdn::apply(
            Func{}
        ,   fdn::tuple_cat(
                fdn::forward_as_tuple(ctx, arg1, arg2, arg3, arg4)
            ,   *rest_args
            )
        );
    }
    
public:
    template <typename Func, typename T, typename... Args,
        fdn::enable_if_t<
            (sizeof...(Args) <= 5) // Enabled with <= 5 parameters
        >* = nullptr>
    static transfer<T*> save_context(
        void* const         sp
    ,   const fdn::size_t   size
    ,   Args* const ...     args
    ) {
        const auto func =
            reinterpret_cast<save_context_func_t>(
                &x86_64_context_policy::on_save<Func, T, Args...>
            );
        
        const auto tr =
            x86_64_context_policy_base::save_context_void(
                sp, size, func, args...
            );
        
        return { static_cast<T*>(tr.p0) };
    }
    
    template <typename Func, typename T,
        typename A1, typename A2, typename A3, typename A4,
        typename... RArgs,
        fdn::enable_if_t<
            (sizeof...(RArgs) >= 2) // Enabled >= 6 parameters
        >* = nullptr>
    static transfer<T*> save_context(
        void* const         sp
    ,   const fdn::size_t   size
    ,   A1* const           arg1
    ,   A2* const           arg2
    ,   A3* const           arg3
    ,   A4* const           arg4
    ,   RArgs* const ...    rest_args
    ) {
        const auto func =
            reinterpret_cast<save_context_func_t>(
                &x86_64_context_policy::on_save_many<
                    Func, T, A1, A2, A3, A4, RArgs...
                >
            );
        
        fdn::tuple<RArgs* ...> rargs{ rest_args... };
        
        const auto tr =
            x86_64_context_policy_base::save_context_void(
                sp, size, func, arg1, arg2, arg3, arg4, &rargs
            );
        
        return { static_cast<T*>(tr.p0) };
    }
    
    template <typename Func, typename T, typename... Args,
        fdn::enable_if_t<
            (sizeof...(Args) <= 5) // Enabled with <= 5 parameters
        >* = nullptr>
    static transfer<T*> swap_context(
        const context<T*>   ctx
    ,   Args* const ...     args
    ) {
        const auto func =
            reinterpret_cast<swap_context_func_t>(
                &x86_64_context_policy::on_save<Func, T, Args...>
            );
        
        const auto tr =
            x86_64_context_policy_base::swap_context_void(
                { ctx.p }, func, args...
            );
        
        return { static_cast<T*>(tr.p0) };
    }
    
    template <typename Func, typename T,
        typename A1, typename A2, typename A3, typename A4,
        typename... RArgs,
        fdn::enable_if_t<
            (sizeof...(RArgs) >= 2) // Enabled >= 6 parameters
        >* = nullptr>
    static transfer<T*> swap_context(
        const context<T*>   ctx
    ,   A1* const           arg1
    ,   A2* const           arg2
    ,   A3* const           arg3
    ,   A4* const           arg4
    ,   RArgs* const ...    rest_args
    ) {
        const auto func =
            reinterpret_cast<swap_context_func_t>(
                &x86_64_context_policy::on_save_many<
                    Func, T, A1, A2, A3, A4, RArgs...
                >
            );
        
        fdn::tuple<RArgs* ...> rargs{ rest_args... };
        
        const auto tr =
            x86_64_context_policy_base::swap_context_void(
                { ctx.p }, func, arg1, arg2, arg3, arg4, &rargs
            );
        
        return { static_cast<T*>(tr.p0) };
    }
    
private:
    template <typename Func, typename T, typename... Args>
    static cond_transfer<T*> on_cond_swap(const context<T*> ctx, Args* const ... args) {
        return Func{}(ctx, args...);
    }
    
    template <typename Func, typename T,
        typename A1, typename A2, typename A3, typename A4,
        typename... RArgs>
    static cond_transfer<T*> on_cond_swap_many(
        const context<T*>   ctx
    ,   A1* const           arg1
    ,   A2* const           arg2
    ,   A3* const           arg3
    ,   A4* const           arg4
    ,   const fdn::tuple<RArgs* ...>* const rest_args
    ) {
        return fdn::apply(
            Func{}
        ,   fdn::tuple_cat(
                fdn::forward_as_tuple(ctx, arg1, arg2, arg3, arg4)
            ,   *rest_args
            )
        );
    }
    
public:
    template <typename Func, typename T, typename... Args,
        fdn::enable_if_t<
            (sizeof...(Args) <= 5) // Enabled with <= 5 parameters
        >* = nullptr>
    static transfer<T*> cond_swap_context(
        const context<T*>   ctx
    ,   Args* const ...     args
    ) {
        const auto func =
            reinterpret_cast<cond_swap_context_func_t>(
                &x86_64_context_policy::on_cond_swap<Func, T, Args...>
            );
        
        const auto tr =
            x86_64_context_policy_base::cond_swap_context_void(
                { ctx.p }, func, args...
            );
        
        return { static_cast<T*>(tr.p0) };
    }
    
    template <typename Func, typename T,
        typename A1, typename A2, typename A3, typename A4,
        typename... RArgs,
        fdn::enable_if_t<
            (sizeof...(RArgs) >= 2) // Enabled >= 6 parameters
        >* = nullptr>
    static transfer<T*> cond_swap_context(
        const context<T*>   ctx
    ,   A1* const           arg1
    ,   A2* const           arg2
    ,   A3* const           arg3
    ,   A4* const           arg4
    ,   RArgs* const ...    rest_args
    ) {
        const auto func =
            reinterpret_cast<cond_swap_context_func_t>(
                &x86_64_context_policy::on_cond_swap_many<
                    Func, T, A1, A2, A3, A4, RArgs...
                >
            );
        
        fdn::tuple<RArgs* ...> rargs{ rest_args... };
        
        const auto tr =
            x86_64_context_policy_base::cond_swap_context_void(
                { ctx.p }, func, arg1, arg2, arg3, arg4, &rargs
            );
        
        return { static_cast<T*>(tr.p0) };
    }
    
private:
    template <typename Func, typename T, typename... Args>
    static transfer<T*> on_restore(Args* const ... args) {
        return Func{}(args...);
    }
    
    template <typename Func, typename T,
        typename A0, typename A1, typename A2, typename A3, typename A4,
        typename... RArgs>
    static transfer<T*> on_restore_many(
        A0* const   arg0
    ,   A1* const   arg1
    ,   A2* const   arg2
    ,   A3* const   arg3
    ,   A4* const   arg4
    ,   const fdn::tuple<RArgs* ...>* const rest_args
    ) {
        return fdn::apply(
            Func{}
        ,   fdn::tuple_cat(
                fdn::forward_as_tuple(arg0, arg1, arg2, arg3, arg4)
            ,   *rest_args
            )
        );
    }
    
public:
    template <typename Func, typename T, typename... Args,
        fdn::enable_if_t<
            (sizeof...(Args) <= 6) // Enabled with <= 6 parameters
        >* = nullptr>
    [[noreturn]]
    static void restore_context(
        const context<T*>   ctx
    ,   Args* const ...     args
    ) {
        // The pointer to the context must be 16-byte aligned.
        CMPTH_P_ASSERT(P, reinterpret_cast<i64>(ctx.p) % 0x10 == 0);
        
        const auto func =
            reinterpret_cast<restore_context_func_t>(
                &x86_64_context_policy::on_restore<Func, T, Args...>
            );
        
        x86_64_context_policy_base::restore_context_void(
            { ctx.p }, func, args...
        );
        
        CMPTH_UNREACHABLE();
    }
    
    template <typename Func, typename T,
        typename A0, typename A1, typename A2, typename A3, typename A4,
        typename... RArgs,
        fdn::enable_if_t<
            (sizeof...(RArgs) >= 2) // Enabled with >= 7 parameters
        >* = nullptr>
    [[noreturn]]
    static void restore_context(
        const context<T*>   ctx
    ,   A0* const           arg0
    ,   A1* const           arg1
    ,   A2* const           arg2
    ,   A3* const           arg3
    ,   A4* const           arg4
    ,   RArgs* const ...    rest_args
    ) {
        // The pointer to the context must be 16-byte aligned.
        CMPTH_P_ASSERT(P, reinterpret_cast<i64>(ctx.p) % 0x10 == 0);
        
        const auto func =
            reinterpret_cast<restore_context_func_t>(
                &x86_64_context_policy::on_restore_many<
                    Func, T, A0, A1, A2, A3, A4, RArgs...
                >
            );
        
        fdn::tuple<RArgs* ...> rargs{ rest_args... };
        
        x86_64_context_policy_base::restore_context_void(
            { ctx.p }, func, arg0, arg1, arg2, arg3, arg4, &rargs
        );
        
        CMPTH_UNREACHABLE();
    }
};

} // namespace cmpth

