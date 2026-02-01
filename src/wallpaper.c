#include <stdio.h>
#include <windows.h>

char  path[MAX_PATH];
char* path_ptr = path;
int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage:\n");
        printf("    wallpaper: [opt] <path>\n");
        printf("Options:\n");
        printf("    -f|--full:  Full path instead of using the default wallpaper directory\n");
        return 0;
    }
    int  i = 0;
    int  j = 0;
    int  k = 0;
    while ((path[i] = argv[1][i])) i++;
    j = i;
    for (i = 2; i < argc - 1; ++i) {
        if ((!strcmp(argv[i], "-f") || !strcmp(argv[i], "--full")) && i+1 < argc) {
            i++;
            path_ptr = argv[i];
        }
    }
    if (i < argc) {
        while ((path[j] = argv[i][k])) {++k; ++j;}
    }
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 1, path_ptr, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

