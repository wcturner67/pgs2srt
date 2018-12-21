#pragma once
#ifndef PGSS_H
#define PGSS_H

#include "bytereadwrite.h"
#include <stdint.h>

namespace pgs_segment
{

    class PCS
    {
    public:
        uint16_t width = 0;
        uint16_t height = 0;
        uint8_t num_objects = 0;
        void eval(char **b)
        {
            this->width = bytestream_get_be16(b);
            this->height = bytestream_get_be16(b);
        }
    };

    class WDS
    {
    public:
        uint16_t x_off = 0;
        uint16_t width = 0;
        uint16_t y_off = 0;
        uint16_t height = 0;
        void eval(char **b)
        {
            *b += 2;
            this->x_off = bytestream_get_be16(b);
            this->y_off = bytestream_get_be16(b);
            this->width = bytestream_get_be16(b);
            this->height = bytestream_get_be16(b);
        }
    };

    class PDS
    {
    public:
        uint8_t Y = 0;
        uint8_t Cr = 0;
        uint8_t Cb = 0;
        uint8_t A = 0;
        void eval(char **b)
        {
            *b += 3;
            this->Y = bytestream_get_byte(b);
            this->Cr = bytestream_get_byte(b);
            this->Cb = bytestream_get_byte(b);
            this->A = bytestream_get_byte(b);
        }
    };

    class ODS
    {
    public:
        uint32_t length = 0;
        char* data;
        void eval(char **b)
        {
            this->length = bytestream_get_be24(b);
            this->data = *b;
            *b += this->length;
        }
    };

    class frame
    {
    public:
        double PTS = 0;
        PCS PCS;
        WDS WDS;
        PDS PDS;
        ODS ODS;
        static void reset()
        {

        }

        static void decode_rle()
        {
            
        }
    };
}

enum SegmentType {
    PALETTE_SEGMENT = 0x14,
    OBJECT_SEGMENT = 0x15,
    PRESENTATION_SEGMENT = 0x16,
    WINDOW_SEGMENT = 0x17,
    DISPLAY_SEGMENT = 0x80,
};

#endif