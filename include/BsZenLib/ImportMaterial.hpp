/** \file
 * Import and cache materials.
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
   * Will import (and cache) a Material from the original game along with all textures it is
   * depending on.
   *
   * @param meshName   Name of the Mesh this material was retrieved from. Needs to be provided since
   *                   materials don't have a name in the original engine.
   * @param material   Material-data pulled from the original game files.
   * @param vdfs       FileIndex to load the textures from.
   *
   * @return bs::f material configured to match the original games material passed in.
   */
  bs::HMaterial ImportAndCacheMaterialWithTextures(const bs::String& meshName,
                                                   const ZenLoad::zCMaterialData& material,
                                                   const VDFS::FileIndex& vdfs);

  /**
   * Load a cached Material.
   *
   * To generate the cached material file, see ImportAndCacheMaterialWithTextures().
   *
   * @param cacheName. See ImportAndCacheMaterialWithTextures().
   *
   * @return Cached bs::f material.
   */
  bs::HMaterial LoadCachedMaterial(const bs::String& cacheName);

  /**
   * Whether the given material has been cached before.
   *
   * See LoadCachedMaterial() to actually load it.
   * See ImportAndCacheMaterialWithTextures() to generate the cache.
   *
   * @param cacheName. See ImportAndCacheMaterialWithTextures().
   *
   * @return Whether the cache for the given material exists.
   */
  bool HasCachedMaterial(const bs::String& cacheName);
}  // namespace BsZenLib
