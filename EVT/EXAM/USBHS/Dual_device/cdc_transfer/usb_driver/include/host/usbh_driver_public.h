/********************************** (C) COPYRIGHT *******************************
* File Name          : usbh_driver_public.h
* Author             : WCH
* Version            : V1.2
* Date               : 2026/05/26
* Description        : Usb host driver public headfile.
*********************************************************************************
* Copyright (c) 2026 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#ifndef __USBH_DRIVER_PUBLIC_H_
#define __USBH_DRIVER_PUBLIC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* @include */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "usb_define.h"
#include "usb_config.h"

/* @struct */

typedef struct usbh_device usbh_device_t;
typedef struct usbh_driver usbh_driver_t;
typedef struct usbh_handle usbh_handle_t;
typedef struct usbh_dev_ops_funcs usbh_dev_ops_funcs_t;

/* @callback */
typedef void(*usbh_data_xfer_cb)(void *drv, usb_bool_t rst, usb_endp_t ep, void *buf, size_t length);
typedef void(*usbh_ctrl_xfer_cb)(void *drv, usb_bool_t rst, const usb_req_t *req, void *buf, uint16_t length);

/* @function declaration */

/**
 * @brief Usb host driver initialization.
 *
 * @param h Usb host driver handle object.
 * @return usb_rst_e Execution result.
 * @note Make sure the handle has been initialized before executing this function.
 */
usb_rst_e usbh_drv_init(usbh_handle_t *h);

/**
 * @brief Usb host driver core task.
 *
 * @param h Usb host driver handle object.
 * @return usb_rst_e Execution result.
 * @note This function is executed in the main loop.
 */
usb_rst_e usbh_drv_task(usbh_handle_t *h);

/**
 * @brief Open usb host controller.
 *
 * @param h Usb host driver handle object.
 * @return usb_rst_e Execution result.
 */
usb_rst_e usbh_open(usbh_handle_t *h);

/**
 * @brief Close usb host controller.
 *
 * @param h Usb host driver handle object.
 * @return usb_rst_e Execution result.
 */
usb_rst_e usbh_close(usbh_handle_t *h);

usb_rst_e usbh_register_driver(usbh_handle_t *h, const usbh_driver_t *driver);
usb_rst_e usbh_unregister_driver(usbh_handle_t *h, const usbh_driver_t *driver);
usb_rst_e usbh_endp_open(usbh_device_t *dev, const desc_endpoint_t *desc);
usb_rst_e usbh_endp_close(usbh_device_t *dev, usb_endp_t ep);
usb_rst_e usbh_ctrl_xfer(usbh_device_t *dev, void *drv, const usb_req_t *req, void *buf, usbh_ctrl_xfer_cb cb);
usb_rst_e usbh_data_xfer(usbh_device_t *dev, void *drv, usb_endp_t ep, void *buf, size_t length, usbh_data_xfer_cb cb);
usb_bool_t usbh_is_ctrl_xfering(usbh_device_t *dev);
usb_bool_t usbh_is_data_xfering(usbh_device_t *dev, usb_endp_t ep);
void usbh_device_insert(usbh_handle_t *h, uint8_t hub_addr, uint8_t hub_port, const usbh_dev_ops_funcs_t *ops);
void usbh_device_remove(usbh_handle_t *h, uint8_t hub_addr, uint8_t hub_port);

#ifdef __cplusplus
}
#endif

#endif
