#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "cjson/cJSON.h"

#include "j2sobject.h"

#define TEST

#ifdef TEST

#include "hrserver_object.h"

// valgrind --leak-check=full ./j2s

int _auto_test() {

  hrobject_wan_t *object = NULL;

  object = HROBJECT_WAN_NEW();

  j2sobject_deserialize_file(J2SOBJECT(object), "1.json");

  printf("index:%d\n", object->index);
  printf("name:%s\n", object->name);
  printf("workmode:%d\n", object->workmode);
  printf("v4 proto:%d\n", object->v4_proto);
  printf("pppoe username:%s, password:%s, dynamic_str:%s\n", object->pppoe.username,
         object->pppoe.password, object->pppoe.dynamic_str ? object->pppoe.dynamic_str : "null");

  printf("pppoe_ptr username:%s, password:%s, dynamic_str:%s\n", object->pppoe_ptr->username,
         object->pppoe_ptr->password, object->pppoe_ptr->dynamic_str ? object->pppoe_ptr->dynamic_str : "null");


  printf("device: name:%s, mac:%s, v4 address:%s, mask:%s, gw:%s, dns:%s %s\n",
         object->device.name, object->device.mac,
         object->device.v4.data.address, object->device.v4.data.netmask,
         object->device.v4.data.gateway, object->device.v4.data.dns,
         object->device.v4.data.dns2);

  snprintf(object->device.name, sizeof(object->device.name), "%s", "wan_name");

  j2sobject_serialize_file(J2SOBJECT(object), "1-modified.json");

  char *str = j2sobject_serialize(J2SOBJECT(object));

  printf("serialize:%s\n", str);

  // free(str);

  HROBJECT_WAN_DEL(object);

  object = HROBJECT_WAN_NEW();

  j2sobject_deserialize(J2SOBJECT(object), str);

  free(str);

  printf("index:%d\n", object->index);
  printf("name:%s\n", object->name);
  printf("workmode:%d\n", object->workmode);
  printf("v4 proto:%d\n", object->v4_proto);
  printf("pppoe username:%s, password:%s, dynamic_str:%s\n", object->pppoe.username,
         object->pppoe.password, object->pppoe.dynamic_str ? object->pppoe.dynamic_str : "null");

  printf("pppoe_ptr username:%s, password:%s, dynamic_str:%s\n", object->pppoe_ptr->username,
         object->pppoe_ptr->password, object->pppoe_ptr->dynamic_str ? object->pppoe_ptr->dynamic_str : "null");

  printf("device: name:%s, mac:%s, v4 address:%s, mask:%s, gw:%s, dns:%s %s\n",
         object->device.name, object->device.mac,
         object->device.v4.data.address, object->device.v4.data.netmask,
         object->device.v4.data.gateway, object->device.v4.data.dns,
         object->device.v4.data.dns2);
  HROBJECT_WAN_DEL(object);

  return 0;
}

int main(int argc, char **argv) {
  int i = 0;
  while (i++ < 1) {
    _auto_test();
  }

  return 0;
}
#endif
