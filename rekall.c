#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>

#include "rekall.h"
#include "log.h"

int rekall_lookup(const char *rekall_profile,
                    const char *symbol,
                    const char *subsymbol,
                    addr_t *rva,
                    addr_t *size)
{
    int ret = 1;
    if (!rekall_profile || !symbol) {
        goto done;
    }
    json_object *root = json_object_from_file(rekall_profile);
    if (!root) {
        writelog(LV_ERROR, "Rekall profile '%s' couldn't be opened", rekall_profile);
        goto done;
    }
    if (!subsymbol && !size) {
        json_object *constants = NULL, *jsymbol = NULL;
        if (!json_object_object_get_ex(root, "$CONSTANTS", &constants)) {
             writelog(LV_DEBUG, "Rekall profile: no $CONSTANTS section found");
             goto done;
        }
        if (!json_object_object_get_ex(constants, symbol, &jsymbol)) {
            writelog(LV_DEBUG, "Rekall profile: symbol '%s' not found", symbol);
            goto done;
        }
        *rva = json_object_get_int64(jsymbol);
        ret = 0;
    }
    else {
        json_object *structs = NULL, *jstruct = NULL, *jstruct2 = NULL, *jmember = NULL, *jvalue = NULL;
        if (!json_object_object_get_ex(root, "$STRUCTS", &structs)) {
            writelog(LV_DEBUG, "Rekall profile: no $STRUCTS section found");
            goto done;
        }
        if (!json_object_object_get_ex(structs, symbol, &jstruct)) {
            writelog(LV_DEBUG, "Rekall profile: no '%s' found", symbol);
            goto done;
        }
        if (size) {
            json_object *jsize = json_object_array_get_idx(jstruct, 0);
            *size = json_object_get_int64(jsize);
            goto done;
        }

        jstruct2 = json_object_array_get_idx(jstruct, 1);
        if (!jstruct2) {
            writelog(LV_DEBUG, "Rekall profile: struct '%s' has no second element", symbol);
            goto done;
        }
        if (!json_object_object_get_ex(jstruct2, subsymbol, &jmember)) {
            writelog(LV_DEBUG, "Rekall profile: '%s' has no '%s' member", symbol, subsymbol);
            goto done;
        }
        jvalue = json_object_array_get_idx(jmember, 0);
        if (!jvalue) {
            writelog(LV_DEBUG, "Rekall profile: '%s'.'%s' has no RVA defined", symbol, subsymbol);
            goto done;
        }
        *rva = json_object_get_int64(jvalue);
        ret = 0;
    }
done:
    if (root) {
        json_object_put(root);
    }
    return ret;
}
