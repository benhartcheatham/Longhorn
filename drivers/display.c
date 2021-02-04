#include <stddef.h>
#include "display.h"

dis_driver_t *current_dd = NULL;

void set_display_driver(dis_driver_t *dd) {
    current_dd = dd;
}

dis_driver_t *get_display_driver() {
    return current_dd;
}