#include "i8042.h"
#include <machine/asm.h>

.intel_syntax

.global _C_LABEL(scancode)
.global _C_LABEL (kbc_asm_ih)

.text
 _C_LABEL(kbc_asm_ih):
    
    in  al,STAT_REG

    test al,OBF

    jz fim
    
    in  al,STAT_REG

    test al, PAR_ERR
    
    test al, TO_ERR
    
    jnz fim
    
    in al, OUT_BUF
    
    mov scancode,al

    ret


fim: 
	ret
