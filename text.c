#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>

#include "font.xbm"

void pixel(struct fb_var_screeninfo *vinfo,
           struct fb_fix_screeninfo *finfo,
           char *fbp,
           int x,
           int y,
           unsigned char val)
{
    long int location;

    if(x >= vinfo->xres || y >= vinfo->yres) return;

    location = (x + vinfo->xoffset) * (vinfo->bits_per_pixel/8) +
               (y + vinfo->yoffset) * finfo->line_length;

    fbp[location] = 0x0;
    fbp[location + 1] = val;
}

void clearscreen(struct fb_var_screeninfo *vinfo,
                 struct fb_fix_screeninfo *finfo,
                 char *fbp)
{
    int x, y;

    for(y = 0; y < vinfo->yres; y++) {
        for(x = 0; x < vinfo->xres; x++) {
            pixel(vinfo, finfo, fbp, x, y, 0);
        }
    }
}

void draw_glyph(struct fb_var_screeninfo *vinfo,
                struct fb_fix_screeninfo *finfo,
                char *fbp,
                int offx, int offy,
                int gx, int gy)
{
    int x, y;
    unsigned char v;
    char b;
    int stride;

    stride = font_width / 8; /* divide by 8 */

    for (y = 0; y < 8; y++) {
        for (x = 0; x < 8; x++) {
            b = font_bits[(8 * gy + y) * stride + gx];
            v = ((b & (1 << x)) != 0) * 0xff;
            pixel(vinfo, finfo, fbp, x + offx, y + offy, v);
        }
    }
}

void draw_char(struct fb_var_screeninfo *vinfo,
               struct fb_fix_screeninfo *finfo,
               char *fbp,
               int offx, int offy,
               char c)
{
    int gx, gy;
    char o;

    o = c - ' '; /* start at 0 */

    gx = o % (font_width >> 3);
    gy = o / (font_width >> 3);

    draw_glyph(vinfo, finfo, fbp, offx, offy, gx, gy);
}

void draw_string(struct fb_var_screeninfo *vinfo,
                 struct fb_fix_screeninfo *finfo,
                 char *fbp,
                 int offx, int offy,
                 const char *str)
{
    int len;
    int n;
    len = strlen(str);

    for (n = 0; n < len; n++) {
        draw_char(vinfo, finfo, fbp, offx + 8*n, offy, str[n]);
    }
}

int main()
{
    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;

    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }

    clearscreen(&vinfo, &finfo, fbp);

    draw_string(&vinfo, &finfo, fbp, 0, 0, "there is");
    draw_string(&vinfo, &finfo, fbp, 0, 8, "one art");
    draw_string(&vinfo, &finfo, fbp, 0, 16, "no more");
    draw_string(&vinfo, &finfo, fbp, 0, 24, "no less");
    draw_string(&vinfo, &finfo, fbp, 0, 32, "to do");
    draw_string(&vinfo, &finfo, fbp, 0, 40, "all things");
    draw_string(&vinfo, &finfo, fbp, 0, 48, "with art-");
    draw_string(&vinfo, &finfo, fbp, 0, 56, "lessness.");

    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}
