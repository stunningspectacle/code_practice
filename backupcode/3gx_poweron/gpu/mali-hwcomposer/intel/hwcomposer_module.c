
#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>

int hwc_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static struct hw_module_methods_t hwc_module_methods = {
    .open = hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = HWC_HARDWARE_MODULE_ID,
        .name = "hwcomposer module",
        .author = "Intel Mobile Communications",
        .methods = &hwc_module_methods,
    .dso = NULL,
    .reserved = {},
    }
};


