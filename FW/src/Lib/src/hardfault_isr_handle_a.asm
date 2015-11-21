  
  PUBLIC  HARDFAULT_ISR_HANDLE_d
  
  RSEG CODE:CODE:NOROOT(2)
  
  IMPORT hard_fault_handler_c 
  
HARDFAULT_ISR_HANDLE_d
    TST LR, #4  
    ITE EQ  
    MRSEQ R0, MSP  
    MRSNE R0, PSP  
    B hard_fault_handler_c
    
    END