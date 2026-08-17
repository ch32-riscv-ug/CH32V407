/********************************** (C) COPYRIGHT *******************************
* File Name          : usbd_driver_core.c
* Author             : WCH
* Version            : V1.2
* Date               : 2026/05/26
* Description        : Usb device driver core file.
*********************************************************************************
* Copyright (c) 2026 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/* @include */
#include <string.h>

#include "usb_driver.h"
#include "device/usbd_driver_private.h"

#ifdef USB_DEVICE_DRIVER_EN

/* @function declaration */
static void _reset_parameter(usbd_handle_t *h);
static void _setup_process(usbd_handle_t *h, usbd_event_t *e);
static void _data_process(usbd_handle_t *h, usbd_event_t *e);
static void _ctrl_xfer_process(usbd_handle_t *h, usbd_event_t *e);
static usb_rst_e _register_standard_request(usbd_handle_t *h);
static usb_rst_e _set_address_setup(usbd_handle_t *h, const usb_req_t *req, void **buf, size_t *size);
static usb_rst_e _clear_feature_setup(usbd_handle_t *h, const usb_req_t *req, void **buf, size_t *size);
static usb_rst_e _get_config_setup(usbd_handle_t *h, const usb_req_t *req, void **buf, size_t *size);
static usb_rst_e _get_status_setup(usbd_handle_t *h, const usb_req_t *req, void **buf, size_t *size);
static usb_rst_e _set_config_setup(usbd_handle_t *h, const usb_req_t *req, void **buf, size_t *size);
static usb_rst_e _set_feature_setup(usbd_handle_t *h, const usb_req_t *req, void **buf, size_t *size);
static void _set_address_status(usbd_handle_t *h, const usb_req_t *req, void *buf, size_t size);
static void _set_config_status(usbd_handle_t *h, const usb_req_t *req, void *buf, size_t size);
static void _set_feature_status(usbd_handle_t *h, const usb_req_t *req, void *buf, size_t size);

usb_rst_e usbd_drv_init(usbd_handle_t *h)
{
    USB_LOGI("===========================================");
    USB_LOGI(" USB Device Driver Init");
    USB_LOGI(" Version: %s", USB_DRIVER_VERSION_STRING);
    USB_LOGI(" Build Time: %s %s", __DATE__, __TIME__);
    USB_LOGI("===========================================");

    if (!h)
    {
        USB_LOGE("Init handle is NULL!");
        return USB_RST_NULL_PTR;
    }

    if (h->feature.ep0_size != 8 && h->feature.ep0_size != 64)
    {
        USB_LOGE("Init EP0 size error!");
        return USB_RST_ERROR_PARAMETER;
    }

    h->feature.self_powered = USBD_SELF_POWERED;

    h->open_status = USB_FALSE;

    h->ctrl_req_cb_count = 0;
    memset(&h->ctrl_req_cb_items, 0, sizeof(h->ctrl_req_cb_items));
    memset(&h->ctrl_itf_cb_items, 0, sizeof(h->ctrl_itf_cb_items));
    _reset_parameter(h);

    usb_rst_e rst = _register_standard_request(h);
    if (rst != USB_RST_OK)
    {
        USB_LOGE("Register standard request error!");
        return rst;
    }

    return USB_RST_OK;
}

usb_rst_e usbd_drv_task(usbd_handle_t *h)
{
    usbd_event_t e;
    e.e = USBD_EVENT_NONE;

    if (h->get_event(&e) == USB_RST_OK)
    {
        switch (e.e)
        {
        case USBD_EVENT_NONE:
            break;

        case USBD_EVENT_RESET:
            USB_LOGI("Bus reset is %s speed.",
                     (e.reset.link_speed == USB_SPEED_HIGH) ? "high" :
                     (e.reset.link_speed == USB_SPEED_FULL) ? "full" :
                     (e.reset.link_speed == USB_SPEED_LOW) ? "low" : "unknown");

            _reset_parameter(h);
            h->set_address(0x00);
            h->feature.link_speed = e.reset.link_speed;
            h->clear_event(&e);
            if (h->event_cbs[USBD_CB_EVENT_RESET])
            {
                h->event_cbs[USBD_CB_EVENT_RESET](h, 0);
            }
            break;

        case USBD_EVENT_SUSPEND:
            USB_LOGI("Bus suspend");
            h->clear_event(&e);
            if (h->event_cbs[USBD_CB_EVENT_SUSPEND] && h->feature.address)
            {
                h->event_cbs[USBD_CB_EVENT_SUSPEND](h, 0);
            }
            break;

        case USBD_EVENT_SOF:
            h->clear_event(&e);
            if (h->event_cbs[USBD_CB_EVENT_SOF])
            {
                h->event_cbs[USBD_CB_EVENT_SOF](h, e.sof.frame_no);
            }
            break;

        case USBD_EVENT_SETUP:
            _setup_process(h, &e);
            h->clear_event(&e);
            break;

        case USBD_EVENT_XFER:
            if (ENDP_NUM(e.xfer.ep) == 0x00)
            {
                _ctrl_xfer_process(h, &e);
            }
            else
            {
                _data_process(h, &e);
            }
            h->clear_event(&e);
            break;
        }
    }

    return USB_RST_OK;
}

