/********************************** (C) COPYRIGHT *******************************
* File Name          : usb_config.h
* Author             : WCH
* Version            : V1.2
* Date               : 2026/05/26
* Description        : Usb driver config headfile.
*********************************************************************************
* Copyright (c) 2026 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#ifndef __USB_CONFIG_H_
#define __USB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* @include */
#include <stdio.h>

#include "usb_define.h"

/* @define */
// usb driver common configuration macro definitions.
#define USB_DEVICE_DRIVER_EN
#define USB_HOST_DRIVER_EN

// usb driver log configuration macro definitions.
#define USB_DRIVER_LOG_INFO_EN
#define USB_DRIVER_LOG_WARNING_EN
#define USB_DRIVER_LOG_ERROR_EN

#ifdef USB_DRIVER_LOG_INFO_EN
#define USB_LOGI(format, ...)           printf("[USB INFO]: " format "\r\n", ##__VA_ARGS__)
#else
#define USB_LOGI(format, ...)
#endif

#ifdef USB_DRIVER_LOG_WARNING_EN
#define USB_LOGW(format, ...)           printf("[USB WARNING]: " format "\r\n", ##__VA_ARGS__)
#else
#define USB_LOGW(format, ...)
#endif

#ifdef USB_DRIVER_LOG_ERROR_EN
#define USB_LOGE(format, ...)           printf("[USB ERROR]: " format "\r\n", ##__VA_ARGS__)
#else
#define USB_LOGE(format, ...)
#endif

// usb device driver configuration macro definitions.
#define USBD_REQ_CB_MAX_ITEMS           32
#define USBD_ITF_CB_MAX_ITEMS           16

#define USBD_SELF_POWERED               USB_FALSE

// usb host driver configuration macro definitions.
#define USBH_MAX_DEVICES                8
#define USBH_MAX_DRIVERS                8
#define USBH_MAX_ENDPOINTS              32
#define USBH_ENUM_BUFFER_SIZE           1024

#ifdef __cplusplus
}
#endif

#endif
