#pragma once

#include <string>
#include <fstream>
#include <stdio.h>
#include <jadel/jadel.h>


struct BinaryParser
{
    const void* file;
    const uint8* pointer;
};

inline bool initParser(const void* file, BinaryParser* parser)
{
    if (!file) return false;
    parser->file = file;
    parser->pointer = (uint8*)file;
    return true;
}

inline uint8 getUint8(BinaryParser* parser)
{
    uint8 result = *((uint8*)parser->pointer);
    parser->pointer += sizeof(uint8);
    return result;
}

inline uint16 getUint16(BinaryParser* parser)
{
    uint16 result = *((uint16*)parser->pointer);
    parser->pointer += sizeof(uint16);
    return result;
}

inline uint32 getUint32(BinaryParser* parser)
{
    uint32 result = *((uint32*)parser->pointer);
    parser->pointer += sizeof(uint32);
    return result;
}

inline uint64 getUint64(BinaryParser* parser)
{
    uint64 result = *((uint64*)parser->pointer);
    parser->pointer += sizeof(uint64);
    return result;
}

inline int8 getInt8(BinaryParser* parser)
{
    int8 result = *((int8*)parser->pointer);
    parser->pointer += sizeof(int8);
    return result;
}

inline int16 getInt16(BinaryParser* parser)
{
    int16 result = *((int16*)parser->pointer);
    parser->pointer += sizeof(int16);
    return result;
}

inline int32 getInt32(BinaryParser* parser)
{
    int32 result = *((int32*)parser->pointer);
    parser->pointer += sizeof(int32);
    return result;
}

inline int64 getInt64(BinaryParser* parser)
{
    int64 result = *((int64*)parser->pointer);
    parser->pointer += sizeof(int64);
    return result;
}


inline std::string loadTextFile(const std::string &filepath)
{
    std::string text = "";
    std::ifstream file;
    file.open(filepath);
    if (!file)
    {
//        std::cout << "Could not open file: " << filepath << std::endl;
        return text;
    }
    std::string line;

    if (file.is_open())
    {
        while (getline(file, line))
        {
            text.append(line + "\n");
        }
        file.close();
    }
    return text;
}
/*
inline bool loadTextFile(const char* filepath, String* target)
{
    if (!target)
    {
        err("Could not load text file: %s - target string was NULL!\n");
        return false;
    }
    FILE* fp = fopen(filepath, "r");
    if (!fp)
    {
        err("Could not open text file: %s\n", filepath);
        return false;
    }
    //Determine size of file;
    
    fseek(fp, 0, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    target->init(fileSize, fileSize);
    fread(target->content, 1, fileSize, fp);
    fclose(fp);
    return true;
}
*/
inline uint32 determineFileSize(FILE* file)
{
    if (!file)
    {
        //err("Could not open binary file: %s\n", filepath);
        return 0;
    }
    //Determine size of file
    fseek(file, 0, SEEK_END); 
    size_t result = ftell(file); 
    fseek(file, 0, SEEK_SET);
    //fclose(fp);
    return result;
}

inline bool loadBinaryFile(const char* filepath, void* ptr, size_t numBytes)
{
    FILE* fp = fopen(filepath, "rb");
    if (!fp)
    {
        //err("Could not open binary file: %s\n", filepath);
        return false;
    }
    //Determine size of file
    fseek(fp, 0, SEEK_END); 
    size_t fileSize = ftell(fp); 
    fseek(fp, 0, SEEK_SET);

    size_t bytesToRead;
    if (numBytes > fileSize)
    {
//        warn("Tried to read bytes past end of file: %s\n", filepath); 
        bytesToRead = fileSize;
    }
    else
    {
        bytesToRead = numBytes;
    }
    fread(ptr, 1, bytesToRead, fp);
    fclose(fp);
    return true;
}


inline bool loadBinaryFile(FILE* file, void* ptr, size_t numBytes)
{
    if (!file)
    {
        //err("Could not open binary file: %s\n", filepath);
        return false;
    }

    fread(ptr, 1, numBytes, file);
    fclose(file);
    return true;
}

inline bool saveBinaryFile(const char* filepath, void* data, size_t numBytes)
{
    FILE* fp = fopen(filepath, "wb");
    if (!fp)
    {
//        err("Could not open binary file: %s for writing!\n", filepath);
        return false;
    }
    fwrite(data, numBytes, 1, fp);  
    fclose(fp);
    return true;
}

struct Bitmap
{
    // Header
    char signature[2]; // Should be "BM"
    uint32 fileSize;
    uint32 reservedH; // Unused, should be 0
    uint32 dataOffset;
    //InfoHeader
    uint32 size; //size of InfoHeader = 40
    uint32 width;
    uint32 height;
    uint16 planes;
    uint16 BPP;
    uint32 compression;
    uint32 imageSize;
    uint32 xPixelsPerM;
    uint32 yPixelsPerM;
    uint32 colorsUsed;
    uint32 importantColors; // 0 = all
    // Color table
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 reservedCT; // Unused, should be 0
};

inline bool loadBMP(const char* filepath, jadel::Surface* surface)
{
    FILE* file = fopen(filepath, "rb");
    if (!file) return false;
    
    size_t fileSize = determineFileSize(file);
    if (fileSize == 0) return false;

    void* bitmapBinary = malloc(fileSize);
    loadBinaryFile(file, bitmapBinary, fileSize);
    
    BinaryParser parser;
    initParser(bitmapBinary, &parser);
    
    Bitmap bitmap;
    // Header
    bitmap.signature[0] = getUint8(&parser); // Should be "BM"
    bitmap.signature[1] = getUint8(&parser);
 

    char tempSignature[3] = {bitmap.signature[0], bitmap.signature[1], 0};
    int compareResult = strcmp(tempSignature, "BM");
    if (compareResult != 0) return false;
    bitmap.fileSize = getUint32(&parser);
    bitmap.reservedH = getUint32(&parser); // Unused, should be 0
    bitmap.dataOffset = getUint32(&parser);
    //InfoHeader
    bitmap.size = getUint32(&parser); //size of InfoHeader = 40
    bitmap.width = getUint32(&parser);
    bitmap.height = getUint32(&parser);
    bitmap.planes = getUint16(&parser);
    bitmap.BPP = getUint16(&parser);
    bitmap.compression = getUint32(&parser);
    bitmap.imageSize = getUint32(&parser);
    bitmap.xPixelsPerM = getUint32(&parser);
    bitmap.yPixelsPerM = getUint32(&parser);
    bitmap.colorsUsed = getUint32(&parser);
    bitmap.importantColors = getUint32(&parser); // 0 = all
    // Color table
    bitmap.red = getUint8(&parser);
    bitmap.green = getUint8(&parser);
    bitmap.blue = getUint8(&parser);
    bitmap.reservedCT = getUint8(&parser); // Unused, should be 0
    
    size_t pixelsSize = bitmap.width * bitmap.height * (bitmap.BPP / 8);
    uint32* bitmapPixels = (uint32*)malloc(pixelsSize);

    surface->pixels = (uint32*)memcpy(bitmapPixels, (uint8*)parser.file + bitmap.dataOffset, pixelsSize);
    strncpy(surface->name, filepath, 39);
    surface->name[39] = 0;


    surface->width = bitmap.width;
    surface->height = bitmap.height;
    free(bitmapBinary);
    return true;
}