usb_rst_e usbd_enable(usbd_handle_t *h)
{
    if (!h) return USB_RST_NULL_PTR;
    if (!h->enable) return USB_RST_NULL_PTR;
    if (h->enable_status) return USB_RST_OK;

    usb_rst_e rst = h->enable();
    if (rst != USB_RST_OK)
    {
        USB_LOGE("Usb device controller enable failed!");
    }

    h->enable_status = (rst == USB_RST_OK) ? USB_TRUE : USB_FALSE;
    return rst;
}

usb_rst_e usbd_disable(usbd_handle_t *h)
{
    if (!h) return USB_RST_NULL_PTR;
    if (!h->disable) return USB_RST_NULL_PTR;
    if (!h->enable_status) return USB_RST_OK;

    usb_rst_e rst = h->disable();
    h->enable_status = USB_FALSE;
    h->open_status = USB_FALSE;
    return rst;
}

usb_rst_e usbd_open(usbd_handle_t *h, usb_speed_e speed, usb_bool_t sof_en)
{
    usb_rst_e rst;

    if (!h)
    {
        USB_LOGE("Device handle is NULL!");
        return USB_RST_NULL_PTR;
    }

    if (!h->enable_status)
    {
        USB_LOGE("Device controller not enabled!");
        return USB_RST_NOT_ENABLE;
    }

    if (!h->open || !h->endp_open)
    {
        USB_LOGE("Open func is NULL!");
        return USB_RST_NULL_PTR;
    }

    if (h->open_status) return USB_RST_OK;

    rst = h->open(speed, sof_en);
    if (rst != USB_RST_OK)
    {
        USB_LOGE("Usb device controller open failed!");
        return rst;
    }

    rst = h->endp_open(0x80, USBD_ENDP_FEATURE_NORMAL, h->feature.ep0_size);
    if (rst != USB_RST_OK)
    {
        USB_LOGE("Open EP 0x80 error!");
        return rst;
    }

    rst = h->endp_open(0x00, USBD_ENDP_FEATURE_NORMAL, h->feature.ep0_size);
    if (rst != USB_RST_OK)
    {
        USB_LOGE("Open EP 0x00 error!");
        return rst;
    }

    rst = h->set_endp_buf(0x80, h->ctrl_ctx.buf);
    if (rst != USB_RST_OK) return rst;

    rst = h->set_endp_buf(0x00, h->ctrl_ctx.buf);
    if (rst != USB_RST_OK) return rst;

    rst = h->set_endp_size(0x00, sizeof(usb_req_t));
    if (rst != USB_RST_OK) return rst;

    rst = h->set_endp_response(0x80, ENDP_RESP_NAK);
    if (rst != USB_RST_OK) return rst;

    rst = h->set_endp_response(0x00, ENDP_RESP_ACK);
    if (rst != USB_RST_OK) return rst;

    h->open_status = USB_TRUE;

    return USB_RST_OK;
}

usb_rst_e usbd_close(usbd_handle_t *h)
{
    if (!h) return USB_RST_NULL_PTR;
    if (!h->close) return USB_RST_NULL_PTR;
    if (!h->enable_status) return USB_RST_NOT_ENABLE;
    if (!h->open_status) return USB_RST_OK;

    h->open_status = USB_FALSE;
    return h->close();
}

