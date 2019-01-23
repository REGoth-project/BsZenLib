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
   * @param cacheName  Name the cached material should get.
   * @param material   Material-data pulled from the original game files.
   * @param vdfs       FileIndex to load the textures from.
   *
   * @return bs::f material configured to match the original games material passed in.
   */
  bs::HMaterial ImportAndCacheMaterialWithTextures(const bs::String& cacheName,
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

  /**
   * Combines the submesh-index with the meshes name to make a unique material name.
   * 
   * Since in bs::f, every Material needs the be in its own file, but Gothic has
   * Materials by Mesh, we store each material as "meshname-material-i" or something
   * similar.
   * 
   * This function builds the actual name of the cached materials name from a meshname
   * and the submesh index.
   */
  bs::String BuildMaterialNameForSubmesh(const bs::String& cacheName, bs::UINT32 submesh);
}  // namespace BsZenLib
