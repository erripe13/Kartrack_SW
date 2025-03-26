/* DO NOT EDIT THIS FILE */
/* This file is autogenerated by the text-database code generator */

#include <touchgfx/hal/FlashDataReader.hpp>
#include <fonts/ApplicationFontProvider.hpp>
#include <fonts/CompressedUnmappedFontCache.hpp>
#include <fonts/UnmappedDataFont.hpp>

namespace touchgfx
{
GlyphNode UnmappedDataFont::glyphNodeBuffer;

UnmappedDataFont::UnmappedDataFont(const GlyphNode* glyphs, const uint16_t* unicodeList, uint16_t numGlyphs, uint16_t height, uint16_t baseline, uint8_t pixAboveTop, uint8_t pixBelowBottom, uint8_t bitsPerPixel, uint8_t byteAlignRow, uint8_t maxLeft, uint8_t maxRight, const uint8_t* const* glyphDataList, const KerningNode* kerningList, const Unicode::UnicodeChar fallbackChar, const Unicode::UnicodeChar ellipsisChar, const uint16_t* const gsubData, const FontContextualFormsTable* formsTable)
    : Font(height, baseline, pixAboveTop, pixBelowBottom, bitsPerPixel, byteAlignRow, maxLeft, maxRight, fallbackChar, ellipsisChar),
      glyphList(glyphs),
      listSize(numGlyphs),
      unicodes(unicodeList),
      glyphDataList(glyphDataList),
      kerningData(kerningList),
      gsubTable(gsubData),
      arabicTable(formsTable)
{
}

const GlyphNode* UnmappedDataFont::getGlyph(Unicode::UnicodeChar unicode, const uint8_t*& pixelData, uint8_t& bitsPerPixel) const
{
    int index = lookupUnicode(unicode);

    if (index != -1)
    {
        // Read glyphNode from unmapped flash
        touchgfx::FlashDataReader* const flashReader = ApplicationFontProvider::getFlashReader();
        if (flashReader)
        {
            flashReader->copyData(glyphList + index, &glyphNodeBuffer, sizeof(GlyphNode));

            pixelData = getPixelData(const_cast<const GlyphNode*>(&glyphNodeBuffer));
            bitsPerPixel = getBitsPerPixel();
            return &glyphNodeBuffer;
        }
    }
    return 0;
}

const uint8_t* UnmappedDataFont::getPixelData(const GlyphNode* glyph) const
{
    const uint8_t* const* table = (const uint8_t* const*)glyphDataList;
    return &(table[glyph->unicode / 2048][glyph->dataOffset]);
}

int8_t UnmappedDataFont::getKerning(Unicode::UnicodeChar prevChar, const GlyphNode* glyph) const
{
    if (!glyph || glyph->kerningTableSize == 0)
    {
        return 0;
    }

    const KerningNode* kerndata = kerningData + glyph->kerningTablePos();
    for (uint16_t i = glyph->kerningTableSize; i > 0; i--, kerndata++)
    {
        if (prevChar == kerndata->unicodePrevChar)
        {
            return kerndata->distance;
        }
        if (prevChar < kerndata->unicodePrevChar)
        {
            break;
        }
    }
    return 0;
}

int UnmappedDataFont::lookupUnicode(uint16_t unicode) const
{
    int32_t min = 0;
    int32_t max = listSize - 1;

    int32_t mid = min + (unicode - unicodes[min]); // Linear up from [min].unicode
    if (mid < min)
    {
        // Unicode < unicodes[min] => not found
        return -1;
    }
    if (mid > max)
    {
        // Linear up ends too high
        mid = max - (unicodes[max] - unicode); // Linear down from [max].unicode
        if (mid > max)
        {
            // Unicode > unicodes[max] => not found
            return -1;
        }
        if (mid < min)
        {
            // Linear down ends too low, take the middle element
            mid = (min + max) / 2;
        }
    }
    while (min <= max)
    {
        if (unicode == unicodes[mid])
        {
            // Found at [mid]
            return mid;
        }
        if (unicode < unicodes[mid])
        {
            // Unicode is in lower half
            max = mid - 1;
            if (max < min)
            {
                // Range is empty => not found
                break;
            }
            // We adjusted max, try linear down from [max].unicode
            mid = max - (unicodes[max] - unicode);
            if (mid > max)
            {
                // Unicode > [max].unicode => not found
                break;
            }
            if (mid < min)
            {
                // Linear down ends too low, take the middle element
                mid = (min + max) / 2;
            }
        }
        else
        {
            // Unicode is in upper half
            min = mid + 1;
            if (min > max)
            {
                // Range is empty => not found
                break;
            }
            // We adjusted min, try linear up from [min].unicode
            mid = min + (unicode - unicodes[min]);
            if (mid < min)
            {
                // Unicode < [min].unicode => not found
                break;
            }
            if (mid > max)
            {
                // Linear up ends too high, take the middle element
                mid = (min + max) / 2;
            }
        }
    }
    return -1;
}

CompressedUnmappedDataFont::CompressedUnmappedDataFont(const GlyphNode* glyphs, const uint16_t* unicodes, uint16_t numGlyphs, uint16_t height, uint16_t baseline, uint8_t pixAboveTop, uint8_t pixBelowBottom, uint8_t bitsPerPixel, uint8_t byteAlignRow, uint8_t maxLeft, uint8_t maxRight, const uint8_t* const* glyphDataList, const KerningNode* kerningList, const Unicode::UnicodeChar fallbackChar, const Unicode::UnicodeChar ellipsisChar, const uint16_t* const gsubData, const FontContextualFormsTable* formsTable)
    : UnmappedDataFont(glyphs, unicodes, numGlyphs, height, baseline, pixAboveTop, pixBelowBottom, bitsPerPixel, byteAlignRow, maxLeft, maxRight,  glyphDataList, kerningList, fallbackChar, ellipsisChar, gsubData, formsTable)
{
}

const GlyphNode* CompressedUnmappedDataFont::getGlyph(Unicode::UnicodeChar unicode) const
{
    const int index = lookupUnicode(unicode);
    if (index != -1)
    {
        const GlyphNode* glyphNode = glyphList + index;

        // Is the GlyphNode already cached
        const GlyphNode* cachedNode = CompressedUnmappedFontCache::hasCachedGlyphNode(glyphNode);
        if (cachedNode)
        {
            return cachedNode;
        }

        // Read glyphNode from unmapped flash
        touchgfx::FlashDataReader* const flashReader = ApplicationFontProvider::getFlashReader();
        if (flashReader)
        {
            GlyphNode* newGlyphNode = CompressedUnmappedFontCache::cacheGlyphNode(glyphNode);
            flashReader->copyData(glyphList + index, (uint8_t*)newGlyphNode, sizeof(GlyphNode));
            return newGlyphNode;
        }
    }

    return 0;
}

const GlyphNode* CompressedUnmappedDataFont::getGlyph(Unicode::UnicodeChar unicode, const uint8_t*& pixelData, uint8_t& bitsPerPixel) const
{
    const int index = lookupUnicode(unicode);
    if (index != -1)
    {
        const GlyphNode* glyphNode = glyphList + index;
        const uint8_t* pixels = 0;
        const GlyphNode* cachedNode = CompressedUnmappedFontCache::hasCachedGlyphData(glyphNode, pixels);
        if (pixels)
        {
            pixelData = pixels;
            bitsPerPixel = 4;
            return cachedNode;
        }

        // Cache glyphNode if not found already
	if (cachedNode == 0)
	{
	    cachedNode = getGlyph(unicode);
	}
	if (cachedNode->width() == 0)
	{
	    bitsPerPixel = 4;
	    pixelData = 0;
	    return cachedNode;
	}

	// Read data from unmapped flash
	touchgfx::FlashDataReader* const flashReader = ApplicationFontProvider::getFlashReader();
	if (flashReader)
	{
            const uint8_t* const* table = (const uint8_t* const*)glyphDataList;
            const uint32_t dataOffset = cachedNode->dataOffset & 0x3FFFFFFF;
            const uint8_t* compressedData = &(table[unicode / 2048][dataOffset]);
            const uint8_t* cachedData = CompressedUnmappedFontCache::cacheGlyphData(glyphNode, compressedData, flashReader);
            if (cachedData)
            {
                bitsPerPixel = 4;
                pixelData = cachedData;
                return cachedNode;
            }
        }
    }
    return 0;
}

const uint8_t* CompressedUnmappedDataFont::getPixelData(const GlyphNode* glyph) const
{
    return 0;
}
} // namespace touchgfx