usb_bool_t usbd_is_enable(usbd_handle_t *h)
{
    if (!h) return USB_FALSE;
    return h->enable_status;
}

usb_bool_t usbd_is_open(usbd_handle_t *h)
{
    if (!h) return USB_FALSE;
    return h->open_status;
}

usb_bool_t usbd_is_enum(usbd_handle_t *h)
{
    if (!h) return USB_FALSE;
    return h->enum_status;
}

usb_speed_e usbd_get_speed(usbd_handle_t *h)
{
    if (!h) return USB_SPEED_UNKNOW;
    return h->feature.link_speed;
}

usb_rst_e usbd_resume(usbd_handle_t *h)
{
    if (!h) return USB_RST_NULL_PTR;
    if (!h->enable_status) return USB_RST_NOT_ENABLE;
    if (!h->open_status) return USB_RST_NOT_OPEN;
    if (!h->resume) return USB_RST_NULL_PTR;
    if (!h->feature.remote_wakeup) return USB_RST_FAILED;

    return h->resume();
}

usb_rst_e usbd_interrupt_ctrl(usbd_handle_t *h, usb_bool_t status)
{
    if (!h) return USB_RST_NULL_PTR;
    if (!h->interrupt_ctrl) return USB_RST_NULL_PTR;

    return h->interrupt_ctrl(status);
}

usb_rst_e usbd_endp_open(usbd_handle_t *h, usb_endp_t ep, uint8_t feature, uint16_t max_size)
{
    usb_rst_e rst;
    uint8_t num = ENDP_NUM(ep);

    if (!h) return USB_RST_NULL_PTR;
    if (!h->enable_status) return USB_RST_NOT_ENABLE;
    if (!h->endp_open || !h->set_endp_toggle)
        return USB_RST_NULL_PTR;
    if (num == 0) return USB_RST_UNSUPPORTED_ENDP;

    usbd_endp_param_t *param = &h->endp_params[ENDP_DIR(ep) ? USB_DIR_IN : USB_DIR_OUT][num];
    param->status = ENDP_STA_IDLE;
    param->feature = feature & ~USBD_ENDP_FEATURE_BURST;
    param->max_size = max_size;
    param->xfer_length = 0;
    param->xfer_offset = 0;

    rst = h->set_endp_toggle(ep, ENDP_TOG_DATA0);
    if (rst != USB_RST_OK) return rst;

    rst = h->set_endp_response(ep, ENDP_RESP_NAK);
    if (rst != USB_RST_OK) return rst;

    rst = h->endp_open(ep, feature, max_size);
    if (rst != USB_RST_OK) return rst;

    return USB_RST_OK;
}

usb_rst_e usbd_endp_close(usbd_handle_t *h, usb_endp_t ep)
{
    if (!h) return USB_RST_NULL_PTR;
    if (!h->enable_status) return USB_RST_NOT_ENABLE;
    if (!h->endp_close) return USB_RST_NULL_PTR;

    return h->endp_close(ep);
}

usb_rst_e usbd_endp_set_response(usbd_handle_t *h, usb_endp_t ep, endp_resp_e resp)
{
    return h->set_endp_response(ep, resp);
}

usb_rst_e usbd_endp_set_size(usbd_handle_t *h, usb_endp_t ep, size_t size)
{
    return h->set_endp_size(ep, size);
}

usb_rst_e usbd_endp_set_buf(usbd_handle_t *h, usb_endp_t ep, void *buf)
{
    h->endp_params[ENDP_DIR(ep) ? 1 : 0][ENDP_NUM(ep)].xfer_buffer = buf;
    return h->set_endp_buf(ep, buf);
}

usb_rst_e usbd_register_event_callback(usbd_handle_t *h, usbd_cb_event_e event, event_cb cb)
{
    if (!h) return USB_RST_NULL_PTR;
    if (event >= USBD_CB_EVENT_COUNT) return USB_RST_UNSUPPORTED_EVENT;

    h->event_cbs[event] = cb;
    return USB_RST_OK;
}

