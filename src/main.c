#include <stdlib.h>
#include <stdio.h>

#ifndef FFMPEG_PATH
#define FFMPEG_PATH "ffmpeg"  // fallback if not defined
#endif

int main(void) {
    // Paths to input JPEG and output AVIF
    const char *input_file = "img/image.jpg";
    const char *output_file = "img/image.avif";

    // Build the ffmpeg command; the -y flag forces overwrite if needed.
    // This command uses the libaom-av1 encoder (enabled during FFmpeg configuration).
    char command[1024];
    int ret = snprintf(command, sizeof(command),
                       "\"%s\" -y -i \"%s\" -c:v libaom-av1 \"%s\"",
                       FFMPEG_PATH, input_file, output_file);
    if (ret < 0 || ret >= sizeof(command)) {
        fprintf(stderr, "Error: command buffer overflow.\n");
        return 1;
    }

    printf("Executing command:\n%s\n", command);
    ret = system(command);
    if (ret != 0) {
        fprintf(stderr, "FFmpeg conversion failed with code %d\n", ret);
        return ret;
    }

    printf("Conversion successful. %s was converted to %s\n", input_file, output_file);
    return 0;
}
