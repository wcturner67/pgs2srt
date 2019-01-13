#include "colorspace.h"
#include "bytereadwrite.h"

inline uint32_t RGBA(uint8_t R, uint8_t G, uint8_t B , uint8_t A)
{
    //return A << 24 | R << 16 | G << 8 | B;
    return B << 24 | G << 16 | R << 8 | A;
}

uint32_t ycc2rgb(char *&buff)
{
    uint8_t R, G, B;
    double Y = bytestream_get_byte(buff) - 16,
        Cr = bytestream_get_byte(buff) - 128,
        Cb = bytestream_get_byte(buff) - 128,
        A = bytestream_get_byte(buff);

    R = 1.164*Y            + 1.793*Cr;
    G = 1.164*Y - 0.213*Cb - 0.533*Cr;
    B = 1.164*Y - 2.112*Cb;
    
    return RGBA(R, G, B, A);
}