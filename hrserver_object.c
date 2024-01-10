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
    {.name = "dynamic_str",              .type = J2S_STRING,    .offset = _J2SOBJECT_PPPOE_DATA_OFFSET(dynamic_str),              .offset_len = 0},
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
    {.name = "pppoe_ptr",            .type = J2S_OBJECT,    .offset = _J2SOBJECT_WAN_DATA_OFFSET(pppoe_ptr),          .offset_len = 0, .proto = &_hrobject_pppoe_prototype},
    {0}
};
// clang-format on

int hrobject_ipv4_ctor(struct j2sobject *obj) {
    if (!obj)
        return -1;
    obj->name = "ipv4";
    obj->field_protos = _hrobject_ipv4_fields_prototype;
    printf("%s(%d): ctor:%p\n", __FUNCTION__, __LINE__, obj);
    return 0;
}

static int hrobject_ipv4_dtor(struct j2sobject *obj) {
    if (!obj) return 0;
    printf("%s(%d): ctor:%p\n", __FUNCTION__, __LINE__, obj);
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
    printf("%s(%d): ctor:%p, dynamic_str:%p\n", __FUNCTION__, __LINE__, obj, ((hrobject_pppoe_t*)obj)->dynamic_str);

    return 0;
}

static int hrobject_pppoe_dtor(struct j2sobject *obj) {
    if (!obj) return 0;
    printf("%s(%d): dtor:%p, dynamic_str:%p\n", __FUNCTION__, __LINE__, obj, ((hrobject_pppoe_t*)obj)->dynamic_str);
    if (((hrobject_pppoe_t*)obj)->dynamic_str) {
        free(((hrobject_pppoe_t*)obj)->dynamic_str);
        ((hrobject_pppoe_t*)obj)->dynamic_str = NULL;
    }
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


