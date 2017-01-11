.text
.globl mgctx_entrypoint
.type mgctx_entrypoint,@function
.align 16
mgctx_entrypoint:
    // Pass the result as a parameter.
    movq    %rax, %rdi
    
    // Restore the function pointer.
    movq    (%rsp), %rax
    
    // Call the user-defined function.
    call    *%rax
    
    // Terminate because of unintended return
    // from the user-defined function.
    call    abort@PLT
    
.size mgctx_entrypoint,.-mgctx_entrypoint

// Mark that we don't need executable stack.
.section .note.GNU-stack,"",%progbits
