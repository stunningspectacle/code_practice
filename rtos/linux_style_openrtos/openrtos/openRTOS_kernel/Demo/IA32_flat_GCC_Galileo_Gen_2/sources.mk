PROJECT_VPATH = ..\openrtos_kernel\Demo:..\openrtos_kernel\Demo\IA32_flat_GCC_Galileo_Gen_2: ..\openrtos_kernel\Platform\x86\clanton:..\openrtos_kernel\Demo\IA32_flat_GCC_Galileo_Gen_2\Full_Demo ..\openrtos_kernel\Platform\x86\Common ..\openrtos_kernel\Platform\Common 

override PROJECT_CFLAGS := $(PROJECT_CFLAGS) -I ..\openrtos_kernel\Demo\IA32_flat_GCC_Galileo_Gen_2  \
                                             -I ..\openrtos_kernel\Demo\IA32_flat_GCC_Galileo_Gen_2\Full_Demo  \
                                             -I ..\openrtos_kernel\Demo\IA32_flat_GCC_Galileo_Gen_2\Support_Files \
                                             -I ..\openrtos_kernel\Platform\Common\include_local \
                                             -I ..\openrtos_kernel\Demo\Common\include \
                                             -I ..\openRTOS_kernel\Platform\x86\clanton \
                                             -I ..\..\kernel_add_ons\include\mmio_headers
                                             
override PROJECT_ASRC := $(PROJECT_ASRC) Full_Demo\RegTest.S io.S
override PROJECT_CSRC := $(PROJECT_CSRC) main.c Blinky_Demo\main_blinky.c Full_Demo\main_full.c Full_Demo\IntQueueTimer.c \
						Common\Minimal\IntQueue.c Common\Minimal\QPeek.c Common\Minimal\QueueSet.c Common\Minimal\TaskNotify.c Common\Minimal\death.c \
						Common\Minimal\flop.c Common\Minimal\dynamic.c Common\Minimal\BlockQ.c Common\Minimal\blocktim.c \
						Common\Minimal\GenQTest.c Common\Minimal\recmutex.c Common\Minimal\semtest.c \
						Common\Minimal\TimerDemo.c Common\Minimal\countsem.c Common\Minimal\QueueOverwrite.c \
						Common\Minimal\EventGroupsDemo.c Common\Minimal\IntSemTest.c \
						Support_Files\HPET.c Support_Files\freestanding_functions.c Support_Files\galileo-support.c \
						Support_Files\GPIO_I2C.c empty_interrupts_reg.c Support_Files\printf.c Common\i2c.c