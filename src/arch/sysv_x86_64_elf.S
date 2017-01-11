.text
.globl mgctx_entrypoint
.type mgctx_entrypoint,@function
.align 16
mgctx_entrypoint:
    
.size mgctx_entrypoint,.-mgctx_entrypoint

// Mark that we don't need executable stack.
.section .note.GNU-stack,"",%progbits