usb_rst_e usbd_register_req_callback(usbd_handle_t *h, uint8_t bmRequestType, uint8_t bRequest, ctrl_setup_cb setup,
                                     ctrl_data_cb data, ctrl_status_cb status)
{
    if (!h) return USB_RST_NULL_PTR;
    if (h->ctrl_req_cb_count >= USBD_REQ_CB_MAX_ITEMS) return USB_RST_OVERFLOW;

    size_t i;
    for (i = 0; i < h->ctrl_req_cb_count; i++)
    {
        if (h->ctrl_req_cb_items[i].bmRequestType == bmRequestType &&
            h->ctrl_req_cb_items[i].bRequest == bRequest)
        {
            /* Existing item will be overwritten */
            break;
        }
    }
    if (i >= h->ctrl_req_cb_count)
    {
        h->ctrl_req_cb_count++;
    }

    usbd_req_cb_item_t *item = &h->ctrl_req_cb_items[i];
    item->bmRequestType = bmRequestType;
    item->bRequest = bRequest;
    item->setup = setup;
    item->data = data;
    item->status = status;
    return USB_RST_OK;
}

usb_rst_e usbd_register_itf_callback(usbd_handle_t *h, uint16_t itf, ctrl_setup_cb setup, ctrl_data_cb data,
                                     ctrl_status_cb status)
{
    if (!h) return USB_RST_NULL_PTR;
    if (itf >= sizeof(h->ctrl_itf_cb_items) / sizeof(h->ctrl_itf_cb_items[0])) return USB_RST_OVERFLOW;

    h->ctrl_itf_cb_items[itf].setup = setup;
    h->ctrl_itf_cb_items[itf].data = data;
    h->ctrl_itf_cb_items[itf].status = status;
    return USB_RST_OK;
}

usb_rst_e usbd_register_data_callback(usbd_handle_t *h, usb_endp_t ep, data_xfer_cb cb)
{
    if (!h) return USB_RST_NULL_PTR;

    uint8_t num = ENDP_NUM(ep);
    if (num >= USB_MAX_EP_NUM) return USB_RST_UNSUPPORTED_ENDP;

    usbd_endp_param_t *param = &h->endp_params[ENDP_DIR(ep) ? USB_DIR_IN : USB_DIR_OUT][num];
    param->xfer_cb = cb;

    return USB_RST_OK;
}

usb_rst_e usbd_upload(usbd_handle_t *h, usb_endp_t ep, void *buf, size_t size)
{
    usbd_endp_param_t *param = &h->endp_params[1][ENDP_NUM(ep)];

    switch (param->status)
    {
    case ENDP_STA_IDLE:
        param->status = ENDP_STA_BUSY;
        param->xfer_length = size;
        param->xfer_offset = 0;
        param->xfer_buffer = buf;

        h->set_endp_buf(ep, buf);
        h->set_endp_size(ep, param->feature & USBD_ENDP_FEATURE_BURST ? size : USB_MIN(size, param->max_size));
        h->set_endp_response(ep, param->feature & USBD_ENDP_FEATURE_ISO ? ENDP_RESP_NONE : ENDP_RESP_ACK);
        return USB_RST_OK;
        break;

    case ENDP_STA_BUSY:
        return USB_RST_ENDP_BUSY;
        break;

    case ENDP_STA_STALL:
        return USB_RST_ENDP_STALL;
        break;
    }

    return USB_RST_FAILED;
}

usb_rst_e usbd_download(usbd_handle_t *h, usb_endp_t ep, void *buf, size_t size)
{
    usbd_endp_param_t *param = &h->endp_params[0][ENDP_NUM(ep)];

    switch (param->status)
    {
    case ENDP_STA_IDLE:
    case ENDP_STA_BUSY:
        param->status = ENDP_STA_BUSY;
        param->xfer_length = size;
        param->xfer_offset = 0;
        param->xfer_buffer = buf;

        h->set_endp_buf(ep, buf);
        h->set_endp_size(ep, param->feature & USBD_ENDP_FEATURE_BURST ? size : USB_MIN(size, param->max_size));
        h->set_endp_response(ep, param->feature & USBD_ENDP_FEATURE_ISO ? ENDP_RESP_NONE : ENDP_RESP_ACK);
        return USB_RST_OK;
        break;

    case ENDP_STA_STALL:
        return USB_RST_ENDP_STALL;
        break;
    }

    return USB_RST_FAILED;
}

