/*
 * Author: Brendan Le Foll <brendan.le.foll@intel.com>
 * Author: Thomas Ingleby <thomas.c.ingleby@intel.com>
 * Copyright (c) 2014 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stddef.h>

#include "maa.h"
#include "intel_galileo_rev_d.h"
#include "gpio.h"
#include "version.h"

static maa_pininfo_t* pindata;
static maa_board_t* plat = NULL;

const char *
maa_get_version()
{
    return gVERSION;
}

maa_result_t
maa_init()
{
    /** Once more board definitions have been added,
     *  A method for detecting them will need to be devised.
     */
    if (plat != NULL) {
        return MAA_ERROR_PLATFORM_ALREADY_INITIALISED;
    }
    plat = maa_intel_galileo_rev_d();
    return MAA_SUCCESS;
}

static maa_result_t
maa_setup_mux_mapped(maa_pin_t meta)
{
    int mi;
    for (mi = 0; mi < meta.mux_total; mi++) {
        maa_gpio_context* mux_i;
        mux_i = maa_gpio_init_raw(meta.mux[mi].pin);
        if (mux_i == NULL)
            return MAA_ERROR_INVALID_HANDLE;
        if (maa_gpio_dir(mux_i, MAA_GPIO_OUT) != MAA_SUCCESS)
            return MAA_ERROR_INVALID_RESOURCE;
        if (maa_gpio_write(mux_i, meta.mux[mi].value) != MAA_SUCCESS)
            return MAA_ERROR_INVALID_RESOURCE;
    }
    return MAA_SUCCESS;
}

unsigned int
maa_check_gpio(int pin)
{
    if (plat == NULL)
        return -1;

    if (pin < 0 || pin > plat->phy_pin_count)
        return -1;

    if(plat->pins[pin].capabilites.gpio != 1)
      return -1;

    if (plat->pins[pin].gpio.mux_total > 0)
       if (maa_setup_mux_mapped(plat->pins[pin].gpio) != MAA_SUCCESS)
            return -1;
    return plat->pins[pin].gpio.pinmap;
}

unsigned int
maa_check_aio(int aio)
{
    if (plat == NULL)
        return -3;

    if (aio < 0 || aio > plat->aio_count)
        return -1;

    int pin = aio + plat->gpio_count;

    if(plat->pins[pin].capabilites.aio != 1)
      return -1;

    if (plat->pins[pin].aio.mux_total > 0)
       if (maa_setup_mux_mapped(plat->pins[pin].aio) != MAA_SUCCESS)
            return -1;
    return plat->pins[pin].aio.pinmap;

}

unsigned int
maa_check_i2c(int bus_s)
{
    if (plat == NULL)
        return -3;

    if (plat->i2c_bus_count >! 0) {
        fprintf(stderr, "No i2c buses defined in platform");
        return -1;
    }
    int bus = 0;

    int pos = plat->i2c_bus[bus].sda;
    if (plat->pins[pos].i2c.mux_total > 0)
        if (maa_setup_mux_mapped(plat->pins[pos].i2c) != MAA_SUCCESS)
             return -2;

    pos = plat->i2c_bus[bus].scl;
    if (plat->pins[pos].i2c.mux_total > 0)
        if (maa_setup_mux_mapped(plat->pins[pos].i2c) != MAA_SUCCESS)
             return -2;

    return plat->i2c_bus[bus].bus_id;
}