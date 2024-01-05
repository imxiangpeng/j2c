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

#include "hrserver_object.h"

#include <stdio.h>
#include <string.h>

#include "hrif_network.h"
#include "j2sobject.h"

#define _J2SOBJECT_IPV4_DATA_OFFSET(ele) \
    offsetof(hrif_ipv4_t, ele) + sizeof(struct j2sobject)
#define _J2SOBJECT_IP_DATA_LEN(ele) \
    sizeof(((hrif_ipv4_t *)0)->ele)

#define _J2SOBJECT_DEVICE_DATA_OFFSET(ele) \
    offsetof(hrobject_network_device_t, ele)
#define _J2SOBJECT_DEVICE_DATA_LEN(ele) \
    sizeof(((hrobject_network_device_t *)0)->ele)

#define _J2SOBJECT_PPPOE_DATA_OFFSET(ele) \
    offsetof(hrobject_pppoe_t, ele)
#define _J2SOBJECT_PPPOE_DATA_LEN(ele) \
    sizeof(((hrobject_pppoe_t *)0)->ele)


#define _J2SOBJECT_WAN_DATA_OFFSET(ele) \
    offsetof(hrobject_wan_t, ele)

#define _J2SOBJECT_WAN_DATA_LEN(ele) \
    sizeof(((hrobject_wan_t *)0)->ele)

#define _J2SOBJECT_WAN_STATE_DATA_OFFSET(ele) \
    offsetof(struct hr_wan_state, ele) + sizeof(struct j2sobject)


static int hrobject_ipv4_ctor(struct j2sobject *obj);
static int hrobject_ipv4_dtor(struct j2sobject *obj);

static int hrobject_network_device_ctor(struct j2sobject *obj);
static int hrobject_network_device_dtor(struct j2sobject *obj);

static int hrobject_pppoe_ctor(struct j2sobject *obj);
static int hrobject_pppoe_dtor(struct j2sobject *obj);

static int hrobject_wan_ctor(struct j2sobject *obj);
static int hrobject_wan_dtor(struct j2sobject *obj);

// clang-format off
static struct j2sobject_prototype _hrobject_ipv4_prototype = {
    .name = "ipv4",
    .type = J2S_OBJECT,
    .size = sizeof(hrobject_ipv4_t),
    .ctor = hrobject_ipv4_ctor,
    .dtor = hrobject_ipv4_dtor
};

// must special offset_len when you using char array
static struct j2sobject_fields_prototype _hrobject_ipv4_fields_prototype[] = {
    {.name = "address",              .type = J2S_STRING,    .offset = _J2SOBJECT_IPV4_DATA_OFFSET(address),              .offset_len = _J2SOBJECT_IP_DATA_LEN(address)},
    {.name = "gateway",              .type = J2S_STRING,    .offset = _J2SOBJECT_IPV4_DATA_OFFSET(gateway),              .offset_len = _J2SOBJECT_IP_DATA_LEN(gateway)},
    {.name = "netmask",              .type = J2S_STRING,    .offset = _J2SOBJECT_IPV4_DATA_OFFSET(netmask),              .offset_len = _J2SOBJECT_IP_DATA_LEN(netmask)},
    {.name = "dns",                  .type = J2S_STRING,    .offset = _J2SOBJECT_IPV4_DATA_OFFSET(dns),              .offset_len = _J2SOBJECT_IP_DATA_LEN(dns)},
    {.name = "dns2",                 .type = J2S_STRING,    .offset = _J2SOBJECT_IPV4_DATA_OFFSET(dns2),              .offset_len = _J2SOBJECT_IP_DATA_LEN(dns2)},
    {0}
};
static struct j2sobject_prototype _hrobject_network_device_prototype = {
    .name = "network_device",
    .type = J2S_OBJECT,
    .size = sizeof(hrobject_network_device_t),
    .ctor = hrobject_network_device_ctor,
    .dtor = hrobject_network_device_dtor
};

// must special offset_len when you using char array
static struct j2sobject_fields_prototype _hrobject_network_device_fields_prototype[] = {
    {.name = "name",              .type = J2S_STRING,    .offset = _J2SOBJECT_DEVICE_DATA_OFFSET(name),              .offset_len = _J2SOBJECT_DEVICE_DATA_LEN(name)},
    {.name = "mac",              .type = J2S_STRING,    .offset = _J2SOBJECT_DEVICE_DATA_OFFSET(mac),              .offset_len = _J2SOBJECT_DEVICE_DATA_LEN(mac)},
    {.name = "mtu",             .type = J2S_INT,       .offset = _J2SOBJECT_DEVICE_DATA_OFFSET(mtu),             .offset_len = 0},
    {.name = "v4",                .type = J2S_OBJECT,    .offset = _J2SOBJECT_DEVICE_DATA_OFFSET(v4),          .offset_len = _J2SOBJECT_DEVICE_DATA_LEN(v4), .proto = &_hrobject_ipv4_prototype},
    {0}
};

