#ifndef PGSS_H
#define PGSS_H

#include "bytereadwrite.h"

// Leptonica headers needed for Tesseract
#include <leptonica\environ.h>
#include <leptonica\pix.h>

// Tesseract API headers
#include <tesseract\baseapi.h>

#define RGBA(r,g,b,a) (((unsigned)(a) << 24) | ((r) << 16) | ((g) << 8) | (b))

namespace pgs_segment
{

    class PCS
    {
    public:
        uint16_t width = 0;
        uint16_t height = 0;
        uint8_t num_objects = 0;
        void eval(char *&buff)
        {
            this->width = bytestream_get_be16(buff);
            this->height = bytestream_get_be16(buff);
        }
    };

    class WDS
    {
    public:
        uint16_t x_off = 0;
        uint16_t width = 0;
        uint16_t y_off = 0;
        uint16_t height = 0;
        void eval(char *&buff)
        {
            buff += 2;
            this->x_off = bytestream_get_be16(buff);
            this->y_off = bytestream_get_be16(buff);
            this->width = bytestream_get_be16(buff);
            this->height = bytestream_get_be16(buff);
        }
    };

    class PDS
    {
    public:
        uint8_t Y = 0;
        uint8_t Cr = 0;
        uint8_t Cb = 0;
        uint8_t A = 0;
        void eval(char *&buff)
        {
            buff += 3;
            this->Y = bytestream_get_byte(buff);
            this->Cr = bytestream_get_byte(buff);
            this->Cb = bytestream_get_byte(buff);
            this->A = bytestream_get_byte(buff);
        }
    };

    class ODS
    {
    public:
        uint16_t ID = 0;
        uint32_t length = 0;
        char* data;
        void eval(char *&buff)
        {
            this->length = bytestream_get_be24(buff);
            this->data = buff;
            buff += this->length;
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
        std::ofstream f;

        void reset()
        {
            this->PTS = 0;
            this->PCS = pgs_segment::PCS();
            this->WDS = pgs_segment::WDS();
            this->PDS = pgs_segment::PDS();
            this->ODS = pgs_segment::ODS();
        }

        Pix* decode_rle()
        {
            char* b = this->ODS.data;
            char* end = this->ODS.length + b;
            uint64_t size = this->WDS.height * this->WDS.width;
            Pix* p = new Pix;
            p->h = this->WDS.height;
            p->w = this->WDS.width;
            for (int i = 0; i < size; i++)
            {
                p->data[i] = 1;
            }
            return p;
        }

        void decode(char *buff)
        {
            /*
             * Just a note, only every other frame should actually contain information
             * The even frames are used to mark the end time of the odd
             */
            if (!this->ODS.data) { return; }

            // Read end time from next segment
            buff += 2;
            std::string end_time = std::to_string((double)bytestream_get_be32(buff) / 9e4);

            this->sub_num++;

            tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
            Pix* p = this->decode_rle();
            api->SetImage(p);
            char* text = api->GetUTF8Text();

            // Replace with ofstream when done
            this->f <<
                std::to_string(this->sub_num) + '\n'
                + std::to_string(this->PTS) + " --> " + end_time + '\n'
                + text + '\n'
                + '\n';

            delete[] p;
            delete[] text;
            api->End();
        }

        frame (std::string &fname) : 
            f(std::ofstream(fname.substr(0, fname.size()-4) + ".srt")) {};
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

#endif //PGSS_H