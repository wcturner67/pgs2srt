#pragma once
#ifndef PGSS_H
#define PGSS_H

#include "bytereadwrite.h"
#include <stdint.h>

namespace pgs_segment
{
    class segment
    {
    public:
        double PTS = 0;
        uint16_t size = 0;
    };

    class PCS : public segment
    {
    public:
        uint16_t width = 0;
        uint16_t height = 0;
        uint8_t num_objects = 0;
    };

    class WDS : public segment
    {
    public:
        uint16_t x_off = 0;
        uint16_t width = 0;
        uint16_t y_off = 0;
        uint16_t height = 0;
    };

    class PDS : public segment
    {
    public:
        uint8_t Y = 0;
        uint8_t Cr = 0;
        uint8_t Cb = 0;
        uint8_t A = 0;
    };

    class ODS : public segment
    {
    public:

    };

    class frame
    {
    public:
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