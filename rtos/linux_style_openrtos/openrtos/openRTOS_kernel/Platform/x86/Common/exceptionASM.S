

.file "exception.S"

#include "ISR_Support.h"


   .extern initDone
   .text

.align 4
.func vPortCentralExcWrapper
vPortCentralExcWrapper:

    /* When the CPU performs a call to an exception handler procedure
     * it pushes the current state of the EFLAGS, CS, and EIP registers on the current stack (USER SPACE: the CPU pushes also the ss and the esp).
     *
     * No need to issue a 'cli' command since the CPU automatically clears the IF flag upon interrupt execution.
     */

    /*
     * Save the handler callback pushed on the stack by the caller
     * in eax without losing eax content.
     */
    xchgl   %eax, (%esp)

    /* Save ecx on the stack */
    pushl   %ecx

    /* Increment nesting count, if counter > 2, we reset FW in release mode and halt in debug mode. */
    incl    ulInterruptNesting
    cmpl    $0x1, ulInterruptNesting
    je      switchToSysStack
    cmpl    $0x2, ulInterruptNesting
    jae      alreadyOnSysStack

nestedException:
#ifdef DEBUG

    hlt

#else

    call pm_reset

#endif

switchToSysStack:

    /* No need to switch to system stack during init flow execution
    because we are already running on it*/
    cmpl   $0x0, initDone
    je     alreadyOnSysStack

    movl    %esp, %ecx
    movl    ulTopOfSystemStack, %esp
    pushl   %ecx

    /* TODO: USER SPACE: push with user space SS */

    /* Copy exception stack frame (EFLAGS, CS, EIP and Error code) pushed by CPU to the system task. */
    pushl   20(%ecx)     /* EFLAGS */
    pushl   16(%ecx)     /* CS */
    pushl   12(%ecx)     /* EIP */
    pushl   8(%ecx)      /* Error code */

    /* Copy eax and ecx we pushed before jumping to switchToSysStack */
    pushl   4(%ecx)     /* original eax */
    pushl   (%ecx)     /* original ecx */

alreadyOnSysStack:

    /*
     * Save other GP registers in the same order "pusha" instruction pushes on the stack, thus we can use
     * "popa" later to restore these resgisters (+ eax and ecx were pushed earlier).
     */
    pushl %edx
    pushl %ebx
    pushl $0
    pushl %ebp
    pushl %esi
    pushl %edi

    /* Push possible page-fault linear address */
    movl %cr2, %ecx
    pushl %ecx

    /* Send esp (which poiting to the vPortExcFrame) as input parameter to exception handler pointed by %ecx */
    pushl   %esp
    call    *%eax

    /* Discard vPortExcFrame parameter and cr2 */
    addl    $8, %esp

    /* Pop general purpose registers. */
    popa

    decl    ulInterruptNesting
    cmpl    $0x0, ulInterruptNesting
    jne     end

    cmpl   $0x0, initDone
    je     end

switchBackFromSysStack:

    addl    $16, %esp /* Pop exception frame: EFLAGS, CS, EIP and Error code*/

    /* reload original esp */
    popl    %esp

    /* discard eax and ecx pushed on the stack */
    addl    $8, %esp


end:
    /* Discard error code pushed by CPU upon exception flow execution*/
    addl    $4, %esp

    iret
.endfunc


.macro CREATE_EXC_WRAPPER wrapper_name exc_handler
.align 4
.global \wrapper_name
.func \wrapper_name
\wrapper_name:
    pushl $\exc_handler
    jmp  vPortCentralExcWrapper
.endfunc
.endm


.macro CREATE_EXC_WRAPPER_NO_ERR_CODE wrapper_name exc_handler
.align 4
.global \wrapper_name
.func \wrapper_name
\wrapper_name:

    /* push dummy error code*/
    pushl $0
    pushl $\exc_handler
    jmp  vPortCentralExcWrapper

    /* never return from exc_handler, unrecovable error has been detected. */
.endfunc
.endm

#if ISH_CONFIG_SUPPORT_PAGING
CREATE_EXC_WRAPPER vPortPageFaultHandler demandPgMgrHook
#endif

CREATE_EXC_WRAPPER vPortSegmentNotPresentWrapper sys_fatal_segment_not_present
CREATE_EXC_WRAPPER vPortInvalidTssWrapper sys_fatal_invalid_tss
CREATE_EXC_WRAPPER vPortStackFaultWrapper sys_fatal_stack_fault
CREATE_EXC_WRAPPER vPortGeneralProtectionWrapper sys_fatal_general_protection

CREATE_EXC_WRAPPER_NO_ERR_CODE vPortInvalidOpcodeWrapper sys_fatal_invalid_opcode
CREATE_EXC_WRAPPER_NO_ERR_CODE vPortDivideErrorWrapper sys_fatal_divide_error
CREATE_EXC_WRAPPER_NO_ERR_CODE vPortNmiIntteruptWrapper sys_fatal_nmi_intterupt
CREATE_EXC_WRAPPER_NO_ERR_CODE vPortOverflowWrapper sys_fatal_overflow
CREATE_EXC_WRAPPER_NO_ERR_CODE vPortBoundRangeExceededWrapper sys_fatal_bound_range_exceeded
CREATE_EXC_WRAPPER_NO_ERR_CODE vPortDeviceNotAvailableWrapper fp_device_not_available_handler
CREATE_EXC_WRAPPER_NO_ERR_CODE vPortDoubleFaultWrapper sys_fatal_double_fault
CREATE_EXC_WRAPPER_NO_ERR_CODE vPortCoprocessorSegmentOverrunWrapper sys_fatal_coprocessor_segment_overrun
CREATE_EXC_WRAPPER_NO_ERR_CODE vPortFpuFloatingPointWrapper sys_fatal_fpu_floating_point
CREATE_EXC_WRAPPER_NO_ERR_CODE vPortAlignmentCheckWrapper sys_fatal_alignment_check
CREATE_EXC_WRAPPER_NO_ERR_CODE vPortMachineCheckWrapper sys_fatal_machine_check
CREATE_EXC_WRAPPER_NO_ERR_CODE vPortSimdFloatingPointWrapper sys_fatal_simd_floating_point

.end
