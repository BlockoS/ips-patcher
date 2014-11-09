#include <iostream>
#include <cstring>
#include "log.h"
#include "ips.h"
#include "io.h"

/**
 * Print usage.
 */
void usage()
{
    std::cerr << "usage: ips-patcher-cli source destination patch" << std::endl;
    std::cerr << "       Apply IPS patch to \"source\" file and write output to \"destination\"." << std::endl;
}

/**
 * Apply patch to input file and write output to another file.
 * @param [in] in    Input filename.
 * @param [in] out   Output filename.
 * @param [in] patch IPS patch.
 */
bool apply(const char* in, const char* out, IPS::Patch const& patch)
{
    FILE *input;
    FILE *output;
    
    input = fopen(in, "rb");
    if(nullptr == input)
    {
        Error("Failed to open %s: %s", in, strerror(errno));
        return false;
    }
    
    output = fopen(out, "wb");
    if(nullptr == output)
    {
        Error("Failed to open %s: %s", out, strerror(errno));
        fclose(input);
        return false;
    }

    size_t n;
    bool ret = true;
    
    // Copy file.
    while(!feof(input) && ret)
    {
        uint8_t byte;
        n = fread(&byte, 1, 1, input);
        if(1 == n)
        {
            n = fwrite(&byte, 1, 1, output);
            if(1 != n)
            {
                Error("Write failure: %s", strerror(errno));
                ret = false;
            }
        }
        else if(!feof(input))
        {
            Error("Read failure: %s %d", strerror(errno), n);
            ret = false;
        }
    }
    fclose(input);
    
    if(false == ret)
    {
        fclose(output);
        return ret;
    }
    
    // Write records.
    ret = true;
    for(size_t i=0; ret && (i<patch.count()); i++)
    {
        IPS::Record const& record = patch[i];
        fseek(output, record.offset, SEEK_SET);
        if(record.rle)
        {
            uint8_t byte = static_cast<uint8_t>(record.data);
            for(size_t j=0; ret && (j<record.size); j++)
            {
                n = fwrite(&byte, 1, 1, output);
                if(1 != n)
                {
                    Error("Failed to write record #%d: %s", i, strerror(errno));
                    ret = false;
                }
            }
        }
        else
        {
            n = fwrite(reinterpret_cast<uint8_t*>(record.data), 1, record.size, output);
            if(record.size != n)
            {
                Error("Failed to write record #%d: %s", i, strerror(errno));
                ret = false;
            }
        }
    }
    fclose(output);
    return ret;
}

/**
 * Main entry point.
 */
int main(int argc, char** argv)
{
    if(argc < 3)
    {
        usage();
        return 0;
    }

    Log::Logger& logger = Log::Logger::instance();
    Log::Output* output = new Log::Output();
    
    IPS::Patch  patch;
    IPS::IO     io;
    bool ret;
    
    logger.begin(output);
    
    ret = io.read(argv[3], patch);
    if(false == ret)
    {
        Error("Failed to read %s", argv[3]);
    }
    else
    {
        apply(argv[1], argv[2], patch);
    }
    
    logger.end();

    free(output);

    return 0;
}
