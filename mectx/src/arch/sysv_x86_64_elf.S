.text
.globl mectx_entrypoint
.type mectx_entrypoint,@function
.align 16
mectx_entrypoint:
    // Pass the result as a parameter.
    movq    %rax, %rdi
    
    // Restore the function pointer.
    movq    (%rsp), %rax
    
    // Call the user-defined function.
    call    *%rax
    
    // Terminate because of unintended return
    // from the user-defined function.
    call    abort@PLT
    
.size mectx_entrypoint,.-mectx_entrypoint

// Mark that we don't need executable stack.
.section .note.GNU-stack,"",%progbits
