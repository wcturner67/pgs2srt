/* PGS spec site: http://blog.thescorpius.com/index.php/2017/07/15/presentation-graphic-stream-sup-files-bluray-subtitle-format/
*/

#include "pgs2srt.h"

/**
 * Construct the frames used in OCR analysis
 * 
 * @param buff pointer to buffer to be processed
 * @param end points to end of buff
 */
static void process(char* &buff, const char* end,
    std::string filename, tesseract::TessBaseAPI* tess)
{
    uint8_t seg_type;
    uint16_t seg_length;
    uint32_t PCS = 0, WDS = 0, PDS = 0,
        ODS = 0, END = 0;
    pgs_segment::frame frame(filename);

    while (buff < end)
    {
        if (bytestream_get_be16(buff) != 0x5047)
        {
            buff--;
            continue;
        }

        // Gather PTS info if not already read, otherwise skip
        if (!frame.PTS)
        {
            frame.PTS = (double)bytestream_get_be32(buff) / 9e4;
            buff += 4;
        }
        else buff += 8;

        seg_type = bytestream_get_byte(buff);
        seg_length = bytestream_get_be16(buff);
        switch (seg_type)
        {
        case PRESENTATION_SEGMENT:
            PCS++;

            frame.PCS.eval(buff);
            buff += seg_length - 4;
            break;
        case WINDOW_SEGMENT:
            WDS++;

            frame.WDS.eval(buff);
            break;
        case PALETTE_SEGMENT:
            PDS++;

            frame.PDS.eval(buff, seg_length);
            break;
        case OBJECT_SEGMENT:
            ODS++;

            buff += 3;
            if (bytestream_get_byte(buff) != FLIS)
            {
                // If this comes up a lot, then this feature needs to be implemented
                std::cout << "Unexpected LIS flag at " << buff << '\n';
                buff += seg_length - 4;
                break;
            }

            frame.ODS.eval(buff);
            break;
        case DISPLAY_SEGMENT:
            END++;

            frame.decode(buff, tess);
            frame.reset();
            break;
        default:
            std::cout << "Unrecognized segment type " << seg_type << " at "
                << buff-10 << '\n';
            break;
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
    if (filename.empty())
    {
        std::cout << "Filename input not received" << '\n';
        return 5;
    }

    auto start = std::chrono::steady_clock::now();

    // Instantiate tesseract
    const char* tessdata = "C:\\Program Files\\tesseract\\data\\";
    tesseract::TessBaseAPI *tess = new tesseract::TessBaseAPI;
    if (tess->Init(tessdata, "eng"))
    {
        std::cout << "Failed to start tesseract" << '\n';
        return 5;
    }

    std::ifstream file (filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        std::cout << "Failed to open file " << filename << '\n';
        return 2;
    }

    uint64_t size = file.tellg();
    if (size > 1e9)
    {
        std::cout << "Warning: filesize exceeds 1GB"
            << '\n' << "Proceed? [Y/n]: ";
        char in = tolower(std::getchar());
        if ((in != 'y') && (in != '\n'))
        {
            std::cout << "Aborting due to user input" << '\n';
            return 10;
        }
    }
    file.seekg(0);
    char* buff = new char[size];
    file.read(buff, size);
    file.close();
    process(buff, buff+size, filename, tess);
    tess->End();

    auto stop = std::chrono::steady_clock::now();
    std::cout << '\n' << "Execution time: " <<
        std::chrono::duration<double>(stop - start).count() << "s" << '\n';
    return 0;
}