#ifndef PGSS_H
#define PGSS_H

#include <fstream>
#include "bytereadwrite.h"

// Leptonica headers needed for Tesseract
#include <leptonica/alltypes.h>

// Tesseract API headers
#include <tesseract/baseapi.h>

namespace pgs_segment
{
    class PCS
    {
    public:
        uint8_t num_objects = 0;
        uint16_t width = 0, height = 0;
        
        void eval(char *&buff);
    };

    class WDS
    {
    public:
        uint16_t x_off = 0, y_off = 0,
            width = 0, height = 0;
        
        void eval(char *&buff);
    };

    class PDS
    {
    public:
        uint32_t colors[255] = {};

        void eval(char *&buff, uint16_t seg_length);
    };

    class ODS
    {
    public:
        uint32_t length = 0;
        char* data;

        void eval(char *&buff);
    };

    class frame
    {
    public:
        uint32_t sub_num = 0;
        double PTS = 0;
        PCS PCS;
        WDS WDS;
        PDS PDS;
        ODS ODS;
        std::ofstream f;

        //
        void reset();

        //
        Pix* decode_rle();

        //
        void decode(char *buff, tesseract::TessBaseAPI* api);

        frame(std::string fname);
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