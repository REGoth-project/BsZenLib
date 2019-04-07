/** \file
 * Import and cache fonts.
 */

#pragma once
#include <BsCorePrerequisites.h>

namespace ZenLoad
{
  struct zCMaterialData;
}

namespace VDFS
{
  class FileIndex;
}

namespace BsZenLib
{
  /**
   * Whether the given font has been cached before.
   *
   * See LoadCachedFont() to actually load it.
   * See ImportAndCacheFont() to generate the cache.
   *
   * @param  originalFileName  Name the font file has in the original game, like `FONT_DEFAULT.FNT`.
   *
   * @return Whether the cache for the given font exists.
   */
  bool HasCachedFont(const bs::String& originalFileName);

  /**
   * Load a cached Font.
   *
   * To generate the cached font file, see ImportAndCacheFont().
   *
   * @param cacheName. See ImportAndCacheFont().
   *
   * @return Cached bs::f font.
   */
  bs::HFont LoadCachedFont(const bs::String& originalFileName);

  /**
   * Will import (and cache) a Font from the original game along with all textures it is
   * depending on.
   *
   * @param originalFileName  Name the cached font should get.
   * @param font              Font-data pulled from the original game files.
   * @param vdfs              FileIndex to load the textures from.
   *
   * @return bs::f font configured to match the original games font passed in.
   */
  bs::HFont ImportAndCacheFont(const bs::String& originalFileName, const VDFS::FileIndex& vdfs);
}