static void _reset_parameter(usbd_handle_t *h)
{
    h->enum_status = USB_FALSE;

    for (uint8_t i = 0; i < sizeof(h->endp_params) / sizeof(h->endp_params[0]); i++)
    {
        for (uint8_t j = 0; j < sizeof(h->endp_params[0]) / sizeof(h->endp_params[0][0]); j++)
        {
            h->endp_params[i][j].status = ENDP_STA_IDLE;
            h->endp_params[i][j].feature = 0;
            h->endp_params[i][j].max_size = 0;
            h->endp_params[i][j].xfer_length = 0;
            h->endp_params[i][j].xfer_offset = 0;
            h->endp_params[i][j].xfer_buffer = NULL;
        }
    }

    h->feature.address = 0x00;
    h->feature.link_speed = USB_SPEED_FULL;
    h->feature.config_num = 0x00;
    h->feature.remote_wakeup = USB_FALSE;
}

static void _setup_process(usbd_handle_t *h, usbd_event_t *e)
{
    usb_rst_e rst = USB_RST_FAILED;
    usbd_ctrl_ctx_t *ctx = &h->ctrl_ctx;

    memcpy(&ctx->req, ctx->buf, sizeof(ctx->req));
    USB_LOGI("Setup packet: bmRequestType=0x%02X, bRequest=0x%02X, wValue=0x%04X, wIndex=0x%04X, wLength=%d.",
             ctx->req.bmRequestType, ctx->req.bRequest, ctx->req.wValue, ctx->req.wIndex, ctx->req.wLength);

    ctrl_data_cb data_cb = NULL;
    ctrl_status_cb status_cb = NULL;

    /* Priority query interface control requests */
    if (ctx->req.bmRequestType_bit.recipient == USB_REQ_RCPT_INTERFACE &&
        ctx->req.wIndex < sizeof(h->ctrl_itf_cb_items) / sizeof(h->ctrl_itf_cb_items[0]) &&
        h->ctrl_itf_cb_items[ctx->req.wIndex].setup)
    {
        rst = h->ctrl_itf_cb_items[ctx->req.wIndex].setup(h, &ctx->req, &ctx->buf_ptr, &ctx->size);
        data_cb = h->ctrl_itf_cb_items[ctx->req.wIndex].data;
        status_cb = h->ctrl_itf_cb_items[ctx->req.wIndex].status;
    }
    else
    {
        usbd_req_cb_item_t *item;
        for (size_t i = 0; i < h->ctrl_req_cb_count; i++)
        {
            item = &h->ctrl_req_cb_items[i];

            if (item->bmRequestType == ctx->req.bmRequestType && item->bRequest == ctx->req.bRequest)
            {
                rst = item->setup ? item->setup(h, &ctx->req, &ctx->buf_ptr, &ctx->size) : USB_RST_NULL_PTR;
                data_cb = item->data;
                status_cb = item->status;
                break;
            }
        }
    }

    if (rst == USB_RST_OK)
    {
        ctx->offset = 0;
        ctx->stage = CTRL_STAGE_SETUP;
        ctx->dir = ctx->req.bmRequestType_bit.direction ? USB_DIR_IN : USB_DIR_OUT;
        ctx->data_cb = data_cb;
        ctx->status_cb = status_cb;
        _ctrl_xfer_process(h, e);
    }
    else
    {
        /* Unsupported control transmission */
        USB_LOGW("Unsupported control request.");
        h->set_endp_response(0x00, ENDP_RESP_STALL);
        h->set_endp_response(0x80, ENDP_RESP_STALL);
    }
}

static void _data_process(usbd_handle_t *h, usbd_event_t *e)
{
    uint8_t ep = e->xfer.ep;
    size_t size = e->xfer.size;

    usbd_endp_param_t *param = &h->endp_params[ENDP_DIR(ep) ? USB_DIR_IN : USB_DIR_OUT][ENDP_NUM(ep)];

    param->xfer_offset += size;

    usb_bool_t need_zlp = (param->feature & USBD_ENDP_FEATURE_ZLP) ? USB_TRUE : USB_FALSE;
    usb_bool_t xfer_done = (param->xfer_offset >= param->xfer_length) ? USB_TRUE : USB_FALSE;
    usb_bool_t short_packet = (size % param->max_size || size == 0) ? USB_TRUE : USB_FALSE;

    if (short_packet || (xfer_done && !need_zlp))
    {
        param->status = ENDP_STA_IDLE;

        h->set_endp_buf(ep, param->xfer_buffer);
        h->set_endp_size(ep, param->feature & USBD_ENDP_FEATURE_BURST ? param->xfer_length :
                         USB_MIN(param->xfer_length, param->max_size));

        endp_resp_e resp = ENDP_RESP_NAK;
        if (param->xfer_cb)
        {
            resp = param->xfer_cb(h, ep, param->xfer_buffer, param->xfer_offset);
        }

        param->xfer_offset = 0;
        h->set_endp_response(ep, resp);
    }
    else
    {
        h->set_endp_buf(ep, (uint8_t *)param->xfer_buffer + param->xfer_offset);
        h->set_endp_size(ep, USB_MIN(param->xfer_length - param->xfer_offset, param->max_size));
        h->set_endp_response(ep, ENDP_RESP_ACK);
    }
}

