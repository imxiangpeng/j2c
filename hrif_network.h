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

#ifndef _HRIF_NETWORK_
#define _HRIF_NETWORK_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define HRIF_WAN_NAME_LEN 64
#define HRIF_MAC_STR_LEN 18
#define HRIF_INTERFACE_NAME_LEN 24

#define HRIF_WAN_PPPOE_USERNAME_LEN 64
#define HRIF_WAN_PPPOE_PASSWORD_LEN 64

#define HRIF_IP_ADDRESS_LEN 16
#define HRIF_IP6_ADDRESS_LEN 48  // 40

#define HRIF_HOSTNAME_LEN 64

typedef enum hr_wan_proto {
    HRIF_WAN_PROTO_NONE,
    HRIF_WAN_PROTO_STATIC,
    HRIF_WAN_PROTO_DHCP,
    HRIF_WAN_PROTO_IPOE,
    HRIF_WAN_PROTO_PPPOE,
    HRIF_WAN_PROTO_V6_STATIC,
    HRIF_WAN_PROTO_V6_SLAAC,
    HRIF_WAN_PROTO_V6_DHCPV6,
    HRIF_WAN_PROTO_LAST
} hrif_protocol_e;
typedef enum hr_workmode {
    HRIF_WORKMODE_UNKNOWN = 0,
    HRIF_WORKMODE_BRIDGE,
    HRIF_WORKMODE_ROUTER
} hrif_workmode_e;

typedef enum hr_connstatus {
    HRIF_CONNSTATUS_UNCONFIGURED,
    HRIF_CONNSTATUS_CONNECTING,
    HRIF_CONNSTATUS_CONNECTED,
    HRIF_CONNSTATUS_DISCONNECTING,
    HRIF_CONNSTATUS_DISCONNECTED,
} hrif_connstatus_e;

typedef enum hr_duplex {
    HRIF_DUPLEX_FULL
} hrif_duplex_e;

typedef struct hr_ipv4 {
    char address[HRIF_IP_ADDRESS_LEN];
    char gateway[HRIF_IP_ADDRESS_LEN];
    char netmask[HRIF_IP_ADDRESS_LEN];
    char dns[HRIF_IP_ADDRESS_LEN];
    char dns2[HRIF_IP_ADDRESS_LEN];
} hrif_ipv4_t;

typedef struct hr_ipv6 {
    char address[HRIF_IP6_ADDRESS_LEN];
    uint32_t netmask;
    char gateway[HRIF_IP6_ADDRESS_LEN];
    char prefix[HRIF_IP6_ADDRESS_LEN];
    char dns[HRIF_IP6_ADDRESS_LEN];
    char dns2[HRIF_IP6_ADDRESS_LEN];
} hrif_ipv6_t;

typedef struct hr_intf_statistic {
    uint32_t total_sent_bytes;
    uint32_t total_received_bytes;
    uint32_t total_sent_packets;
    uint32_t total_received_packets;
    uint32_t rx_kbps;
    uint32_t tx_kbps;
} hrif_intf_statistic_t;

typedef struct hr_wan {
    int index;
    char name[HRIF_WAN_NAME_LEN];
    hrif_workmode_e workmode;
    char mac[HRIF_MAC_STR_LEN];

    // uint32_t speed;
    // hr_duplex duplex;
    // uint32_t rx_kbps;
    // uint32_t tx_kbps;
    // uint32_t online_time;

    char iface[HRIF_INTERFACE_NAME_LEN];
    // pppoe
    char pppoe_username[HRIF_WAN_PPPOE_USERNAME_LEN];
    char pppoe_password[HRIF_WAN_PPPOE_PASSWORD_LEN];

    // ipv4
    hrif_protocol_e v4_proto;
    // hr_connstatus v4_status;
    hrif_ipv4_t v4;
    /*char v4_address[HRIF_IP_ADDRESS_LEN];
    char v4_gateway[HRIF_IP_ADDRESS_LEN];
    char v4_netmask[HRIF_IP_ADDRESS_LEN];
    char v4_dns[HRIF_IP_ADDRESS_LEN];
    char v4_dns2[HRIF_IP_ADDRESS_LEN];*/

    // ipv6
    hrif_protocol_e v6_proto;
    char v6_address[HRIF_IP6_ADDRESS_LEN];
    uint32_t v6_netmask;
    char v6_gateway[HRIF_IP6_ADDRESS_LEN];
    char v6_prefix[HRIF_IP6_ADDRESS_LEN];
    char v6_dns[HRIF_IP6_ADDRESS_LEN];
    char v6_dns2[HRIF_IP6_ADDRESS_LEN];
} hrif_wan_t;

#endif  //_HRIF_NETWORK_
