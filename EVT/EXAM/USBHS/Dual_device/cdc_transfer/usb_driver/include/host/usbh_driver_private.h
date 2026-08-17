/********************************** (C) COPYRIGHT *******************************
* File Name          : usbh_driver_private.h
* Author             : WCH
* Version            : V1.2
* Date               : 2026/05/26
* Description        : Usb host driver private headfile.
*********************************************************************************
* Copyright (c) 2026 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#ifndef __USBH_DRIVER_PRIVATE_H_
#define __USBH_DRIVER_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* @include */
#include <stdint.h>
#include <stddef.h>

#include "usb_define.h"
#include "usb_config.h"
#include "host/usbh_driver_public.h"

typedef enum
{
    USBH_DEV_FEATURE_SUSPEND = 2,
    USBH_DEV_FEATURE_RESET = 4,
    USBH_DEV_FEATURE_POWER = 8,
    USBH_DEV_FEATURE_TEST = 21,
    USBH_DEV_FEATURE_INDICATOR = 22,
} usbh_dev_feature_e;

/* @callback */
typedef usb_rst_e(*usbh_drv_bind_cb)(usbh_handle_t *h, usbh_device_t *dev, const uint8_t *desc, uint16_t length);
typedef usb_rst_e(*usbh_drv_unbind_cb)(usbh_handle_t *h, usbh_device_t *dev);

/* @struct */
typedef struct usbh_xfer_unit
{
    uint8_t pid;
    uint8_t endp;
    uint8_t token;
    uint8_t toggle;
    uint8_t dev_addr;
    uint8_t endp_addr;
    uint16_t length;
    void *buffer;
    void *object;
    split_data_t split_data;
    struct usbh_xfer_unit *next;
} usbh_xfer_unit_t;

typedef struct
{
    usb_bool_t xfering;
    uint8_t toggle;
    uint16_t retry;
    uint32_t tick;
    size_t length;
    size_t offset;
    void *buffer;
    void *driver;
    usbh_data_xfer_cb callback;
    usbh_xfer_unit_t xfer_unit;
} usbh_data_xfer_ctx_t;

typedef struct
{
    uint8_t stage;
    uint8_t toggle;
    usb_bool_t xfering;
    uint16_t retry;
    uint16_t length;
    uint16_t offset;
    uint32_t tick;
    void *buffer;
    void *driver;
    const usb_req_t *request;
    usbh_ctrl_xfer_cb callback;
    usbh_xfer_unit_t xfer_unit;
} usbh_ctrl_xfer_ctx_t;

typedef struct usbh_endpoint
{
    uint8_t type;
    uint8_t address;
    uint16_t interval;
    uint16_t max_size;
    usbh_data_xfer_ctx_t data_xfer_ctx;
    struct usbh_endpoint *next;
} usbh_endpoint_t;

typedef struct
{
    union
    {
        uint16_t status;

        struct
        {
            uint16_t connect : 1;
            uint16_t enable : 1;
            uint16_t suspend : 1;
            uint16_t over_current : 1;
            uint16_t reset : 1;
            uint16_t reserved1 : 3;
            uint16_t power : 1;
            uint16_t low_speed : 1;
            uint16_t high_speed : 1;
            uint16_t test_mode : 1;
            uint16_t port_indicator : 1;
            uint16_t reserved2 : 3;
        };
    };
} usbh_dev_status_t;

struct usbh_dev_ops_funcs
{
    usb_rst_e(*set_feature)(usbh_device_t *dev, usbh_dev_feature_e feature);
    usb_rst_e(*get_status)(usbh_device_t *dev, usbh_dev_status_t *status);
};

struct usbh_device
{
    uint8_t address;
    uint8_t hub_addr;
    uint8_t hub_port;
    uint8_t ep0_size;
    uint8_t enum_stage;
    uint16_t language_id;
    usb_speed_e speed;
    usb_speed_e host_speed;
    usbh_endpoint_t *endp_list;
    usbh_endpoint_t *endp_array[2][USBH_MAX_ENDPOINTS];
    usbh_ctrl_xfer_ctx_t ctrl_xfer_ctx;
    desc_device_t device_desc;
    const usbh_dev_ops_funcs_t *ops;
    struct usbh_device *next;
};

struct usbh_driver
{
    uint16_t vid;
    uint16_t pid;

    uint8_t dev_class;
    uint8_t dev_subclass;
    uint8_t dev_protocol;

    uint8_t itf_class;
    uint8_t itf_subclass;
    uint8_t itf_protocol;

    // bit0: vid, bit1: pid.
    // bit2: dev_class, bit3: dev_subclass, bit4: dev_protocol.
    // bit5: itf_class, bit6: itf_subclass, bit7: itf_protocol.
    // other: reserved.
    uint16_t match_bit;

    usbh_drv_bind_cb bind_cb;
    usbh_drv_unbind_cb unbind_cb;
};

struct usbh_handle
{
    usb_bool_t xfering;

    usb_speed_e speed;
    uint32_t tick;
    uint32_t bus_address_bitmap[4];

    usb_req_t enum_request;
    uint8_t enum_buffer[USBH_ENUM_BUFFER_SIZE];

    usbh_device_t *device_list;
    usbh_xfer_unit_t *xfer_list;
    const usbh_driver_t *driver_list[USBH_MAX_DRIVERS];

    usb_rst_e(*open)(void);
    usb_rst_e(*close)(void);
    usb_rst_e(*start_transfer)(void);
};

#ifdef __cplusplus
}
#endif

#endif
