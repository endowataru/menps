.text
.globl _mectx_entrypoint
.align 8
_mectx_entrypoint:
    // Pass the result as a parameter.
    movq    %rax, %rdi
    
    // Restore the function pointer.
    movq    (%rsp), %rax
    
    // Call the user-defined function.
    call    *%rax
    
    // Terminate because of unintended return
    // from the user-defined function.
    call    _abort

