/* PGS spec site: http://blog.thescorpius.com/index.php/2017/07/15/presentation-graphic-stream-sup-files-bluray-subtitle-format/
*/

#include "pch.h"

/**
 * Construct the frames used in OCR analysis
 * 
 * @param buff pointer to buffer to be processed
 * @param size number of bytes to process
 */
static void process(char* buff, uint64_t size)
{
    char *end = buff + size;
    auto b = &buff;
    unsigned int PCS = 0;
    unsigned int WDS = 0;
    unsigned int PDS = 0;
    unsigned int ODS = 0;
    unsigned int END = 0;
    unsigned int seg_type;
    unsigned int seg_length;
    pgs_segment::frame frame;

    while (buff < end)
    {
        if (bytestream_get_be16(b) == 0x5047)
        {
            // Gather PTS info if not already read, otherwise skip
            if (!frame.PTS)
            {
                frame.PTS = (double)bytestream_get_be32(b) / 9e4;
                buff += 4;
            }
            else
            {
                buff += 8;
            }

            seg_type = bytestream_get_byte(b);
            seg_length = bytestream_get_be16(b);
            switch (seg_type)
            {
            case PRESENTATION_SEGMENT:
                PCS++;

                frame.PCS.eval(b);
                buff += seg_length - 4;
                break;
            case WINDOW_SEGMENT:
                WDS++;

                frame.WDS.eval(b);
                break;
            case PALETTE_SEGMENT:
                PDS++;

                frame.PDS.eval(b);
                buff += seg_length - 7;
                break;
            case OBJECT_SEGMENT:
                ODS++;

                buff += 3;
                if (bytestream_get_byte(b) != 0xC0)
                {
                    // If this comes up a lot, then this feature needs to be implemented
                    std::cout << "Unexpected LISF flag at " << buff << '\n';
                    buff += seg_length - 4;
                    break;
                }

                frame.ODS.eval(b);
                break;
            case DISPLAY_SEGMENT:
                END++;

                frame.decode_rle();
                frame.reset();
                break;
            default:
                break;
            }
        }
        else
        {
            buff--;
        }
    }
    std::cout 
        << "PCS Segments: " << PCS << '\n'
        << "WDS Segments: " << WDS << '\n'
        << "PDS Segments: " << PDS << '\n'
        << "ODS Segments: " << ODS << '\n'
        << "END Segments: " << END << '\n';
}

int main(int argc, char** argv)
{
    // Parse inputs
    std::string filename;
    for (int i = 2; i < argc; i++)
    {
        if (std::string(argv[i-1]) == "-i")
        {
            filename = argv[i];
            break;
        }
    }

    auto start = std::chrono::steady_clock::now();
    std::ifstream file (filename, std::ios::binary | std::ios::ate);
    if (file.is_open())
    {
        uint64_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        char* buff = new char[size];
        file.read(buff, size);
        process(buff, size);
    }
    else
    {
        std::cout << "Failed to open file" << std::endl;
        return 2;
    }

    file.close();
    auto stop = std::chrono::steady_clock::now();
    std::cout << '\n' << "Execution time: " <<
        std::chrono::duration<double, std::milli>(stop - start).count() << " ms" << '\n';
    return 0;
}