#include <leptonica/allheaders.h>

#include "pgsspec.h"
#include "colorspace.h"


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

    void PDS::eval(char *&buff, uint16_t seg_length)
    {
        char* end = buff + seg_length;

        // First PDS in example file starts at 0x44
        uint8_t id = bytestream_get_byte(buff);
        buff += 1;

        while (buff < end)
        {
            uint8_t c_id = bytestream_get_byte(buff);
            this->colors[c_id] = ycc2rgb(buff);
        }
    }

    void ODS::eval(char *&buff)
    {
        this->length = bytestream_get_be24(buff)-4;
        buff += 4;
        this->data = buff;
        buff += this->length;
    }

    void frame::reset()
    {
        this->PTS = 0;
        this->PCS = pgs_segment::PCS();
        this->WDS = pgs_segment::WDS();
        this->PDS = pgs_segment::PDS();
        this->ODS = pgs_segment::ODS();
    }

    // For debugging only
    inline void print_bmp(Pix* p)
    {
        pixWrite("out.bmp", p, 1);
    }
        
    Pix* frame::decode_rle()
    {
        uint8_t Lbuff;
        uint16_t L;
        uint32_t r, c, color;
        char* b = this->ODS.data;
        char* end = b + this->ODS.length;
        Pix* p = pixCreate(this->WDS.width, this->WDS.height, 32);
        uint32_t* d = p->data;

        for (r = 0; r < this->WDS.height; r++)
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
                    pixSetPixel(p, c, r, this->PDS.colors[color]);
                //memset(p->data + r*(this->WDS.width) + c, this->PDS.colors[color], L);
                //c += L;
            }
        }
        
        // Convert to gray-scale to fill in gaps...
        Pix *p2 = pixScaleRGBToGray2(p, 0.33, 0.33, 0.33);
        pixDestroy(&p);

        // ... then binarize and invert for tesseract
        p = pixScaleGrayToBinaryFast(p2, 1, 10);
        pixDestroy(&p2);
        p2 = pixInvert(nullptr, p);
        pixDestroy(&p);
        p2->xres = p2->yres = 100;
        
        print_bmp(p2); // For debugging only, remove when done implementing this function
        return p2;
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
            + text + '\n';

        // Release memory used by tesseract - this isn't entirely working!
        pixDestroy(&p);
        delete[] text;
        api->Clear();
    }

    frame::frame(std::string fname) : 
        f(std::ofstream(fname.substr(0, fname.size() - 4) + ".srt")) {};
}
