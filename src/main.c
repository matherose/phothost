#include <stdio.h>
#include <libavutil/avutil.h>

int main(void) {
    printf("Hello, World!\n");
    printf("FFmpeg version: %s\n", av_version_info());
    return 0;
}
