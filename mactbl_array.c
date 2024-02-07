#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "j2sobject.h"

typedef struct {
    J2SOBJECT_DECLARE_OBJECT;
    int id;
    char mac[18];
    int type;
} hrtbl_mactbl_t;

#define _MACTBL_DATA_OFFSET(ele) offsetof(hrtbl_mactbl_t, ele)
#define _MACTBL_DATA_LEN(ele) sizeof(((hrtbl_mactbl_t *)0)->ele)

static int hrtbl_mactbl_ctor(struct j2sobject *obj);

struct j2sobject_prototype hrtbl_mac_tbl_prototype = {.name = "mac_tbl",
                                                      .type = J2S_OBJECT,
                                                      .size = sizeof(
                                                          hrtbl_mactbl_t),
                                                      .ctor = hrtbl_mactbl_ctor,
                                                      .dtor = NULL

};

static struct j2sobject_fields_prototype _hrtbl_mactbl_fields_prototype[] = {
    {.name = "id", .type = J2S_INT, .offset = _MACTBL_DATA_OFFSET(id), .offset_len = 0},
    {.name = "mac", .type = J2S_STRING, .offset = _MACTBL_DATA_OFFSET(mac), .offset_len = _MACTBL_DATA_LEN(mac)},
    {.name = "type", .type = J2S_INT, .offset = _MACTBL_DATA_OFFSET(type), .offset_len = 0},
    {0}};

static int hrtbl_mactbl_ctor(struct j2sobject *obj) {
    if (!obj)
        return -1;
    printf("%s(%d): .....\n", __FUNCTION__, __LINE__);
    obj->name = "mactbl";
    obj->field_protos = _hrtbl_mactbl_fields_prototype;
    return 0;
}

int main(int argc, char **argv) {
    struct j2sobject *array = NULL;
    struct j2sobject *p = NULL, *n = NULL;

    printf("%s(%d): .....\n", __FUNCTION__, __LINE__);

    array = j2sobject_create_array(&hrtbl_mac_tbl_prototype);

    printf("%s(%d): .....\n", __FUNCTION__, __LINE__);
    j2sobject_deserialize_file(array, "mactbl_array.json");

    printf("%s(%d): .....\n", __FUNCTION__, __LINE__);

    hrtbl_mactbl_t *t = NULL;

    printf("%s(%d): .....\n", __FUNCTION__, __LINE__);
    for (t = (hrtbl_mactbl_t *)array->next; t != (hrtbl_mactbl_t *)array; t = (hrtbl_mactbl_t *)J2SOBJECT(t)->next) {
        printf("mac tbl:%p\n", t);
        printf("mac tbl id:%d\n", t->id);
        printf("mac tbl mac:%s\n", t->mac);
        printf("mac tbl type:%d\n", t->type);
    }

    printf("%s(%d): .....\n", __FUNCTION__, __LINE__);

#if 0
    // loop all elements free all element
    for (p = array->next, n = p->next; p != array; p = n, n = p->next) {
        printf("%s(%d): .....p:%p, head:%p, next:%p\n", __FUNCTION__, __LINE__, p, array, p->next);
        p->prev->next = p->next;
        p->next->prev = p->prev;
        j2sobject_free(p);
    }
#else

    j2sobject_free(array);
#endif

    //j2sobject_free(array);

    return 0;
}
