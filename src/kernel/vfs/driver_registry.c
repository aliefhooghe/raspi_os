
#include "driver_registry.h"
#include "dev/tty.h"

const character_device_ops_t *load_char_device(void)
{
    return dev_tty_create();
}
