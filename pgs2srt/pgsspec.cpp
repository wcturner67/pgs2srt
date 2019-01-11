#include <leptonica/allheaders.h>

#include "pgsspec.h"


namespace pgs_segment
{
    void PCS::eval(char *&buff)
    {
        this->width = bytestream_get_be16(buff);
        this->height = bytestream_get_be16(buff);
    }

    void WDS::eval(char *&buff)
    {
        buff += 2;
        this->x_off = bytestream_get_be16(buff);
        this->y_off = bytestream_get_be16(buff);
        this->width = bytestream_get_be16(buff);
        this->height = bytestream_get_be16(buff);
    }

    void PDS::eval(char *&buff)
    {
        buff += 3;
        this->Y = bytestream_get_byte(buff);
        this->Cr = bytestream_get_byte(buff);
        this->Cb = bytestream_get_byte(buff);
        this->A = bytestream_get_byte(buff);
    }

    void ODS::eval(char *&buff)
    {
        this->length = bytestream_get_be24(buff);
        this->data = buff;
        buff += this->length;
    }
        
    Pix* frame::decode_rle()
    {
        uint8_t color, Lbuff;
        uint16_t L;
        uint32_t r, c;
        char* b = this->ODS.data;
        char* end = b + this->ODS.length;
        Pix* p = pixCreate(this->WDS.width, this->WDS.height, 8);

        /*
            Note that in example file, first
            ODS data offset starts at 0x197
        */

        for (r = 0; r < this->ODS.length; r++)
        {
            c = 0;
            while (c < this->WDS.width)
            {
                L = 1;
                color = bytestream_get_byte(b);
                if (!color)
                {
                    Lbuff = bytestream_get_byte(b);
                    if (!Lbuff)
                        break;

                    L = Lbuff & 0x3F;
                    if (Lbuff & 0x40)
                        L = (L << 8) | bytestream_get_byte(b);

                    if (Lbuff & 0x80)
                        color = bytestream_get_byte(b);
                }
                L += c;

                for (c; c < L; c++)
                    pixSetPixel(p, c, r, color);
            }
        }

        // For debugging
        FILE* F = fopen("out.bmp", "w");
        pixWriteStreamBmp(F, p);
        fclose(F);

        return p;
    }

    void frame::decode(char *buff, tesseract::TessBaseAPI* api)
    {
        /*
         * Just a note, only every other frame should actually contain information
         * The even frames are used to mark the end time of the odd frames
         */
        if (!this->ODS.data)
            return;

        // Read end time from next segment
        buff += 2;
        std::string end_time = std::to_string((double)bytestream_get_be32(buff) / 9e4);

        this->sub_num++;

        Pix* p = this->decode_rle();
        api->SetImage(p);
        char* text = api->GetUTF8Text();

        this->f <<
            std::to_string(this->sub_num) + '\n'
            + std::to_string(this->PTS) + " --> " + end_time + '\n'
            + text + '\n'
            + '\n';

        // Release memory used by tesseract
        delete[] p, text;
        api->Clear();
    }

    frame::frame(std::string fname) : 
        f(std::ofstream(fname.substr(0, fname.size() - 4) + ".srt")) {};
}