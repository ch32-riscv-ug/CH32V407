/********************************** (C) COPYRIGHT *******************************
* File Name          : hubh.h
* Author             : WCH
* Version            : V1.2
* Date               : 2026/05/26
* Description        : Hub host class header file.
*********************************************************************************
* Copyright (c) 2026 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#ifndef __HUBH_H_
#define __HUBH_H_

#ifdef __cplusplus
extern "C" {
#endif

/* @include */
#include <stdint.h>

#include "usb_define.h"
#include "class/hub/hub.h"

/* @extern */
extern const usbh_driver_t hubh_driver;

/* @function declaration */
void hubh_drv_init(void);
void hubh_drv_task(void);

#ifdef __cplusplus
}
#endif

#endif
