/** \file
 * Import and Cache Textures (.TGA, .TEX)
 */

#pragma once
#include <string>
#include <Image/BsTexture.h>

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
   * While the creators of Gothic worked mostly with TGA-files, the engine converted/compressed them
   * automatically to a DDS like format, called zTEX. Given an input file `IMAGE.TGA`, the engine
   * would produce a file called `IMAGE-C.TEX`, called the *compiled* file.
   *
   * This function will interpret these .TEX-files and load them into a BsTexture.
   *
   * @note  Given a .TGA-file as input, this function will automatically look for the correct
   * compiled .TEX-file.
   *
   * @param originalFileName  Name of the texture in the original games files (ie. "STONE.TGA")
   * @param vdfs              Virtual Filesystem to load the texture from.
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
   * @param originalFileName Name of the texture in the original games files (ie. "STONE.TGA")
   * @param vdfs             Virtual Filesystem to load the texture from.
   *
   * @return BsTexture containing the Data from the Gothic zTEX. Empty handle if importing failed.
   */
  bs::HTexture ImportAndCacheTexture(const bs::String& originalFileName,
                                     const VDFS::FileIndex& vdfs);

  /**
   * Load a cached Texture from disk using the original texture name.
   *
   * Given the name of the texture used in the original game files (ie. "STONE.TGA"), this
   * function will search for a cached version representing that texture.
   *
   * If unsure whether the cached file exists, call HasCachedTexture().
   * To create the cache, call ImportAndCacheTexture() first.
   *
   * @param originalFileName  Name of the texture in the original games files (ie. "STONE.TGA")
   * @return Handle to the cached texture (Empty handle if none was found)
   */
  bs::HTexture LoadCachedTexture(const bs::String& originalFileName);

  /**
   * Checks whether the cache for the given original texture name exists.
   *
   * To create the cache, call ImportAndCacheTexture().
   *
   * @param originalFileName  Name of the texture in the original games files (ie. "STONE.TGA")
   * @return Whether the cache exists for the given texture.
   */
  bool HasCachedTexture(const bs::String& originalFileName);
}  // namespace BsZenLib