static struct j2sobject_prototype _hrobject_pppoe_prototype = {
    .name = "pppoe",
    .type = J2S_OBJECT,
    .size = sizeof(hrobject_pppoe_t),
    .ctor = hrobject_pppoe_ctor,
    .dtor = hrobject_pppoe_dtor
};

// must special offset_len when you using char array
static struct j2sobject_fields_prototype _hrobject_pppoe_fields_prototype[] = {
    {.name = "username",              .type = J2S_STRING,    .offset = _J2SOBJECT_PPPOE_DATA_OFFSET(username),              .offset_len = _J2SOBJECT_PPPOE_DATA_LEN(username)},
    {.name = "password",              .type = J2S_STRING,    .offset = _J2SOBJECT_PPPOE_DATA_OFFSET(password),              .offset_len = _J2SOBJECT_PPPOE_DATA_LEN(password)},
    {0}
};

/*static*/ struct j2sobject_prototype hrobject_wan_prototype = {
    .name = "wan",
    .type = J2S_OBJECT,
    .size = sizeof(hrobject_wan_t),
    .ctor = hrobject_wan_ctor,
    .dtor = hrobject_wan_dtor
};

// must special offset_len when you using char array
static struct j2sobject_fields_prototype _hrobject_wan_fields_prototype[] = {
    {.name = "index",             .type = J2S_INT,       .offset = _J2SOBJECT_WAN_DATA_OFFSET(index),             .offset_len = 0},
    {.name = "name",              .type = J2S_STRING,    .offset = _J2SOBJECT_WAN_DATA_OFFSET(name),              .offset_len = _J2SOBJECT_WAN_DATA_LEN(name)},
    {.name = "workmode",          .type = J2S_INT,       .offset = _J2SOBJECT_WAN_DATA_OFFSET(workmode),          .offset_len = 0},
    {.name = "v4_proto",          .type = J2S_INT,       .offset = _J2SOBJECT_WAN_DATA_OFFSET(v4_proto),          .offset_len = 0 },
    {.name = "device",            .type = J2S_OBJECT,    .offset = _J2SOBJECT_WAN_DATA_OFFSET(device),          .offset_len = _J2SOBJECT_WAN_DATA_LEN(device), .proto = &_hrobject_network_device_prototype},
    {.name = "pppoe",            .type = J2S_OBJECT,    .offset = _J2SOBJECT_WAN_DATA_OFFSET(pppoe),          .offset_len = _J2SOBJECT_WAN_DATA_LEN(pppoe), .proto = &_hrobject_pppoe_prototype},
    {0}
};
// clang-format on

int hrobject_ipv4_ctor(struct j2sobject *obj) {
    if (!obj)
        return -1;
    obj->name = "ipv4";
    obj->field_protos = _hrobject_ipv4_fields_prototype;
    return 0;
}

static int hrobject_ipv4_dtor(struct j2sobject *obj) {
    if (!obj) return 0;

    return 0;
}

static int hrobject_network_device_ctor(struct j2sobject *obj) {
    if (!obj)
        return -1;
    obj->name = "network_device";
    obj->field_protos = _hrobject_network_device_fields_prototype;
    return 0;
}

static int hrobject_network_device_dtor(struct j2sobject *obj) {
    if (!obj) return 0;

    return 0;
}

static int hrobject_pppoe_ctor(struct j2sobject *obj) {
    if (!obj)
        return -1;
    obj->name = "pppoe";
    obj->field_protos = _hrobject_pppoe_fields_prototype;
    return 0;
}

static int hrobject_pppoe_dtor(struct j2sobject *obj) {
    if (!obj) return 0;

    return 0;
}

static int hrobject_wan_ctor(struct j2sobject *obj) {
    if (!obj)
        return -1;
    obj->name = "wan";
    obj->field_protos = _hrobject_wan_fields_prototype;
    return 0;
}

static int hrobject_wan_dtor(struct j2sobject *obj) {
    if (!obj) return 0;

    return 0;
}


#if 0
// clang-format off
// must special offset_len when you using char array
static struct j2sobject_fields_prototype hrobject_intf_statistic_prototype[] = {
    {.name = "total_sent_bytes",              .type = J2S_INT,    .offset = _J2SOBJECT_INTF_STAT_DATA_OFFSET(total_sent_bytes),              .offset_len = 0},
    {.name = "total_received_bytes",          .type = J2S_INT,    .offset = _J2SOBJECT_INTF_STAT_DATA_OFFSET(total_received_bytes),          .offset_len = 0},
    {.name = "total_sent_packets",           .type = J2S_INT,    .offset = _J2SOBJECT_INTF_STAT_DATA_OFFSET(total_sent_packets),           .offset_len = 0},
    {.name = "total_received_packets",       .type = J2S_INT,    .offset = _J2SOBJECT_INTF_STAT_DATA_OFFSET(total_received_packets),       .offset_len = 0},
    {.name = "rx_kbps",                       .type = J2S_INT,    .offset = _J2SOBJECT_INTF_STAT_DATA_OFFSET(rx_kbps),                       .offset_len = 0},
    {.name = "tx_kbps",                       .type = J2S_INT,    .offset = _J2SOBJECT_INTF_STAT_DATA_OFFSET(tx_kbps),                       .offset_len = 0},
    {0}
};
// clang-format on

