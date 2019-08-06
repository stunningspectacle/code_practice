PROJECT_VPATH = ..\openRTOS_kernel\Platform\x86\glv:..\openRTOS_kernel\Platform\x86\Common:..\openRTOS_kernel\Demo\x86:..\openRTOS_kernel\Demo:..\openRTOS_kernel\Platform\x86\IA32

override PROJECT_CFLAGS := $(PROJECT_CFLAGS) -I .\ -I ..\openRTOS_kernel\Demo\x86 \
                                             -I ..\openRTOS_kernel\Platform\Common\include_local \
                                             -I ..\openRTOS_kernel\Demo\Common\include \
                                             -I ..\openRTOS_kernel\Platform\x86\$(TARGET_PLATFORM) \
                                             -I ..\..\kernel_add_ons\include\
                                             -I ..\..\kernel_add_ons\include\mmio_headers\
                                             -I ..\..\include\
                                             -I ..\..\include_ext\
                                             -I ..\..\include\drivers\
                                             -I ..\..\include_ext\posix\

override PROJECT_CSRC := $(PROJECT_CSRC) main.c Common\Minimal\flop.c Common\Minimal\dynamic.c \
                         Common\Minimal\BlockQ.c Common\Minimal\blocktim.c Common\Minimal\GenQTest.c \
                        Common\Minimal\recmutex.c Common\Minimal\semtest.c Common\Minimal\TimerDemo.c \
                        Common\Minimal\countsem.c Common\Minimal\QueueOverwrite.c Common\Minimal\EventGroupsDemo.c \
                        Common\Minimal\IntSemTest.c empty_interrupts_reg.c Common\i2c.c 