/*
 * Copyright (C) 2024 Inspur Group Co., Ltd. Unpublished
 *
 * Inspur Group Co., Ltd.
 * Proprietary & Confidential
 *
 * This source code and the algorithms implemented therein constitute
 * confidential information and may comprise trade secrets of Inspur
 * or its associates, and any use thereof is subject to the terms and
 * conditions of the Non-Disclosure Agreement pursuant to which this
 * source code was originally received.
 */

#ifndef _HRSERVER_OBJECT_
#define _HRSERVER_OBJECT_

#include <stddef.h>

#include "hrif_network.h"
#include "j2sobject.h"

typedef struct j2sobject hrobject;

typedef struct hrobject_ipv4 {
    J2SOBJECT_DECLARE_OBJECT;
    hrif_ipv4_t data;
    /*char address[HRIF_IP_ADDRESS_LEN];
    char gateway[HRIF_IP_ADDRESS_LEN];
    char netmask[HRIF_IP_ADDRESS_LEN];
    char dns[HRIF_IP_ADDRESS_LEN];
    char dns2[HRIF_IP_ADDRESS_LEN];*/
} hrobject_ipv4_t;

typedef struct hrobject_ipv6 {
    J2SOBJECT_DECLARE_OBJECT;
    char address[HRIF_IP_ADDRESS_LEN];
    int mask;

    char local_address[HRIF_IP_ADDRESS_LEN];
    int local_mask;
} hrobject_ipv6_t;

// only stats data, you can calc speed
// /sys/class/network/iface/statistics
typedef struct hrobject_network_stats {
    J2SOBJECT_DECLARE_OBJECT;
    uint32_t rx_bytes;
    uint32_t tx_bytes;
    uint32_t rx_packets;
    uint32_t tx_packets;
    uint32_t rx_dropped;
    uint32_t tx_dropped;
    // uint32_t rx_errors;
    // uint32_t tx_errors;
    uint32_t crc_errors;
} hrobject_network_stats_t;

typedef struct hrobject_network_device {
    J2SOBJECT_DECLARE_OBJECT;
    char name[HRIF_INTERFACE_NAME_LEN];
    char mac[HRIF_MAC_STR_LEN];
    int mtu;  // 1500

    hrobject_ipv4_t v4;
    hrobject_ipv6_t v6;
} hrobject_network_device_t;

typedef struct hrobject_pppoe {
    J2SOBJECT_DECLARE_OBJECT;
    char username[HRIF_WAN_PPPOE_USERNAME_LEN];
    char password[HRIF_WAN_PPPOE_PASSWORD_LEN];
    char* dynamic_str;
} hrobject_pppoe_t;

typedef struct hrobject_wan {
    J2SOBJECT_DECLARE_OBJECT;
    int index;  // donot use index
    char name[HRIF_WAN_NAME_LEN];
    hrif_workmode_e workmode;

    hrif_protocol_e v4_proto;
    hrobject_pppoe_t pppoe;
    hrobject_network_device_t device;
    hrobject_network_stats_t stats;
} hrobject_wan_t;

#if 0
typedef struct hrobject_wan_state {
    J2SOBJECT_DECLARE_OBJECT;
    hrif_wan_state_t data;
} hrobject_wan_state;

typedef struct hrobject_lanhost {
    J2SOBJECT_DECLARE_OBJECT;
    hrif_lanhost_t data;
} hrobject_lanhost_t;
#endif

extern struct j2sobject_prototype hrobject_wan_prototype;

#define HROBJECT_WAN_NEW() \
  (hrobject_wan_t *)j2sobject_create(&hrobject_wan_prototype)

#define HROBJECT_WAN_DEL(self) \
  j2sobject_free(J2SOBJECT(self))

#define HROBJECT_WAN_RESET(self) \
  j2sobject_reset(J2SOBJECT(self));
// phywan

#endif  //_HRSERVER_OBJECT_
