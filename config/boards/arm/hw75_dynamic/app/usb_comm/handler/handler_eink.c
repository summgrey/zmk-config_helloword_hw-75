/*
 * Copyright (c) 2022-2023 XiNGRZ
 * SPDX-License-Identifier: MIT
 */

#include "handler.h"
#include "usb_comm.pb.h"

#include <device.h>
#include <drivers/display.h>
#include <drivers/display/ssd16xx.h>

#include <logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <pb_encode.h>
#include <pb_decode.h>

#define EINK_NODE DT_NODELABEL(eink)
#define EINK_WIDTH DT_PROP(EINK_NODE, width)
#define EINK_HEIGHT DT_PROP(EINK_NODE, height)

static const struct device *eink;

static const struct device *ssd16xx;
static bool ssd16xx_inited = false;

bool handle_eink_set_image(const EinkImage *req, const void *bits, uint32_t bits_len,
			   EinkImage *res)
{
	if (!eink || !ssd16xx) {
		LOG_ERR("E-Ink device not found, ignoring update request");
		return true;
	}

	res->id = req->id;

	struct display_buffer_descriptor desc = {
		.buf_size = bits_len,
		.width = EINK_WIDTH,
		.height = EINK_HEIGHT,
		.pitch = EINK_WIDTH,
	};

	LOG_DBG("Start updating E-Ink");

	if (!ssd16xx_inited) {
		ssd16xx_clear(ssd16xx);
		ssd16xx_inited = true;
	}

	display_write(eink, 0, 0, &desc, bits);

	LOG_DBG("E-Ink update finished");
	return true;
}

static int handler_eink_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	eink = device_get_binding("EINK");
	ssd16xx = device_get_binding("SSD16XX");
	if (!eink || !ssd16xx) {
		LOG_ERR("E-Ink device not found");
		return -EIO;
	}

	return 0;
}

SYS_INIT(handler_eink_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
