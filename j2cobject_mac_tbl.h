/*file is auto created , do not modify it!!!*/
/*
* Copyright (C) 2023 Inspur Group Co., Ltd. Unpublished
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

#ifndef _J2COBJECT_MAC_TBL_H_
#define _J2COBJECT_MAC_TBL_H_

#include "stddef.h"
#include "stdlib.h"
#include "j2cobject.h"


struct j2cobject_mac_tbl {
  J2COBJECT_DECLARE_OBJECT;
  char* mac;
  char* name;
};

static struct j2cobject_prototype j2cobject_prototype_mac_tbl[] = {
  {
    .name= "mac",
    .type= J2C_STRING,
    .offset= offsetof(struct j2cobject_mac_tbl, mac),
    .offset_len= 0
  },
  {
    .name= "name",
    .type= J2C_STRING,
    .offset= offsetof(struct j2cobject_mac_tbl, name),
    .offset_len= 0
  },
  {0}
};

static inline struct j2cobject_mac_tbl* j2cobject_mac_tbl_allocate() {
  struct j2cobject *obj = j2cobject_create(J2C_OBJECT, sizeof(struct j2cobject_mac_tbl));
  if (!obj) return NULL;
  obj->name = "mac_tbl";
  obj->prototype = j2cobject_prototype_mac_tbl;
  return (struct j2cobject_mac_tbl*)obj;
}
static inline void j2cobject_mac_tbl_deallocate(struct j2cobject_mac_tbl* obj) {
  if (!obj) return;
   j2cobject_free(J2COBJECT(obj));
}
#endif //_J2COBJECT_MAC_TBL_H_
