#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cjson/cJSON.h"
#include "j2sobject.h"

#define SUPPORT_NUM_ARRAY 1
// #define SUPPORT_STRING_ARRAY 1

// 获取元素 ele 在结构体中偏移量
#define _J2SOBJECT_DEMO_DATA_OFFSET(ele) \
    offsetof(struct j2sobject_demo, ele)

// 获取结构体中 ele 元素大小
#define _J2SOBJECT_DEMO_DATA_LEN(ele) \
    sizeof(((struct j2sobject_demo *)0)->ele)

#define _J2SOBJECT_DEMO_DATA_ARRAY_LEN(ele) \
    sizeof(((struct j2sobject_demo *)0)->ele) / sizeof(((struct j2sobject_demo *)0)->ele[0])

struct j2sobject_demo {
    J2SOBJECT_DECLARE_OBJECT;
    int intval;
    char prealloc_str[128];
    char *dynamic_str;
#if SUPPORT_NUM_ARRAY
    int intarray[4];
    double doublearray[10];
#endif
};

static int j2sobject_demo_ctor(struct j2sobject *obj);

static struct j2sobject_prototype _j2sobject_demo_prototype = {
    .name = "demo",
    .type = J2S_OBJECT,
    .size = sizeof(struct j2sobject_demo),
    .ctor = j2sobject_demo_ctor,
    .dtor = NULL};

static struct j2sobject_fields_prototype _j2sobject_demo_fields_prototype[] = {
    {.name = "intval", .type = J2S_INT, .offset = _J2SOBJECT_DEMO_DATA_OFFSET(intval), .offset_len = 0},
    {.name = "prealloc_str", .type = J2S_STRING, .offset = _J2SOBJECT_DEMO_DATA_OFFSET(prealloc_str), .offset_len = _J2SOBJECT_DEMO_DATA_LEN(prealloc_str) /*string buffer has been allocated*/},
    {.name = "dynamic_str", .type = J2S_STRING, .offset = _J2SOBJECT_DEMO_DATA_OFFSET(dynamic_str), .offset_len = 0 /*string buffer will dynamic allocated when needed*/},
#if SUPPORT_NUM_ARRAY
    {.name = "intarray", .type = J2S_INT, .offset = _J2SOBJECT_DEMO_DATA_OFFSET(intarray), .offset_len = _J2SOBJECT_DEMO_DATA_ARRAY_LEN(intarray)},
    {.name = "doublearray", .type = J2S_DOUBLE, .offset = _J2SOBJECT_DEMO_DATA_OFFSET(doublearray), .offset_len = _J2SOBJECT_DEMO_DATA_ARRAY_LEN(doublearray)},
#endif
    {0}};

static int j2sobject_demo_ctor(struct j2sobject *obj) {
    if (!obj)
        return -1;
    obj->name = "demo";
    obj->field_protos = _j2sobject_demo_fields_prototype;
    return 0;
}
int main(int argc, char **argv) {
    struct j2sobject_demo *object = (struct j2sobject_demo *)j2sobject_create(&_j2sobject_demo_prototype);

    // 将 json 文件  1.json 反序列化到对象
    j2sobject_deserialize_file(J2SOBJECT(object), "j2s-num.json");

    // 访问 struct j2sobject_demo* 中相关字段或者作出修改

    printf("int array:\n");
    for (int i = 0; i < sizeof(object->intarray) / sizeof(object->intarray[0]); i++) {
        printf("i:%d -> %d\n", i, object->intarray[i]);
    }
    printf("double array:\n");
    for (int i = 0; i < sizeof(object->doublearray) / sizeof(object->doublearray[0]); i++) {
        printf("i:%d -> %f\n", i, object->doublearray[i]);
    }
    // 将对象重新序列化为可打印字符串
    char *str = j2sobject_serialize(J2SOBJECT(object));
    printf("serialize:%s\n", str);
    // 注意，内存必须手动释放
    free(str);

    object->intval = 100;
    snprintf(object->prealloc_str, sizeof(object->prealloc_str), "%s", "this is prealloc string");
    if (object->dynamic_str) {
        free(object->dynamic_str);
        object->dynamic_str = NULL;
    }
    object->dynamic_str = strdup("this is dynamic str");

    object->intarray[0] = 1;
    object->intarray[1] = 2;
    object->intarray[2] = 3;
    object->intarray[3] = 4;

    object->doublearray[0] = 1.1;
    object->doublearray[1] = 0.2;
    object->doublearray[2] = 3.45;
    object->doublearray[3] = 40000.34;

    // 将对象重新序列化保存到文件
    j2sobject_serialize_file(J2SOBJECT(object), "j2s-num-modified.json");

    free(object->dynamic_str);
    object->dynamic_str = NULL;
    // 释放对象
    j2sobject_free(J2SOBJECT(object));

    return 0;
}
