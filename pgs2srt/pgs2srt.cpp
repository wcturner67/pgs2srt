/* PGS spec site: http://blog.thescorpius.com/index.php/2017/07/15/presentation-graphic-stream-sup-files-bluray-subtitle-format/
*/

#include "pch.h"

/**
 * Construct the frames used in OCR analysis
 * 
 * @param buff pointer to buffer to be processed
 * @param size number of bytes to process
 */
static void process(char* &buff, uint64_t size, std::string &filename)
{
    char *end = buff + size;
    unsigned int PCS = 0, WDS = 0, PDS = 0,
        ODS = 0, END = 0, seg_type, seg_length;
    pgs_segment::frame frame(filename);
    std::string line;

    while (buff < end)
    {
        if (bytestream_get_be16(buff) == 0x5047)
        {
            // Gather PTS info if not already read, otherwise skip
            if (!frame.PTS)
            {
                frame.PTS = (double)bytestream_get_be32(buff) / 9e4;
                buff += 4;
            }
            else
            {
                buff += 8;
            }

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

                frame.PDS.eval(buff);
                buff += seg_length - 7;
                break;
            case OBJECT_SEGMENT:
                ODS++;

                buff += 3;
                if (bytestream_get_byte(buff) != FLIS)
                {
                    // If this comes up a lot, then this feature needs to be implemented
                    std::cout << "Unexpected LISF flag at " << buff << '\n';
                    buff += seg_length - 4;
                    break;
                }

                frame.ODS.eval(buff);
                break;
            case DISPLAY_SEGMENT:
                END++;

                frame.decode(buff); // line goes here when tess implemented
                frame.reset();
                break;
            default:
                std::cout << "Unrecognized segment type " << seg_type << " at "
                    << buff << '\n';
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
    if (filename.empty())
    {
        std::cout << "Filename input not received" << '\n';
        return 5;
    }

    auto start = std::chrono::steady_clock::now();
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
        if (in != 'y' || in != '\n')
        {
            std::cout << "Aborting due to user input" << '\n';
            return 10;
        }
    }
    file.seekg(0);
    char* buff = new char[size];
    file.read(buff, size);
    file.close();
    process(buff, size, filename);

    auto stop = std::chrono::steady_clock::now();
    std::cout << '\n' << "Execution time: " <<
        std::chrono::duration<double, std::milli>(stop - start).count() << " ms" << '\n';
    return 0;
}