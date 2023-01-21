#ifndef X264_TEST_SCREEN_H
#define X264_TEST_SCREEN_H

#include <stdint.h>

void screen_init_capture(uint16_t *width, uint16_t *height);

void screen_capture(uint8_t *image, uint8_t *bytes_per_pixel);

#endif //X264_TEST_SCREEN_H
