#include <io.h>
#include <Windows.h>
#include <json.h>
#include <stdio.h>


typedef struct json_value_s*          json_val;
typedef struct json_object_s*         json_obj;
typedef struct json_object_element_s* json_elem;
typedef struct json_array_element_s*  json_arr_elem;
typedef struct json_string_s*         json_str;
typedef struct json_array_s*          json_arr;

static char def_prof_hash[64]; 
static char prof_hash    [64];

const char* find_id(json_arr_elem e) {
    json_obj  obj  = e->value->payload;
    json_elem iter = obj->start;
    while (iter) {
        if (!strcmp(iter->name->string, "guid"))
            return ((json_str)iter->value->payload)->string; 
        iter = iter->next;
    }
    return 0;
}

int find_default_profile(const char* cont, size_t s, json_arr_elem* outp) {
    json_val  root = json_parse(cont, s);
    json_obj  origin = root->payload;
    json_obj  obj  = root->payload;
    json_elem iter = obj->start;
    int i = 0;
    while (iter) {
        const char* str  = iter->name->string;
        if (!strcmp(str, "defaultProfile")) {
            json_str jstr = ((json_str)iter->value->payload);
            memcpy(def_prof_hash, jstr->string, jstr->string_size);
            i++;
        }
        if (!strcmp(str, "profiles")) {
            i++;
            break;            
        }
        iter = iter->next;
    }
    if (i != 2) {
        printf("Default profile not found or corrupted json.");
        return 1;
    }
    obj  = iter->value->payload;
    iter = obj->start;
    i    = 0;
    json_arr_elem iter_arr;
    while (iter) {
        const char* str = iter->name->string;
        if (strcmp(str, "list")) {
            iter = iter->next;
            continue;
        }
        json_arr obj2  = iter->value->payload;
        iter_arr       = obj2->start;
        ++i;
        break;
    }
    if (!i) {
        printf("List field not found");
        return 2;
    }
    *outp = 0;
    while (iter_arr) {
        const char* id = find_id(iter_arr);
        if (!strcmp(id, def_prof_hash)) {
            *outp = iter_arr;
            break;
        }
        iter_arr = iter_arr->next;
    }
    if (!*outp) {
        printf("Default Porfile not found\n");
        return 3;
    } 
    return 0;
}

int make_changes(json_arr_elem profile) {
    
    return 0;
//    static char outs[1<<14];
//    json_write_pretty_object(origin, 2, "   ", "\n", outs);
//    printf("%s\n", outs);    
}

int process() {
    int    i;
    int    s;
    int    state;
    FILE*  f;
    HANDLE hdl;
    json_arr_elem outp;
    def_prof_hash[0] = 0;
    prof_hash[0]     = 0;
    char*       cont = 0;
    const char* path = "settings.json";
    fopen_s(&f, path, "r+");
    if (!f) return 1;
    i    = 0;
    hdl  = (HANDLE)_get_osfhandle(_fileno(f));
    s    = GetFileSize(hdl, 0);
    cont = (char*)malloc(s);
    
    fread(cont, s, 1, f);
    i = find_default_profile(cont, s, &outp);
    make_changes(outp);
    free(cont);
    return i;
}


int main() {
    process();
    return 0;
}