static void _ctrl_xfer_process(usbd_handle_t *h, usbd_event_t *e)
{
    size_t size;
    usbd_ctrl_ctx_t *ctx = &h->ctrl_ctx;

    switch (ctx->stage)
    {
    case CTRL_STAGE_SETUP:
    case CTRL_STAGE_DATA:
        /* Device to host control transfer */
        if (ctx->dir)
        {
            if (ctx->size > 0)
            {
                uint8_t ep0_size = h->feature.link_speed == USB_SPEED_LOW ? 8 : h->feature.ep0_size;
                size = USB_MIN(ctx->size, ep0_size);
                memcpy(ctx->buf, (uint8_t *)ctx->buf_ptr + ctx->offset, size);
                ctx->offset += size;
                ctx->size -= size;
                h->set_endp_size(0x80, size);
                h->set_endp_response(0x80, ENDP_RESP_ACK);
                ctx->stage = CTRL_STAGE_DATA;
            }
            else
            {
                usb_rst_e rst = ctx->data_cb ? ctx->data_cb(h, &ctx->req, ctx->buf_ptr, ctx->offset) : USB_RST_OK;
                h->set_endp_size(0x80, 0);
                h->set_endp_size(0x00, sizeof(usb_req_t));
                h->set_endp_response(0x80, ENDP_RESP_ACK);
                h->set_endp_response(0x00, rst == USB_RST_OK ? ENDP_RESP_ACK : ENDP_RESP_STALL);
                ctx->stage = CTRL_STAGE_STATUS;
            }
        }
        /* Host to device control transfer */
        else
        {
            if (ctx->stage == CTRL_STAGE_DATA)
            {
                h->get_endp_size(0x00, &size);
                uint8_t ep0_size = h->feature.link_speed == USB_SPEED_LOW ? 8 : h->feature.ep0_size;
                size = USB_MIN(ctx->size, ep0_size);
                memcpy((uint8_t *)ctx->buf_ptr + ctx->offset, ctx->buf, size);
                ctx->offset += size;
                ctx->size -= size;
            }

            if (ctx->size > 0)
            {
                uint8_t ep0_size = h->feature.link_speed == USB_SPEED_LOW ? 8 : h->feature.ep0_size;
                size = USB_MIN(ctx->size, ep0_size);
                h->set_endp_size(0x00, size);
                h->set_endp_response(0x00, ENDP_RESP_ACK);
                ctx->stage = CTRL_STAGE_DATA;
            }
            else
            {
                usb_rst_e rst = ctx->data_cb ? ctx->data_cb(h, &ctx->req, ctx->buf_ptr, ctx->offset) : USB_RST_OK;
                h->set_endp_size(0x80, 0);
                h->set_endp_size(0x00, sizeof(usb_req_t));
                h->set_endp_response(0x00, ENDP_RESP_ACK);
                h->set_endp_response(0x80, rst == USB_RST_OK ? ENDP_RESP_ACK : ENDP_RESP_STALL);
                ctx->stage = CTRL_STAGE_STATUS;
            }
        }
        break;

    case CTRL_STAGE_STATUS:
        if (ctx->status_cb)
        {
            ctx->status_cb(h, &ctx->req, ctx->buf_ptr, ctx->offset);
        }
        break;
    }
}

