/* The following linker definitions should be placed in the stack section */

    /* debugger state */
    __debugger_stack_bottom__ = . ;
    . += 0x48;  /* 16 previous mode registers + SPSR + UNDEF Next Instruction Address */
    __debugger_stack__ = .;
    __debugger_stack_top__ = . ;
    
    /* breakpoints */
    __breakpoints_start__ = . ;
    . += 0x40;  /* Single Stepping Breakpoint + 7 Breakpoints */
    __breakpoints_end__ = . ;

/* Symbols */
    __breakpoints_num__ = (__breakpoints_end__ - __breakpoints_start__) / 8;
