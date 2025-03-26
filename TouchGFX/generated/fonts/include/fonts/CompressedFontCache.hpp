/* DO NOT EDIT THIS FILE */
/* This file is autogenerated by the text-database code generator */

#ifndef TOUCHGFX_COMPRESSEDFONTCACHE_HPP
#define TOUCHGFX_COMPRESSEDFONTCACHE_HPP

#include <touchgfx/hal/Types.hpp>
#include <touchgfx/Font.hpp>

namespace touchgfx
{
class CompressedFontCache
{
public:
    static void clearCache();
    static const uint8_t* hasCachedGlyph(const GlyphNode* glyphNode);
    static void unableToCache(const GlyphNode* glyphNode, int byteSize);
    static const uint8_t* cacheGlyph(const GlyphNode* glyph, const uint8_t* compressedData);
    static int usedMemory()
    {
        return pixelsTop - (uint8_t*)bitmapFontCache + glyphsAllocated * sizeof(BitmapFontCacheKey);
    }

    static int cacheClearCounter;
private:
    static uint8_t* decompressGlyph(uint8_t* pixelsTop, const GlyphNode* glyphNode, const uint8_t* compressedData);

    struct BitmapFontCacheKey
    {
        const void* glyphNode;
        const uint8_t* pixels;
    };

    static const int cacheSizeBytes = 4096;
    static const int cacheWords = (cacheSizeBytes + 3) / 4;
    static uint32_t bitmapFontCache[cacheWords];
    static uint8_t* pixelsTop;
    static int glyphsAllocated;
};
} // namespace touchgfx

#endif // TOUCHGFX_COMPRESSEDFONTCACHE_HPP