static usb_rst_e _register_standard_request(usbd_handle_t *h)
{
    usb_rst_e rst;

    rst = usbd_register_req_callback(h, 0x00, USB_REQ_CODE_SET_ADDRESS, _set_address_setup, NULL, _set_address_status);
    if (rst != USB_RST_OK) return rst;

    rst = usbd_register_req_callback(h, 0x00, USB_REQ_CODE_SET_FEATURE, _set_feature_setup, NULL, _set_feature_status);
    if (rst != USB_RST_OK) return rst;

    rst = usbd_register_req_callback(h, 0x00, USB_REQ_CODE_CLEAR_FEATURE, _clear_feature_setup, NULL, NULL);
    if (rst != USB_RST_OK) return rst;

    rst = usbd_register_req_callback(h, 0x00, USB_REQ_CODE_SET_CONFIG, _set_config_setup, NULL, _set_config_status);
    if (rst != USB_RST_OK) return rst;

    rst = usbd_register_req_callback(h, 0x80, USB_REQ_CODE_GET_STATUS, _get_status_setup, NULL, NULL);
    if (rst != USB_RST_OK) return rst;

    rst = usbd_register_req_callback(h, 0x80, USB_REQ_CODE_GET_CONFIGURATION, _get_config_setup, NULL, NULL);
    if (rst != USB_RST_OK) return rst;

    rst = usbd_register_req_callback(h, 0x02, USB_REQ_CODE_SET_FEATURE, _set_feature_setup, NULL, NULL);
    if (rst != USB_RST_OK) return rst;

    rst = usbd_register_req_callback(h, 0x02, USB_REQ_CODE_CLEAR_FEATURE, _clear_feature_setup, NULL, NULL);
    if (rst != USB_RST_OK) return rst;

    rst = usbd_register_req_callback(h, 0x82, USB_REQ_CODE_GET_STATUS, _get_status_setup, NULL, NULL);
    if (rst != USB_RST_OK) return rst;

    return USB_RST_OK;
}

static usb_rst_e _clear_feature_setup(usbd_handle_t *h, const usb_req_t *req, void **buf, size_t *size)
{
    if (req->bmRequestType_bit.recipient == USB_REQ_RCPT_DEVICE &&
        req->wValue == USB_REQ_FEATURE_REMOTE_WAKEUP)
    {
        h->feature.remote_wakeup = USB_FALSE;
        return USB_RST_OK;
    }

    if (req->bmRequestType_bit.recipient == USB_REQ_RCPT_ENDPOINT &&
        req->wValue == USB_REQ_FEATURE_ENDPOINT_HALT)
    {
        usb_endp_t ep = req->wIndex & 0x00FF;
        uint8_t dir = ENDP_DIR(ep);

        usbd_endp_param_t *param = &h->endp_params[dir ? USB_DIR_IN : USB_DIR_OUT][ENDP_NUM(ep)];
        param->status = ENDP_STA_IDLE;
        param->xfer_length = 0;
        param->xfer_offset = 0;
        param->xfer_buffer = NULL;
        h->set_endp_toggle(ep, ENDP_TOG_DATA0);
        h->set_endp_response(ep, dir ? ENDP_RESP_NAK : ENDP_RESP_ACK);
        return USB_RST_OK;
    }

    return USB_RST_FAILED;
}

static usb_rst_e _get_config_setup(usbd_handle_t *h, const usb_req_t *req, void **buf, size_t *size)
{
    h->ctrl_ctx.temp_data = h->feature.config_num;
    *buf = (void *)&h->ctrl_ctx.temp_data;
    *size = USB_MIN(sizeof(uint8_t), req->wLength);
    return USB_RST_OK;
}

static usb_rst_e _get_status_setup(usbd_handle_t *h, const usb_req_t *req, void **buf, size_t *size)
{
    if (req->bmRequestType_bit.recipient == USB_REQ_RCPT_DEVICE)
    {
        h->ctrl_ctx.temp_data = (h->feature.self_powered ? 0x0001 : 0x0000) |
                                (h->feature.remote_wakeup ? 0x0002 : 0x0000);
        *buf = (void *)&h->ctrl_ctx.temp_data;
        *size = USB_MIN(sizeof(uint16_t), req->wLength);
        return USB_RST_OK;
    }

    if (req->bmRequestType_bit.recipient == USB_REQ_RCPT_ENDPOINT)
    {
        usb_rst_e rst;
        usb_endp_t ep = req->wIndex & 0x00FF;
        endp_resp_e resp;

        rst = h->get_endp_response(ep, &resp);
        if (rst != USB_RST_OK)
        {
            return rst;
        }

        h->ctrl_ctx.temp_data = resp == ENDP_RESP_STALL ? 0x0001 : 0x0000;
        *buf = (void *)&h->ctrl_ctx.temp_data;
        *size = USB_MIN(sizeof(uint16_t), req->wLength);
        return USB_RST_OK;
    }

    return USB_RST_FAILED;
}

