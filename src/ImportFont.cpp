#include "ResourceManifest.hpp"
#include <BsZenLib/ImportFont.hpp>
#include <BsZenLib/ImportPath.hpp>
#include <BsZenLib/ImportTexture.hpp>
#include <FileSystem/BsFileSystem.h>
#include <Math/BsVector2.h>
#include <Resources/BsResources.h>
#include <Text/BsFont.h>
#include <vdfs/fileIndex.h>
#include <zenload/zCFont.h>

class ImportFont
{
public:
  ImportFont(const bs::String& originalFileName, const VDFS::FileIndex& vdfs)
      : mZFont(originalFileName.c_str(), vdfs)
      , mFileIndex(vdfs)
  {
    //
  }

  /**
   * @return Whether loading was successfull. Should be checked before calling any of
   *         the other functions.
   */
  bool hasLoadSucceeded() { return mZFont.isValid(); }

  /**
   * Packs the original font into a bs:f font object
   */
  bs::HFont constructFont()
  {
    using namespace bs;

    if (!hasLoadSucceeded())
    {
      BS_EXCEPT(InvalidStateException, "Cannot import bitmap of non-loaded font.");
    }

    auto bitmap = bs::bs_shared_ptr_new<bs::FontBitmap>(makeFontBitmap());
    bs::Vector<bs::SPtr<bs::FontBitmap>> pages = {bitmap};

    return bs::Font::create(pages);
  }

private:
  /**
   * Fills a bs::FontBitmap structure which can then be plugged into a bs::Font.
   */
  bs::FontBitmap makeFontBitmap()
  {
    bs::FontBitmap result;

    result.size = mZFont.getFontInfo().fontHeight;
    result.baselineOffset = 0;  // Is that correct?
    result.lineHeight = mZFont.getFontInfo().fontHeight;
    result.missingGlyph = chardescOf('#');
    result.spaceWidth = chardescOf(' ').width;
    result.texturePages.push_back(importFontBitmap());

    for (bs::UINT16 i = 0; i < 256; i++)
    {
      bs::CharDesc desc = chardescOf((uint8_t)i);

      result.characters[desc.charId] = desc;
    }

    return result;
  }

  /**
   * @return The file name of the font bitmap to use
   */
  bs::String getFontTextureFileName() { return mZFont.getFontInfo().fontName.c_str(); }

  /**
   * Imports/Caches the bitmap to be used for this font
   */
  bs::HTexture importFontBitmap()
  {
    bs::String fontBitmapFile = getFontTextureFileName();

    if (BsZenLib::HasCachedTexture(fontBitmapFile))
    {
      return BsZenLib::LoadCachedTexture(fontBitmapFile);
    }
    else
    {
      return BsZenLib::ImportTexture(fontBitmapFile, mFileIndex);
    }
  }

  /**
   * Tries to build a chardesc as best as possible. This is tricky
   * because Gothic does encoding via the used bitmap texture
   * only. We'll see how that works out...
   */
  bs::CharDesc chardescOf(uint8_t c)
  {
    bs::CharDesc desc;

    const auto& source = mZFont.getFontInfo();

    desc.charId = c;  // Probably wrong, 'c' can be ANY 8-bit encoding and 'charId' is supposed to be
                      // 32-bit unicode.

    desc.height = source.fontHeight;
    desc.width = (bs::UINT32)source.glyphWidth[c];
    desc.page = 0;
    desc.xAdvance = desc.width;
    desc.yAdvance = desc.height;
    desc.xOffset = 0;
    desc.yOffset = 0;

    const ZMath::float2& uvTopLeft = source.fontUV1[c];
    const ZMath::float2& uvBotRight = source.fontUV2[c];

    desc.uvWidth = uvBotRight.x - uvTopLeft.x;
    desc.uvHeight = uvBotRight.y - uvTopLeft.y;
    desc.uvX = uvTopLeft.x;
    desc.uvY = uvTopLeft.y;

    return desc;
  }

private:
  ZenLoad::zCFont mZFont;
  const VDFS::FileIndex& mFileIndex;
};

bs::HFont BsZenLib::ImportAndCacheFont(const bs::String& originalFileName,
                                       const VDFS::FileIndex& vdfs)
{
  ImportFont importer(originalFileName, vdfs);

  if (!importer.hasLoadSucceeded())
  {
    return {};
  }

  bs::HFont font = importer.constructFont();

  const bool overwrite = true;
  bs::gResources().save(font, GothicPathToCachedFont(originalFileName), overwrite);
  AddToResourceManifest(font, GothicPathToCachedFont(originalFileName));

  return font;
}

bs::HFont BsZenLib::LoadCachedFont(const bs::String& originalFileName)
{
  return bs::gResources().load<bs::Font>(GothicPathToCachedFont(originalFileName));
}

bool BsZenLib::HasCachedFont(const bs::String& originalFileName)
{
  return HasCachedResource(GothicPathToCachedFont(originalFileName));
}
