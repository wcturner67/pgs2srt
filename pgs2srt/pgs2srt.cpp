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
    int headers = 0;
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
    frame.decode_rle();

    while (buff < end)
    {
        if (bytestream_get_be16(b) == 0x5047)
        {
            // Gather PTS info if not already read, otherwise skip
            if (!frame.PTS)
            {
                frame.PTS = (double)bytestream_get_be32(b) / 90000;
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

                frame.PCS.width = bytestream_get_be16(b);
                frame.PCS.height = bytestream_get_be16(b);
                break;
            case WINDOW_SEGMENT:
                WDS++;
                break;
            case PALETTE_SEGMENT:
                PDS++;
                break;
            case OBJECT_SEGMENT:
                ODS++;
                break;
            case DISPLAY_SEGMENT:
                END++;
                frame.reset();
                break;
            default:
                break;
            }
            buff += bytestream_get_be16(&buff);
        }
        else
        {
            buff--;
        }
    }
    std::cout 
        << "PCS: " << PCS << std::endl
        << "WDS: " << WDS << std::endl
        << "PDS: " << PDS << std::endl
        << "ODS: " << ODS << std::endl
        << "END: " << END << std::endl;
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
    std::cout << std::endl << "Execution time: " << 
        std::chrono::duration<double, std::milli>(stop - start).count() << " ms" << std::endl;
    return 0;
}