static usb_rst_e _set_address_setup(usbd_handle_t *h, const usb_req_t *req, void **buf, size_t *size)
{
    *size = 0;
    h->feature.address = req->wValue & 0x00FF;
    return USB_RST_OK;
}

static usb_rst_e _set_config_setup(usbd_handle_t *h, const usb_req_t *req, void **buf, size_t *size)
{
    *size = 0;
    h->feature.config_num = req->wValue & 0x00FF;
    return USB_RST_OK;
}

static usb_rst_e _set_feature_setup(usbd_handle_t *h, const usb_req_t *req, void **buf, size_t *size)
{
    if (req->bmRequestType_bit.recipient == USB_REQ_RCPT_DEVICE &&
        req->wValue == USB_REQ_FEATURE_REMOTE_WAKEUP)
    {
        h->feature.remote_wakeup = USB_TRUE;
        return USB_RST_OK;
    }

    if (req->bmRequestType_bit.recipient == USB_REQ_RCPT_DEVICE &&
        req->wValue == USB_REQ_FEATURE_TEST_MODE)
    {
        uint8_t test_select = (req->wIndex >> 8) & 0x00FF;
        if ((req->wIndex & 0x00FF) == 0x0000 && test_select > USB_REQ_TEST_SELECT_RESERVED &&
            test_select <= USB_REQ_TEST_SELECT_FORCE_ENABLE)
        {
            return USB_RST_OK;
        }
    }

    if (req->bmRequestType_bit.recipient == USB_REQ_RCPT_ENDPOINT &&
        req->wValue == USB_REQ_FEATURE_ENDPOINT_HALT)
    {
        usb_endp_t ep = req->wIndex & 0x00FF;
        h->endp_params[ENDP_DIR(ep) ? USB_DIR_IN : USB_DIR_OUT][ENDP_NUM(ep)].status = ENDP_STA_STALL;
        h->set_endp_response(ep, ENDP_RESP_STALL);
        return USB_RST_OK;
    }

    return USB_RST_FAILED;
}

static void _set_address_status(usbd_handle_t *h, const usb_req_t *req, void *buf, size_t size)
{
    h->set_address(h->feature.address);
}

static void _set_config_status(usbd_handle_t *h, const usb_req_t *req, void *buf, size_t size)
{
    h->enum_status = USB_TRUE;
    USB_LOGI("Enumeration completed, config is %d.", h->feature.config_num);

    if (h->event_cbs[USBD_CB_EVENT_ENUM_COMPLETED])
    {
        h->event_cbs[USBD_CB_EVENT_ENUM_COMPLETED](h, 0);
    }
}

static void _set_feature_status(usbd_handle_t *h, const usb_req_t *req, void *buf, size_t size)
{
    if (req->bmRequestType_bit.recipient == USB_REQ_RCPT_DEVICE &&
        req->wValue == USB_REQ_FEATURE_TEST_MODE)
    {
        uint8_t test_select = (req->wIndex >> 8) & 0x00FF;

        switch (test_select)
        {
        case USB_REQ_TEST_SELECT_J:
            USB_LOGI("Enter test mode: TEST_J.");
            break;

        case USB_REQ_TEST_SELECT_K:
            USB_LOGI("Enter test mode: TEST_K.");
            break;

        case USB_REQ_TEST_SELECT_SE0_NAK:
            USB_LOGI("Enter test mode: TEST_SE0_NAK.");
            break;

        case USB_REQ_TEST_SELECT_PACKET:
            USB_LOGI("Enter test mode: TEST_PACKET.");
            break;

        case USB_REQ_TEST_SELECT_FORCE_ENABLE:
            USB_LOGI("Enter test mode: TEST_FORCE_ENABLE.");
            break;
        }

        if (h->test_mode_ctrl)
        {
            h->test_mode_ctrl(test_select);
        }
    }
}

#endif // USB_DEVICE_DRIVER_EN