hrobject_intf_statistic *hrobject_intf_statistic_alloc() {
    struct j2sobject *obj = j2sobject_create(J2S_OBJECT, sizeof(hrobject_intf_statistic));
    if (!obj)
        return NULL;
    obj->name = "ip";
    obj->field_protos = hrobject_intf_statistic_prototype;
    return (hrobject_intf_statistic *)obj;
}
void hrobject_intf_statistic_dealloc(hrobject_intf_statistic *obj) {
    j2sobject_free(J2SOBJECT(obj));
}
#endif

// clang-format off
;
// clang-format on
#if 0
// clang-format off
static struct j2sobject_fields_prototype hrobject_wan_state_prototype[] = {
    {.name = "v4_status",              .type = J2S_INT,    .offset = _J2SOBJECT_WAN_STATE_DATA_OFFSET(v4_status),              .offset_len = 0},
    {.name = "v6_status",              .type = J2S_INT,    .offset = _J2SOBJECT_WAN_STATE_DATA_OFFSET(v6_status),              .offset_len = 0},
    {.name = "online_time",            .type = J2S_INT,    .offset = _J2SOBJECT_WAN_STATE_DATA_OFFSET(online_time),            .offset_len = 0},
    {0}
};
// clang-format on

hrobject_wan_state *hrobject_wan_state_ctor() {
    struct j2sobject *obj = j2sobject_create(J2S_OBJECT, sizeof(hrobject_wan_state));
    if (!obj)
        return NULL;
    obj->name = "ip";
    obj->field_protos = hrobject_wan_state_prototype;
    return (hrobject_wan_state *)obj;
}
void hrobject_wan_state_dtor(hrobject_wan_state *obj) {
    j2sobject_free(J2SOBJECT(obj));
}

// clang-format off
static struct j2sobject_fields_prototype hrobject_lanhost_prototype[] = {
    {.name = "index",              .type = J2S_INT,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(index),              .offset_len = 0},
    {.name = "port",              .type = J2S_INT,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(port),              .offset_len = 0},
    {.name = "mac",            .type = J2S_STRING,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(mac),            .offset_len = _J2SOBJECT_LANHOST_DATA_LEN(mac)},
    {.name = "hostname",            .type = J2S_STRING,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(hostname),            .offset_len = _J2SOBJECT_LANHOST_DATA_LEN(hostname)},
    {.name = "online",              .type = J2S_INT,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(online),              .offset_len = 0},
    {.name = "online_time",            .type = J2S_INT,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(online_time),            .offset_len = 0},
    {.name = "rx_kbps",              .type = J2S_INT,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(rx_kbps),              .offset_len = 0},
    {.name = "tx_kbps",              .type = J2S_INT,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(tx_kbps),              .offset_len = 0},
    {.name = "rx_packets",              .type = J2S_INT,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(rx_packets),              .offset_len = 0},
    {.name = "tx_packets",              .type = J2S_INT,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(tx_packets),              .offset_len = 0},
    {.name = "rx_bytes",              .type = J2S_INT,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(rx_bytes),              .offset_len = 0},
    {.name = "tx_bytes",              .type = J2S_INT,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(tx_bytes),              .offset_len = 0},
    {.name = "crc_errors",              .type = J2S_INT,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(crc_errors),              .offset_len = 0},
    {.name = "max_rx_kbps",              .type = J2S_INT,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(max_rx_kbps),              .offset_len = 0},
    {.name = "max_tx_kbps",              .type = J2S_INT,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(max_tx_kbps),              .offset_len = 0},
    {.name = "v4_address",            .type = J2S_STRING,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(v4_address),            .offset_len = _J2SOBJECT_LANHOST_DATA_LEN(v4_address)},
    {.name = "v6_address",            .type = J2S_STRING,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(v6_address),            .offset_len = _J2SOBJECT_LANHOST_DATA_LEN(v6_address)},
    {.name = "connection_type",              .type = J2S_INT,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(connection_type),              .offset_len = 0},
    {.name = "ssid",            .type = J2S_STRING,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(ssid),            .offset_len = _J2SOBJECT_LANHOST_DATA_LEN(ssid)},
    {.name = "rssi",              .type = J2S_INT,    .offset = _J2SOBJECT_LANHOST_DATA_OFFSET(rssi),              .offset_len = 0},
    {0}
};
// clang-format on

hrobject_lanhost_t *hrobject_lanhost_alloc() {
    struct j2sobject *obj = j2sobject_create(J2S_OBJECT, sizeof(hrobject_lanhost_t));
    if (!obj)
        return NULL;
    obj->name = "lanhost";
    obj->field_protos = hrobject_lanhost_prototype;
    return (hrobject_lanhost_t *)obj;
}
void hrobject_lanhost_dealloc(hrobject_lanhost_t *obj) {
    j2sobject_free(J2SOBJECT(obj));
}
#endif