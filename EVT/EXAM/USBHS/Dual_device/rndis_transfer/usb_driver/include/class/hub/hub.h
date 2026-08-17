/********************************** (C) COPYRIGHT *******************************
* File Name          : hub.h
* Author             : WCH
* Version            : V1.2
* Date               : 2026/05/26
* Description        : Hub class define.
*********************************************************************************
* Copyright (c) 2026 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#ifndef __HUB_H_
#define __HUB_H_

#ifdef __cplusplus
extern "C" {
#endif

/* @include */
#include <stdint.h>

/* @enum */
typedef enum
{
    HUB_FEATURE_C_HUB_LOCAL_POWER = 0,
    HUB_FEATURE_C_HUB_OVER_CURRENT = 1,

    HUB_FEATURE_PORT_CONNECTION = 0,
    HUB_FEATURE_PORT_ENABLE = 1,
    HUB_FEATURE_PORT_SUSPEND = 2,
    HUB_FEATURE_PORT_OVER_CURRENT = 3,
    HUB_FEATURE_PORT_RESET = 4,
    HUB_FEATURE_PORT_POWER = 8,
    HUB_FEATURE_PORT_LOW_SPEED = 9,
    HUB_FEATURE_C_PORT_CONNECTION = 16,
    HUB_FEATURE_C_PORT_ENABLE = 17,
    HUB_FEATURE_C_PORT_SUSPEND = 18,
    HUB_FEATURE_C_PORT_OVER_CURRENT = 19,
    HUB_FEATURE_C_PORT_RESET = 20,
    HUB_FEATURE_PORT_TEST = 21,
    HUB_FEATURE_PORT_INDICATOR = 22,
} hub_feature_e;

/* @struct */
typedef struct
{
    union
    {
        uint16_t hub_status;

        struct
        {
            uint16_t local_power : 1;
            uint16_t over_current : 1;
            uint16_t reserved : 14;
        } hub_status_bits;
    };

    union
    {
        uint16_t hub_change;

        struct
        {
            uint16_t local_power : 1;
            uint16_t over_current : 1;
            uint16_t reserved : 14;
        } hub_change_bits;
    };
} hub_status_t;

typedef struct
{
    union
    {
        uint16_t port_status;

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
        } port_status_bits;
    };

    union
    {
        uint16_t port_change;
        struct
        {
            uint16_t connect : 1;
            uint16_t enable : 1;
            uint16_t suspend : 1;
            uint16_t over_current : 1;
            uint16_t reset : 1;
        } port_change_bits;
    };
} hub_port_status_t;

/* Start single-byte alignment */
#pragma pack(1)

typedef struct
{
    uint8_t bDescLength;
    uint8_t bDescriptorType;
    uint8_t bNbrPorts;
    uint16_t wHubCharacteristics;
    uint8_t bPwrOn2PwrGood;
    uint8_t bHubContrCurrent;
    uint8_t DeviceRemovable;
    uint8_t PortPwrCtrlMask;
} desc_hub_t;

/* End single-byte alignment */
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif
