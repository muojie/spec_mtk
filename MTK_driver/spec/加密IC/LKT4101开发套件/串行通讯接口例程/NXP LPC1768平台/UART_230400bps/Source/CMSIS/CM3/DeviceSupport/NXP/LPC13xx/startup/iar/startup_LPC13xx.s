;/*****************************************************************************
; * @file:    startup_LPC13xx.s
; * @purpose: CMSIS Cortex-M3 Core Device Startup File 
; *           for the NXP LPC13xx Device Series 
; * @version: V1.00
; * @date:    19. October 2009
; *----------------------------------------------------------------------------
; *
; * Copyright (C) 2009 ARM Limited. All rights reserved.
; *
; * ARM Limited (ARM) is supplying this software for use with Cortex-Mx 
; * processor based microcontrollers.  This file can be freely distributed 
; * within development tools that are supporting such ARM based processors. 
; *
; * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
; * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; *
; ******************************************************************************/


;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY:
; it is where the SP start value is found, and the NVIC vector
; table register (VTOR) is initialized to this address if != 0.
;
; Cortex-M version
;

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)
	
        EXTERN  __iar_program_start
        EXTERN  SystemInit
        PUBLIC  __vector_table
        PUBLIC  __vector_table_0x1c
        PUBLIC  __Vectors
        PUBLIC  __Vectors_End
        PUBLIC  __Vectors_Size

        DATA

__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler

        DCD     NMI_Handler
        DCD     HardFault_Handler
        DCD     MemManage_Handler
        DCD     BusFault_Handler
        DCD     UsageFault_Handler
