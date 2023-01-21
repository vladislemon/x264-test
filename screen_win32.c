#include "screen.h"
#include <windows.h>

static uint16_t screen_width;
static uint16_t screen_height;
static HWND screen_desktop_window;
static HDC screen_desktop_dc;
static HDC screen_capture_dc;
static HBITMAP screen_capture_bitmap;

void screen_init_capture(uint16_t *width, uint16_t *height) {
    screen_width = GetSystemMetrics(SM_CXSCREEN);
    screen_height = GetSystemMetrics(SM_CYSCREEN);
    screen_desktop_window = GetDesktopWindow();
    screen_desktop_dc = GetDC(screen_desktop_window);
    screen_capture_dc = CreateCompatibleDC(screen_desktop_dc);
    screen_capture_bitmap = CreateCompatibleBitmap(screen_desktop_dc, screen_width, screen_height);
    *width = screen_width;
    *height = screen_height;
}

void screen_capture(uint8_t *image, uint8_t *bytes_per_pixel) {
    int result;
    SelectObject(screen_capture_dc, screen_capture_bitmap);
    result = BitBlt(screen_capture_dc, 0, 0, screen_width, screen_height, screen_desktop_dc, 0, 0, SRCCOPY | CAPTUREBLT);
    BITMAP bitmap;
    result = GetObject(screen_capture_bitmap, sizeof(BITMAP), &bitmap);
    BITMAPINFOHEADER bitmap_info;
    bitmap_info.biSize = sizeof(BITMAPINFOHEADER);
    result = GetDIBits(screen_capture_dc, screen_capture_bitmap, 0, bitmap.bmHeight, NULL, (LPBITMAPINFO) &bitmap_info, DIB_RGB_COLORS);
//    bitmap_info.biWidth = bitmap.bmWidth;
//    bitmap_info.biHeight = -bitmap.bmHeight;
//    bitmap_info.biPlanes = 1;
//    bitmap_info.biBitCount = 32;
//    bitmap_info.biCompression = BI_RGB;
//    bitmap_info.biSizeImage = 0;
//    bitmap_info.biXPelsPerMeter = 0;
//    bitmap_info.biYPelsPerMeter = 0;
//    bitmap_info.biClrUsed = 0;
//    bitmap_info.biClrImportant = 0;
    result = GetDIBits(screen_capture_dc, screen_capture_bitmap, 0, bitmap.bmHeight, image, (LPBITMAPINFO) &bitmap_info, DIB_RGB_COLORS);
    //MessageBoxA(screen_desktop_window, "HELLO!", "TITLE", MB_HELP);
    *bytes_per_pixel = bitmap.bmWidthBytes / bitmap.bmWidth;
}
