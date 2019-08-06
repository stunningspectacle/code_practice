Galileo Demo
=============

The FreeRTOS/Demo/IA32_flat_GCC_Galileo_Gen_2 directory contains a demo application that uses the OpenRTOS IA32 (x86) flat memory model port, and is pre-configured to run on the Galileo Gen 2. 
For more information about this demo functionality, look at http://www.freertos.org/RTOS_Intel_Quark_Galileo_GCC.html.

How to upgrade to a newer version:
===================================
Basically the Galileo demo ISH has in its branch is nearly the same as the original demo that FreeRTOS provides. 
Here is a short summary of what to do in order to rebase new version:
1. Compare the following files from the original folder with our branch version, and merge significant changes into ours:
	a. FreeRTOS/Demo/IA32_flat_GCC_Galileo_Gen_2/Support_Files/startup.S to freeRTOS_kernel\Platform\x86\galileo\startup.S
	b. FreeRTOS/Demo/IA32_flat_GCC_Galileo_Gen_2/elf_ia32_efi.lds to freeRTOS_kernel\build\galileo.lds
2. Add new files to freeRTOS_kernel/Demo/IA32_flat_GCC_Galileo_Gen_2/sources.mk
3. Change mainCREATE_SIMPLE_BLINKY_DEMO_ONLY of FreeRTOS/Demo/IA32_flat_GCC_Galileo_Gen_2/main.c to 0
4. Remove the following files from "Demo/IA32_flat_GCC_Galileo_Gen_2/Support_Files" folder, we aren't allowed to use them due to insufficient license they have:
	* multiboot.h 
	* printf-stdarg.c 
	