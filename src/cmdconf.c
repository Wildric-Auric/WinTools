#include <io.h>
#include <Windows.h>
#include <json.h>
#include <stdio.h>
#include <stdlib.h>


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

int find_default_profile(const char* cont, size_t s, json_arr_elem* outp, json_val* root) {
    *root = json_parse(cont, s);
    if (!json_value_as_object(*root)) {
        printf("Failed to parse json\n");
        return 1;
    }
    json_obj  origin = (*root)->payload;
    json_obj  obj  = (*root)->payload;
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

typedef struct  {
    const char* newFont;
} json_changes;


int change_font(json_obj fobj, const char* newf) {
    json_elem iter = fobj->start;
    while (iter) {
        if (strcmp(iter->name->string, "face")) {
            iter = iter->next;
            continue;
        } 
        json_str str = (iter->value->payload);
        str->string      = newf;
        str->string_size = strlen(newf);
        return 0;
    }
    return 1;
}

int make_changes(json_arr_elem profile, const json_changes * const diff) {
    json_obj obj = (profile->value->payload);
    json_elem iter = obj->start; 
    while (iter) {
        if (diff->newFont && !strcmp(iter->name->string, "font")) {
            change_font(iter->value->payload, diff->newFont);
        }
        iter = iter->next;
    }
    return 0;
}


void debug_print(json_val root) {
    char* outs = malloc(1<<14);
    json_write_pretty_object((json_obj)root->payload, 2, "   ", "\n", outs);
    printf("%s\n", outs);    
    free(outs);
}

void get_path(char* path) {
    static char  appd[256];
    const char* ext = "\\Packages\\Microsoft.WindowsTerminal_8wekyb3d8bbwe\\LocalState\\settings.json";
    int    i;
    char   ch;
    size_t len = 256;
    i = 0;
    getenv_s(&len, appd, sizeof(appd), "localappdata");
    while ((ch = appd[i])) {
        path[i] = ch;
        i++;
    }
    int j = 0; ch = 0;
    while ((ch = ext[j])) {
        path[i] = ch;
        i++; j++;
    }
    path[i] = 0;
    i = 0; ch = 0;
}

void save_json(const char* path, json_val root, char* cont) {
    FILE* f;
    fopen_s(&f, path,"w");
    char* off = json_write_pretty_object((json_obj)root->payload, 2, "  ", "\n", cont);
    fwrite(cont, off - cont, 1, f);
    fclose(f);
}

int process(json_changes* c) {
    int    i;
    int    s;
    int    state;
    FILE*  f;
    HANDLE hdl;
    json_arr_elem outp;
    def_prof_hash[0] = 0;
    prof_hash[0]     = 0;
    char*        cont = 0;
    static char  path[512];
    get_path(path);
    i = 0;
    fopen_s(&f, path, "rb");
    if (!f) return 1;
    i       = 0;
    hdl     = (HANDLE)_get_osfhandle(_fileno(f));
    s       = GetFileSize(hdl, 0);
    cont    = (char*)malloc(s + 512);
    cont[s] = 0;
    fread(cont, s, 1, f);
    fclose(f);
    json_val root;
    i = find_default_profile(cont, s, &outp, &root); 
    if (i) {
        goto end_of_func;
    }
    make_changes(outp, c);
    //debug_print(root);
    save_json(path, root, cont);
end_of_func:
    free(root);
    free(cont);
    return i;
}

void print_help() {
    printf("%s\n", "Usage: cmdconf [opt][value]");
    printf("%s\n", "commands:");
    printf("%s\n", "  -f|--font [font_name]: change the font");
    printf("%s\n", "  -h|--help            : print this help");
}

#define check_command(str, shrt, lg) (!strcmp(argv[i], shrt) || !strcmp(argv[i], lg))
int main(int argc, char** argv) {
    if (argc < 2) {
        print_help();
    }
    int i = 1;
    json_changes ch;
    ch.newFont = 0;
    while (i < argc) {
        if (check_command(argv[i], "-f", "--font") && i != (argc - 1)) {
            i++;
            ch.newFont = argv[i];
        }
        else if (check_command(argv[i], "-h", "--help")) {
            print_help();
        }
        i++;
    }
    if (ch.newFont)
        process(&ch);
    return 0;
}
