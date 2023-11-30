#include <stddef.h>
#include <stdio.h>
#include <string.h>


#include "cjson/cJSON.h"


#include "j2cobject.h"



#define TEST

#ifdef TEST
    #include "j2cobject_serverinfo.h"

    #include "j2cobject_mac_tbl.h"


// valgrind --leak-check=full ./j2c


int main(int argc, char **argv) {

    struct j2cobject_serverinfo *server = j2cobject_serverinfo_allocate();

    j2cobject_from_file("1.json", J2COBJECT(server));

    printf("int val:%d\n", server->int_val);
    printf("double val:%f\n", server->double_val);
    printf("addr:%s\n", server->bssaddr);
    printf("abiaddr:%s\n", server->abiaddr);
    printf("devid:%s\n", server->devid);
    printf("backup:%p\n", server->backup);

#if 0
    j2cobject_serverinfo_deallocate(server);

    printf("***************************\n");
    const char* str = "{\"bssaddr\":\"http://12306.com\",\"abiaddr\":\"10.30.11.113:port\",\"devtoken\":\"devtokexxxxxxxxxxxyyyyyyyyyyyyyyyyyiiiiiiiiiiiiiiiiiiiiiiixzxxxxxxxxxxx\",\"devid\":\"devid12345678\",\"backup\":[\"backup1\", \"backup2\"],\"int_val\": 24, \"double_val\": 256.123}";

    server = j2cobject_serverinfo_allocate();
    j2cobject_from_string(str, J2COBJECT(server));
    printf("addr:%s\n", server->bssaddr);
    printf("abiaddr:%s\n", server->abiaddr);
    printf("devid:%s\n", server->devid);
    printf("backup:%p\n", server->backup);
    printf("macs:%p\n", server->macs);

    printf("serialize .................\n");

    char* json = j2cobject_serializer(J2COBJECT(server));

    printf("json:%s\n", json ? json : "error");

    free(json);
#endif
    struct j2cobject *backup = server->macs;
    printf("backup ....:%p, type:%d\n", backup, backup->type);


    // only support object not array ...
    j2cobject_write_file(J2COBJECT(server), "autogen.json");


    if (backup->type == J2C_ARRAY) {

        printf("backup ..array..:%p\n", backup);
        struct j2cobject *childs = j2cobject_to_list(backup, (j2cobject_allocate_handler)j2cobject_mac_tbl_allocate);

        if (!childs) {
            printf("get childs failed ...\n");

        } else {
            struct j2cobject *loop = childs;

            printf("backup ....loop:%p\n", loop);
            for (loop = childs; loop != NULL; loop = loop->next) {
                printf("backup ....loop:%p\n", loop);
                struct j2cobject_mac_tbl *mac = (struct j2cobject_mac_tbl *)loop;
                printf("mac:%s, name:%s\n", mac->mac, mac->name);

                //j2cobject_free(mac);
            }

            j2cobject_list_for_each(loop, childs) {
                struct j2cobject_mac_tbl *mac = (struct j2cobject_mac_tbl *)loop;
                printf("2mac:%s, name:%s\n", mac->mac, mac->name);
            }

            j2cobject_list_destroy(childs);
        }
    }

    printf("deallocate ...........\n");
    j2cobject_serverinfo_deallocate(server);


    return 0;
}

#endif
