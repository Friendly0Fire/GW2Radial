#pragma once
#include <Main.h>

inline std::vector<byte> SaveTGA(const std::span<byte>& source, uint width, uint height, uint bpp, size_t pitch = 0)
{
    const uint bytesPerPixel = bpp / 8;
    std::vector<byte> output;

#pragma pack(1)
    struct TGAHeader {
        char  idlength;
        char  colourmaptype;
        char  datatypecode;
        short int colourmaporigin;
        short int colourmaplength;
        char  colourmapdepth;
        short int x_origin;
        short int y_origin;
        short width;
        short height;
        char  bitsperpixel;
        char  imagedescriptor;
    };

    output.resize(sizeof(TGAHeader) + width * height * bytesPerPixel);
    auto* header = reinterpret_cast<TGAHeader*>(output.data());

    header->idlength = 0;
    header->colourmaptype = 0;
    header->datatypecode = 2;

    header->colourmaporigin = 0;
    header->colourmaplength = 0;
    header->colourmapdepth = 0;
    
    header->x_origin = 0;
    header->y_origin = 0;
    header->width = short(width);
    header->height = short(height);
    header->bitsperpixel = char(bpp);
    header->imagedescriptor = char((bpp == 32 ? 8 : 0) + (1 << 5));

    byte* const baseOutput = &output[sizeof(TGAHeader)];

    if(pitch == 0)
        pitch = width * bytesPerPixel;

    for(uint i = 0; i < height; i++)
    {
        byte* lineOutput = baseOutput + i * width * bytesPerPixel;
        byte* lineInput = source.data() + i * pitch;
        memcpy_s(lineOutput, width * bytesPerPixel, lineInput, pitch);
    }

    return output;
}