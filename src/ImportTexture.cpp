/**
 * Import zTEX-Texture
 * ===================
 *
 * This Module can convert a Gothic zTEX-Texture to a Texture.
 *
 *
 * Texture files
 * -------------
 *
 * Gothic stores its textures in a proprietary format called ZTEX, which is basically DDS
 * with a different Header and some minor other modifications.
 *
 * Those ZTEX-files are generated by the game by converting TGA files, which can be seen as
 * caching mechanism. Therefore, in the other game files, the original TGA-name will be used.
 *
 * To still load the correct file, we have to convert the input filename of "SAMPLE.TGA" into
 * "SAMPLE-C.TEX", which is the compiled ZTEX file.
 *
 * In case there is no such compiled ZTEX file, we will try to load the original TGA file instead.
 */

#include "ImportTexture.hpp"
#include "ImportPath.hpp"
#include "ResourceManifest.hpp"
#include <FileSystem/BsFileSystem.h>
#include <Image/BsColor.h>
#include <Image/BsPixelData.h>
#include <Image/BsTexture.h>
#include <Resources/BsResources.h>
#include <vdfs/fileIndex.h>
#include <zenload/ztex2dds.h>

using namespace bs;
using namespace BsZenLib;

static std::vector<uint8_t> readCompiledTexture(const String& path, const VDFS::FileIndex& vdfs);
static String replaceExtension(const String& path, const String& newExtension);
static HTexture createRGBA8Texture(const String& name, UINT32 width, UINT32 height,
                                   const std::vector<uint8_t>& rgbaData);
static HTexture createDXTnTexture(const String& name, std::vector<uint8_t>& ddsData,
                                  const ZenLoad::DDSURFACEDESC2& surfaceDesc);

// - Implementation --------------------------------------------------------------------------------

bool BsZenLib::HasCachedTexture(const String& virtualFilePath)
{
  return HasCachedResource(GothicPathToCachedTexture(virtualFilePath.c_str()));
}

HTexture BsZenLib::LoadCachedTexture(const String& virtualFilePath)
{
  Path path = GothicPathToCachedTexture(virtualFilePath.c_str());

  return gResources().load<Texture>(path);
}

bs::HTexture BsZenLib::ImportAndCacheTexture(const bs::String& virtualFilePath,
                                             const VDFS::FileIndex& vdfs)
{
  BS_LOG(Info, Uncategorized, "Caching Texture: " + virtualFilePath);

  HTexture fromOriginal = ImportTexture(virtualFilePath, vdfs);

  if (!fromOriginal) return {};

  const bool overwrite = true;
  Path path = GothicPathToCachedTexture(virtualFilePath.c_str());

  gResources().save(fromOriginal, path, overwrite);
  AddToResourceManifest(fromOriginal, path);

  return fromOriginal;
}

HTexture BsZenLib::ImportTexture(const String& path, const VDFS::FileIndex& vdfs)
{
  std::vector<uint8_t> ztexData = readCompiledTexture(path, vdfs);

  if (ztexData.empty())
  {
    // TODO: Read uncompiled TGA
    return HTexture();
  }

  std::vector<uint8_t> ddsData;
  ZenLoad::convertZTEX2DDS(ztexData, ddsData);

  ZenLoad::DDSURFACEDESC2 surfaceDesc = ZenLoad::getSurfaceDesc(ddsData);

  // return createDXTnTexture(path, ddsData, surfaceDesc);

  // Optionally convert to RGBA8, which is easier to work with
  std::vector<uint8_t> rgbaData;
  ZenLoad::convertDDSToRGBA8(ddsData, rgbaData);
  return createRGBA8Texture(path, surfaceDesc.dwWidth, surfaceDesc.dwHeight, rgbaData);
}

