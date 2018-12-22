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
        uint16_t ID = 0;
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
        unsigned int sub_num = 0;
        double PTS = 0;
        PCS PCS;
        WDS WDS;
        PDS PDS;
        ODS ODS;
        void reset()
        {
            this->PTS = 0;
            this->PCS = pgs_segment::PCS();
            this->WDS = pgs_segment::WDS();
            this->PDS = pgs_segment::PDS();
            this->ODS = pgs_segment::ODS();
        }

        void decode(char **b)
        {
            if (!this->ODS.data) { return; }

            // TODO
            *b += 2;
            std::string end_time = std::to_string((double)bytestream_get_be32(b) / 9e4);
            *b -= 6;

            this->sub_num++;

            /*
            Just a note, the frame after subs is only used to terminate the frame
            and contains no new information 
            */

            // Replace with ofstream when done
            std::cout << 
                std::to_string(this->sub_num) + '\n'
                + std::to_string(this->PTS) + " --> " + end_time + '\n'
                +'\n';
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

enum SequenceFlag {
    LIS = 0x40,
    FIS = 0x80,
    FLIS = 0xC0
};

#endif