/** \file
 * Import and Cache Textures.
 */

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
   * Import a Gothic zTEX-Texture without saving the results to disk.
   *
   *
   * While the creators of Gothic worked mostly with TGA-files, the engine converted/compressed them automatically to
   * a DDS like format, called zTEX. Given an input file `IMAGE.TGA`, the engine would produce a file called `IMAGE-C.TEX`,
   * called the *compiled* file.
   *
   * This function will interpret these .TEX-files and load them into a BsTexture.
   *
   * @note  Given a .TGA-file as input, this function will automatically look for the correct compiled .TEX-file.
   *
   * @param virtualFilePath  Path to .TGA or .TEX file in the VDFS.
   * @param vdfs             Virtual Filesystem to load the texture from.
   *
   * @return BsTexture containing the Data from the Gothic zTEX. Empty handle if importing failed.
   */
  bs::HTexture ImportTexture(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs);

  /** 
   * Import a Gothic zTEX-Texture and save the results to disk.
   *
   *
   * Imports the given zTEX-Texture and saves them into the cache-folder. The imported texture
   * will also be added to the default resource manifest.
   *
   * See also ImportTexture().
   *
   * @param virtualFilePath  Path to .TGA or .TEX file in the VDFS.
   * @param vdfs             Virtual Filesystem to load the texture from.
   *
   * @return BsTexture containing the Data from the Gothic zTEX. Empty handle if importing failed.
   */
  bs::HTexture ImportAndCacheTexture(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs);
  bs::HTexture LoadCachedTexture(const bs::String& virtualFilePath);
  bool HasCachedTexture(const bs::String& virtualFilePath);
}
