#pragma once
#include <Image/BsTexture.h>
#include <string>

namespace VDFS
{
  class FileIndex;
}

namespace BsZenLib
{
  /**
   * Import a Gothic zTEX-Texture into bs:f.
   *
   * @param virtualFilePath  Path to the virtual texture file.
   * @param vdfs             Virtual Filesystem to load the texture from.
   *
   * @return bs:f-Texture containing the Data from the Gothic zTEX.
   */
  bs::HTexture ImportTexture(const std::string& virtualFilePath, const VDFS::FileIndex& vdfs);
}