__vector_table_0x1c
        DCD     0
        DCD     0
        DCD     0
        DCD     0
        DCD     SVC_Handler
        DCD     DebugMon_Handler
        DCD     0
        DCD     PendSV_Handler
        DCD     SysTick_Handler

        ; External Interrupts
        DCD     WAKEUP_IRQHandler         ; 16+ 0: Wakeup PIO0.0
        DCD     WAKEUP_IRQHandler         ; 16+ 1: Wakeup PIO0.1
        DCD     WAKEUP_IRQHandler         ; 16+ 2: Wakeup PIO0.2
        DCD     WAKEUP_IRQHandler         ; 16+ 3: Wakeup PIO0.3
        DCD     WAKEUP_IRQHandler         ; 16+ 4: Wakeup PIO0.4
        DCD     WAKEUP_IRQHandler         ; 16+ 5: Wakeup PIO0.5
        DCD     WAKEUP_IRQHandler         ; 16+ 6: Wakeup PIO0.6
        DCD     WAKEUP_IRQHandler         ; 16+ 7: Wakeup PIO0.7
        DCD     WAKEUP_IRQHandler         ; 16+ 8: Wakeup PIO0.8
        DCD     WAKEUP_IRQHandler         ; 16+ 9: Wakeup PIO0.9
        DCD     WAKEUP_IRQHandler         ; 16+10: Wakeup PIO0.10
        DCD     WAKEUP_IRQHandler         ; 16+11: Wakeup PIO0.11
        DCD     WAKEUP_IRQHandler         ; 16+12: Wakeup PIO1.0
        DCD     WAKEUP_IRQHandler         ; 16+13: Wakeup PIO1.1
        DCD     WAKEUP_IRQHandler         ; 16+14: Wakeup PIO1.2
        DCD     WAKEUP_IRQHandler         ; 16+15: Wakeup PIO1.3
        DCD     WAKEUP_IRQHandler         ; 16+16: Wakeup PIO1.4
        DCD     WAKEUP_IRQHandler         ; 16+17: Wakeup PIO1.5
        DCD     WAKEUP_IRQHandler         ; 16+18: Wakeup PIO1.6
        DCD     WAKEUP_IRQHandler         ; 16+19: Wakeup PIO1.7
        DCD     WAKEUP_IRQHandler         ; 16+20: Wakeup PIO1.8
        DCD     WAKEUP_IRQHandler         ; 16+21: Wakeup PIO1.9
        DCD     WAKEUP_IRQHandler         ; 16+22: Wakeup PIO1.10
        DCD     WAKEUP_IRQHandler         ; 16+23: Wakeup PIO1.11
        DCD     WAKEUP_IRQHandler         ; 16+24: Wakeup PIO2.0
        DCD     WAKEUP_IRQHandler         ; 16+25: Wakeup PIO2.1
        DCD     WAKEUP_IRQHandler         ; 16+26: Wakeup PIO2.2
        DCD     WAKEUP_IRQHandler         ; 16+27: Wakeup PIO2.3
        DCD     WAKEUP_IRQHandler         ; 16+28: Wakeup PIO2.4
        DCD     WAKEUP_IRQHandler         ; 16+29: Wakeup PIO2.5
        DCD     WAKEUP_IRQHandler         ; 16+30: Wakeup PIO2.6
        DCD     WAKEUP_IRQHandler         ; 16+31: Wakeup PIO2.7
        DCD     WAKEUP_IRQHandler         ; 16+32: Wakeup PIO2.8
        DCD     WAKEUP_IRQHandler         ; 16+33: Wakeup PIO2.9
        DCD     WAKEUP_IRQHandler         ; 16+34: Wakeup PIO2.10
        DCD     WAKEUP_IRQHandler         ; 16+35: Wakeup PIO2.11
        DCD     WAKEUP_IRQHandler         ; 16+36: Wakeup PIO3.0
        DCD     WAKEUP_IRQHandler         ; 16+37: Wakeup PIO3.1
        DCD     WAKEUP_IRQHandler         ; 16+38: Wakeup PIO3.2
        DCD     WAKEUP_IRQHandler         ; 16+39: Wakeup PIO3.3
        DCD     I2C_IRQHandler            ; 16+40: I2C
        DCD     TIMER16_0_IRQHandler      ; 16+41: 16-bit Counter-Timer 0
        DCD     TIMER16_1_IRQHandler      ; 16+42: 16-bit Counter-Timer 1
        DCD     TIMER32_0_IRQHandler      ; 16+43: 32-bit Counter-Timer 0
        DCD     TIMER32_1_IRQHandler      ; 16+44: 32-bit Counter-Timer 1
        DCD     SSP_IRQHandler            ; 16+45: SSP
        DCD     UART_IRQHandler           ; 16+46: UART
        DCD     USB_IRQHandler            ; 16+47: USB IRQ
        DCD     USB_FIQHandler            ; 16+48: USB FIQ
        DCD     ADC_IRQHandler            ; 16+49: A/D Converter
        DCD     WDT_IRQHandler            ; 16+50: Watchdog Timer
        DCD     BOD_IRQHandler            ; 16+51: Brown Out Detect
        DCD     FMC_IRQHandler            ; 16+52: IP2111 Flash Memory Controller
        DCD     PIOINT3_IRQHandler        ; 16+53: PIO INT3
        DCD     PIOINT2_IRQHandler        ; 16+54: PIO INT2
        DCD     PIOINT1_IRQHandler        ; 16+55: PIO INT1
        DCD     PIOINT0_IRQHandler        ; 16+56: PIO INT0
__Vectors_End

__Vectors       EQU   __vector_table
__Vectors_Size 	EQU 	__Vectors_End - __Vectors


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB

        PUBWEAK Reset_Handler
        SECTION .text:CODE:REORDER(2)
Reset_Handler
        LDR     R0, =SystemInit
        BLX     R0
        LDR     R0, =__iar_program_start
        BX      R0

        PUBWEAK NMI_Handler
        SECTION .text:CODE:REORDER(1)
NMI_Handler
        B NMI_Handler

        PUBWEAK HardFault_Handler
        SECTION .text:CODE:REORDER(1)
HardFault_Handler
        B HardFault_Handler

        PUBWEAK MemManage_Handler
        SECTION .text:CODE:REORDER(1)
