#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <x264.h>
#include "socket.h"

#define WIDTH 1920
#define HEIGHT 1080

uint8_t bgr[WIDTH * HEIGHT * 3];

int main() {
    for (int i = 0; i < (WIDTH * 3 * HEIGHT); i++) {
        bgr[i] = (uint8_t) rand();
    }

    x264_param_t x264Param;
    x264_param_default(&x264Param);
    x264_param_default_preset(&x264Param, "veryfast", "zerolatency");
    x264Param.i_threads = 1;
    x264Param.i_width = WIDTH;
    x264Param.i_height = HEIGHT;
    x264Param.i_slice_max_size = 1390;
    x264_param_apply_profile(&x264Param, "high");

    x264_t *encoder = x264_encoder_open(&x264Param);

    x264_picture_t pic_in, pic_out;
//    pic_in.img.i_csp = X264_CSP_BGR;
//    pic_in.img.i_plane = 1;
//    pic_in.img.i_stride[0] = 3 * WIDTH;
//    pic_in.img.plane[0] = bgr;
//    pic_in.i_pts = frameCount;
//    pic_in.i_type = X264_TYPE_AUTO;
    x264_picture_alloc(&pic_in, X264_CSP_I420, WIDTH, HEIGHT);
    x264_picture_alloc(&pic_out, X264_CSP_I420, WIDTH, HEIGHT);

    /* Load 24-bit BGR bitmap */
//    pic_out.i_pts = frameCount;

    x264_nal_t *nals;
    int i_nals;
    /* Returns a frame size of 912 for first frame in this case */
    int frame_size = x264_encoder_encode(encoder, &nals, &i_nals, &pic_in, &pic_out);

    //Socket sender
    socket_global_init();
    Socket sock = socket_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct addrinfo *address_info;
    socket_get_address_info("127.0.0.1", "3344", 0, &address_info);
    socket_connect(sock, address_info->ai_addr, (int) address_info->ai_addrlen);
    socket_free_address_info(address_info);

    //Socket receiver
    Socket sock2 = socket_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct addrinfo *address_info2;
    socket_get_address_info("0.0.0.0", "3344", 0, &address_info2);
    socket_bind(sock2, address_info2->ai_addr, (int) address_info2->ai_addrlen);
    socket_free_address_info(address_info2);

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

    void *buff[2048];
    socket_receive(sock2, buff, 2048, 0);

    socket_shutdown(sock, SHUT_RDWR);
    socket_shutdown(sock2, SHUT_RDWR);
    socket_close(sock);
    socket_close(sock2);
    socket_global_destroy();

    return 0;
}