static HTexture createRGBA8Texture(const String& name, UINT32 width, UINT32 height,
                                   const std::vector<uint8_t>& rgbaData)
{
  TEXTURE_DESC desc = {};
  desc.type = TEX_TYPE_2D;
  desc.width = width;
  desc.height = height;
  desc.format = PF_RGBA8;
  desc.hwGamma = true;

  SPtr<PixelData> pixelData = PixelData::create(desc.width,   //
                                                desc.height,  //
                                                desc.depth,   //
                                                PixelFormat::PF_RGBA8);

  std::vector<Color> colors;
  size_t numPixels = desc.width * desc.height;

  assert(numPixels * 4 == rgbaData.size());

  for (size_t i = 0; i < numPixels; i++)
  {
    colors.emplace_back(rgbaData[0 + 4 * i] / 255.0f,   // R
                        rgbaData[1 + 4 * i] / 255.0f,   // G
                        rgbaData[2 + 4 * i] / 255.0f,   // B
                        rgbaData[3 + 4 * i] / 255.0f);  // A
  }

  pixelData->setColors(colors.data(), (UINT32)colors.size());

  MipMapGenOptions mipMapGenOptions = {};
  mipMapGenOptions.filter = MipMapFilter::Kaiser;

  Vector<SPtr<PixelData>> mipMapPixelData = PixelUtil::genMipmaps(*pixelData, mipMapGenOptions);

  desc.numMips =
      (UINT32)mipMapPixelData.size() - 1;  // -1 since the base level is not included in that count
  HTexture texture = Texture::create(desc);

  for (size_t i = 0; i < mipMapPixelData.size(); i++)
  {
    texture->writeData(mipMapPixelData[i], 0, (UINT32)i);
  }

  texture->writeData(pixelData);
  texture->setName(name);

  return texture;
}

static HTexture createDXTnTexture(const String& name, std::vector<uint8_t>& ddsData,
                                  const ZenLoad::DDSURFACEDESC2& surfaceDesc)
{
  TEXTURE_DESC desc = {};
  desc.type = TEX_TYPE_2D;
  desc.width = surfaceDesc.dwWidth;
  desc.height = surfaceDesc.dwHeight;
  desc.numMips = surfaceDesc.dwMipMapCount;

  switch (surfaceDesc.ddpfPixelFormat.dwFourCC)
  {
    case MAKEFOURCC('D', 'X', 'T', '1'):
      desc.format = PF_BC1;
      break;

    case MAKEFOURCC('D', 'X', 'T', '3'):
      desc.format = PF_BC2;
      break;

    case MAKEFOURCC('D', 'X', 'T', '5'):
      desc.format = PF_BC3;
      break;
    default:
      BS_EXCEPT(InternalErrorException,
                "Cannot import DXTn-texture " + name + ", unsupported FOURCC!")
  }

  HTexture texture = Texture::create(desc);

  for (UINT32 i = 0; i < surfaceDesc.dwMipMapCount; i++)
  {
    UINT32 mipWidth, mipHeight, mipDepth;
    PixelUtil::getSizeForMipLevel(desc.width, desc.height, 1, i, mipWidth, mipHeight, mipDepth);

    SPtr<PixelData> pixelData = PixelData::create(mipWidth,   //
                                                  mipHeight,  //
                                                  mipDepth,   //
                                                  desc.format);

    size_t mipOffset = ZenLoad::getMipFileOffsetFromDDS(ddsData, i);

    UINT8* imageStart = (UINT8*)&ddsData[mipOffset];

    pixelData->setExternalBuffer(imageStart);

    texture->writeData(pixelData, 0, i);
  }

  texture->setName(name);

  return texture;
}

static std::vector<uint8_t> readCompiledTexture(const String& path, const VDFS::FileIndex& vdfs)
{
  String compiledFile = path;

  if (compiledFile.find(".TGA") != String::npos)
  {
    compiledFile = replaceExtension(path, "-C.TEX");
  }

  std::vector<uint8_t> fileData;
  vdfs.getFileData(compiledFile.c_str(), fileData);

  return fileData;
}

static String replaceExtension(const String& path, const String& newExtension)
{
  // assert(path.length() >= 4);

  if (path.length() < 4) return path;

  return path.substr(0, path.length() - 4) + newExtension;
}
