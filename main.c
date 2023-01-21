#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <x264.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include "socket.h"
#include "screen.h"


/********************************************************************************
 * Basics of RGB -> YUV conversion
 *******************************************************************************/
/*
 * RGB -> YUV conversion macros
 */
#define RGB2Y(r, g, b) (uint8_t)(((66 * (r) + 129 * (g) +  25 * (b) + 128) >> 8) +  16)
#define RGB2U(r, g, b) (uint8_t)(((-38 * (r) - 74 * (g) + 112 * (b) + 128) >> 8) + 128)
#define RGB2V(r, g, b) (uint8_t)(((112 * (r) - 94 * (g) -  18 * (b) + 128) >> 8) + 128)

/* Converts R8 G8 B8 color to YUV. */
static __inline__ void
R8G8B8ToYUV(uint8_t r, uint8_t g, uint8_t b, uint8_t *y, uint8_t *u, uint8_t *v) {
    *y = RGB2Y((int) r, (int) g, (int) b);
    *u = RGB2U((int) r, (int) g, (int) b);
    *v = RGB2V((int) r, (int) g, (int) b);
}


int main() {
    uint16_t screen_width;
    uint16_t screen_height;
    screen_init_capture(&screen_width, &screen_height);
    uint8_t *image = malloc(screen_width * screen_height * 4);
    uint8_t bytes_per_pixel;
    screen_capture(image, &bytes_per_pixel);

    for (int i = 0; i < screen_width * screen_height * 4; i += 4) {
        R8G8B8ToYUV(image[i], image[i + 1], image[i + 2], &image[i], &image[i + 1], &image[i + 2]);
    }



    x264_param_t x264Param;
    x264_param_default(&x264Param);
    x264_param_default_preset(&x264Param, "veryfast", "zerolatency");
    x264Param.i_threads = 1;
    x264Param.i_width = screen_width;
    x264Param.i_height = screen_height;
    x264Param.i_slice_max_size = 1390;
    x264_param_apply_profile(&x264Param, "baseline");

    x264_t *encoder = x264_encoder_open(&x264Param);

    x264_picture_t pic_in, pic_out;
//    pic_in.img.i_csp = X264_CSP_BGR;
//    pic_in.img.i_plane = 1;
//    pic_in.img.i_stride[0] = 3 * WIDTH;
//    pic_in.img.plane[0] = bgr;
//    pic_in.i_pts = frameCount;
//    pic_in.i_type = X264_TYPE_AUTO;
    x264_picture_alloc(&pic_in, X264_CSP_I420, screen_width, screen_height);
    pic_in.img.plane[0] = image;
    x264_picture_alloc(&pic_out, X264_CSP_I420, screen_width, screen_height);

    /* Load 24-bit BGR bitmap */
//    pic_out.i_pts = frameCount;

    x264_nal_t *nals;
    int i_nals;
    /* Returns a frame size of 912 for first frame in this case */
    int frame_size = x264_encoder_encode(encoder, &nals, &i_nals, &pic_in, &pic_out);

    socket_global_init();
    //Socket receiver
    Socket sock2 = socket_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in address_info2;
    memset(&address_info2, 0, sizeof(address_info2));
    address_info2.sin_family = AF_INET;
    address_info2.sin_addr.S_un.S_addr = INADDR_ANY;
    address_info2.sin_port = htons(3344);
    int bind_result = socket_bind(sock2, (const struct sockaddr *) &address_info2, sizeof(address_info2));

    //Socket sender
    Socket sock = socket_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct addrinfo *address_info;
    socket_get_address_info("127.0.0.1", "3344", 0, &address_info);
    int connect_result = socket_connect(sock, address_info->ai_addr, (int) address_info->ai_addrlen);
    socket_free_address_info(address_info);

    printf("Decoder returned frame size = %d \n", frame_size);
    printf("Decoder returned %d NAL units \n", i_nals);
    int accum_frame_size = 0;
    if (frame_size >= 0) {
        for (int i = 0; i < i_nals; i++) {
            printf("******************* NAL %d (%d bytes) *******************\n", i, nals[i].i_payload);
            for (int j = 0; j < nals[i].i_payload; j++) {
                if (j == 0) printf("First 10 bytes: ");
                if (j < 10) printf("%02X |", nals[i].p_payload[j]);
                accum_frame_size++;
            }
            printf("\n");
            socket_send(sock, nals[i].p_payload, nals[i].i_payload, 0);
        }
    }

    printf("Verified frame size = %d \n", accum_frame_size);

//    uint8_t *encoded_frame = malloc(accum_frame_size);
//    ISVCDecoder *decoder;
//    WelsCreateDecoder(&decoder);
//    SDecodingParam sDecParam = {0};
//    sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
//    sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;

//    uint8_t buff[2048];
//    do {
//        memset(buff, 0, 2048);
//    } while (socket_receive(sock2, buff, 2048, 0) != 0);

    socket_shutdown(sock, SHUT_RDWR);
    socket_shutdown(sock2, SHUT_RDWR);
    socket_close(sock);
    socket_close(sock2);
    socket_global_destroy();

    return 0;
}