MemManage_Handler
        B MemManage_Handler

        PUBWEAK BusFault_Handler
        SECTION .text:CODE:REORDER(1)
BusFault_Handler
        B BusFault_Handler

        PUBWEAK UsageFault_Handler
        SECTION .text:CODE:REORDER(1)
UsageFault_Handler
        B UsageFault_Handler

        PUBWEAK SVC_Handler
        SECTION .text:CODE:REORDER(1)
SVC_Handler
        B SVC_Handler

        PUBWEAK DebugMon_Handler
        SECTION .text:CODE:REORDER(1)
DebugMon_Handler
        B DebugMon_Handler

        PUBWEAK PendSV_Handler
        SECTION .text:CODE:REORDER(1)
PendSV_Handler
        B PendSV_Handler

        PUBWEAK SysTick_Handler
        SECTION .text:CODE:REORDER(1)
SysTick_Handler
        B SysTick_Handler

        PUBWEAK WAKEUP_IRQHandler
        SECTION .text:CODE:REORDER(1)
WAKEUP_IRQHandler
        B WAKEUP_IRQHandler

        PUBWEAK I2C_IRQHandler
        SECTION .text:CODE:REORDER(1)
I2C_IRQHandler
        B I2C_IRQHandler

        PUBWEAK TIMER16_0_IRQHandler
        SECTION .text:CODE:REORDER(1)
TIMER16_0_IRQHandler
        B TIMER16_0_IRQHandler

        PUBWEAK TIMER16_1_IRQHandler
        SECTION .text:CODE:REORDER(1)
TIMER16_1_IRQHandler
        B TIMER16_1_IRQHandler

        PUBWEAK TIMER32_0_IRQHandler
        SECTION .text:CODE:REORDER(1)
TIMER32_0_IRQHandler
        B TIMER32_0_IRQHandler

        PUBWEAK TIMER32_1_IRQHandler
        SECTION .text:CODE:REORDER(1)
TIMER32_1_IRQHandler
        B TIMER32_1_IRQHandler

        PUBWEAK SSP_IRQHandler
        SECTION .text:CODE:REORDER(1)
SSP_IRQHandler
        B SSP_IRQHandler

        PUBWEAK UART_IRQHandler
        SECTION .text:CODE:REORDER(1)
UART_IRQHandler
        B UART_IRQHandler

        PUBWEAK USB_IRQHandler
        SECTION .text:CODE:REORDER(1)
USB_IRQHandler
        B USB_IRQHandler

        PUBWEAK USB_FIQHandler
        SECTION .text:CODE:REORDER(1)
USB_FIQHandler
        B USB_FIQHandler

        PUBWEAK ADC_IRQHandler
        SECTION .text:CODE:REORDER(1)
ADC_IRQHandler
        B ADC_IRQHandler

        PUBWEAK WDT_IRQHandler
        SECTION .text:CODE:REORDER(1)
WDT_IRQHandler
        B WDT_IRQHandler

        PUBWEAK BOD_IRQHandler
        SECTION .text:CODE:REORDER(1)
BOD_IRQHandler
        B BOD_IRQHandler

        PUBWEAK FMC_IRQHandler
        SECTION .text:CODE:REORDER(1)
FMC_IRQHandler
        B FMC_IRQHandler

        PUBWEAK PIOINT3_IRQHandler
        SECTION .text:CODE:REORDER(1)
PIOINT3_IRQHandler
        B PIOINT3_IRQHandler

        PUBWEAK PIOINT2_IRQHandler
        SECTION .text:CODE:REORDER(1)
PIOINT2_IRQHandler
        B PIOINT2_IRQHandler

        PUBWEAK PIOINT1_IRQHandler
        SECTION .text:CODE:REORDER(1)
PIOINT1_IRQHandler
        B PIOINT1_IRQHandler

        PUBWEAK PIOINT0_IRQHandler
        SECTION .text:CODE:REORDER(1)
PIOINT0_IRQHandler
        B PIOINT0_IRQHandler


        END
