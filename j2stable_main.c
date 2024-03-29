#include <stddef.h>
#include <stdio.h>

#include "j2sobject.h"
#include "j2stable.h"

typedef struct {
    J2STBL_DECLARE_OBJECT;
    int id;
    char mac[18];
    int type;
} j2stbl_mactbl_t;

#define _MACTBL_DATA_OFFSET(ele) offsetof(j2stbl_mactbl_t, ele)
#define _MACTBL_DATA_LEN(ele) sizeof(((j2stbl_mactbl_t*)0)->ele)

static int j2stbl_mactbl_ctor(struct j2sobject* obj);

struct j2sobject_prototype j2stbl_mac_tbl_prototype = {.name = "mac_tbl",
                                                       .type = J2S_OBJECT,
                                                       .size = sizeof(
                                                           j2stbl_mactbl_t),
                                                       .ctor = j2stbl_mactbl_ctor,
                                                       .dtor = NULL

};

static struct j2sobject_fields_prototype _j2stbl_mactbl_fields_prototype[] = {
    J2STBL_OBJECT_PRIV_FIELDS,
    {.name = "id", .type = J2S_INT, .offset = _MACTBL_DATA_OFFSET(id), .offset_len = 0},
    {.name = "mac", .type = J2S_STRING, .offset = _MACTBL_DATA_OFFSET(mac), .offset_len = _MACTBL_DATA_LEN(mac)},
    {.name = "type", .type = J2S_INT, .offset = _MACTBL_DATA_OFFSET(type), .offset_len = 0},
    {0}};

static int j2stbl_mactbl_ctor(struct j2sobject* obj) {
    if (!obj)
        return -1;
    obj->name = "mactbl";
    obj->field_protos = _j2stbl_mactbl_fields_prototype;
    return 0;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    j2stbl_mactbl_t* del = NULL;

    struct j2stbl_object* data = NULL;
    struct j2stable* tbl = j2stable_init("mac_tbl", &j2stbl_mac_tbl_prototype);
    if (!tbl) {
        printf("can not init mac tbl\n");
        return -1;
    }
    j2stbl_mactbl_t* t = NULL;

    data = j2stable_query_all(tbl);

    if (data != NULL) {
        for (t = (j2stbl_mactbl_t*)J2SOBJECT(data)->next; t != (j2stbl_mactbl_t*)J2SOBJECT(data); t = (j2stbl_mactbl_t*)J2SOBJECT(t)->next) {
            printf("mac tbl:%p\n", t);
            printf("mac tbl __id__:%d\n", J2STBL_OBJECT_SELF(t)->__id__);
            printf("mac tbl id:%d\n", t->id);
            printf("mac tbl mac:%s\n", t->mac);
            printf("mac tbl type:%d\n", t->type);
            if (t->id == 3) {
                del = t;
            }
        }
    }

    j2stbl_mactbl_t* item = (j2stbl_mactbl_t*)j2sobject_create(&j2stbl_mac_tbl_prototype);
    item->id = 10;
    item->type = 1;
    snprintf(item->mac, sizeof(item->mac), "%s", "6c:0b:84:3c:71:9e");
    j2stable_insert(tbl, J2STBL_OBJECT_SELF(item));
    // insert again , trigger error ...
    // j2stable_insert(tbl, J2STBL_OBJECT_SELF(item));

    if (del) {
        printf("press any key delete:%p, which __id__:%d\n", del, J2STBL_OBJECT_SELF(del)->__id__);
        getchar();
        j2stable_delete(tbl, J2STBL_OBJECT_SELF(del));
    }
    j2stable_deinit(tbl);
    return 0;
}
