/*
 * assetmanager.h
 */

#pragma once

#include <string>
#include <vector>
#include <stdexcept>

#include "assetfile.h"


class AssetManager {

#pragma pack(push,x1) // Byte alignment (8-bit)
#pragma pack(1)
    struct TGA_HEADER {
        unsigned char IdSize, MapType, ImageType;
        unsigned short PaletteStart, PaletteSize;
        unsigned char PaletteEntryDepth;
        unsigned short X, Y, Width, Height;
        unsigned char ColorDepth, Descriptor;
    };
#pragma pack(pop,x1)

public:
    AssetManager(void* _ioContext = nullptr) { AssetFile::SetContext(_ioContext); }
    virtual ~AssetManager() { }

    std::vector<char> LoadContents(const std::string& fileName) const;
    std::string LoadText(const std::string& fileName) const;
    int ReadAsset(const std::string& fileName, int bytesToRead, void* buffer) const;

    std::vector<char> LoadTGA(const std::string& fileName, int& width, int& height, int& bpp) const;
};
