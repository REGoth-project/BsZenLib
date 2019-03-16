/** \file
 * \rst
 * Import and cache Morph-Meshes (MMB)
 * ===================================
 *
 * Morph-Meshes are triangular 3d models which can have animations on each vertex.
 * They are used for mouth-movement animations on talking characters or flags moving
 * in the wind.
 *
 * In Gothic, all human characters are using morph meshes as heads.
 *
 * .. note::
 *
 *    Morph-meshes are not completely supported by ZenLib, so only a static representation
 *    will be imported.
 *
 *
 * The following example will load and cache a morph-mesh:
 *
 * .. code-block:: cpp
 *
 *    Res::HMeshWithMaterials head = BsZenLib::ImportAndCacheMorphMesh("HEAD.MMB", vdfs);
 *
 * Where ``vdfs`` is a VDFS FileIndex containing ``HEAD.MMB``.
 *
 * \endrst
 */

#pragma once
#include <string>
#include "ZenResources.hpp"

namespace VDFS
{
  class FileIndex;
}

namespace BsZenLib
{
  /**
   * Import a Gothic Morph-Mesh and save the results to disk.
   *
   *
   * Imports the given Morph-Mesh and saves them into the cache-folder. The imported Morph-Mesh
   * will also be added to the default resource manifest.
   *
   * @param originalFileName Name of the MorphMesh in the original games files (ie. `HEAD.MMB`)
   * @param vdfs             Virtual Filesystem to load the MorphMesh from.
   *
   * @return Handle to the cached mesh (Empty handle if none was found)
   */
  Res::HMeshWithMaterials ImportAndCacheMorphMesh(const bs::String& originalFileName, const VDFS::FileIndex& vdfs);

  /**
   * Load a cached MorphMesh from disk using the original files name.
   *
   * Given the name of the MorphMesh used in the original game files (ie. "HEAD.MMB"), this
   * function will search for a cached version representing that MorphMesh.
   *
   * If unsure whether the cached file exists, call HasCachedMorphMesh().
   * To create the cache, call ImportAndCacheMorphMesh() first.
   *
   * @param originalFileName  Name of the MorphMesh in the original games files (ie. `HEAD.MMB`)
   *
   * @return Handle to the cached MorphMesh (Empty handle if none was found)
   */
  Res::HMeshWithMaterials LoadCachedMorphMesh(const bs::String& originalFileName);

  /**
   * Checks whether the cache for the given original MorphMesh name exists.
   *
   * To create the cache, call ImportAndCacheMorphMesh().
   *
   * @param originalFileName  Name of the MorphMesh in the original games files (ie. `HEAD.MMB`)
   *
   * @return Whether the cache exists for the given MorphMesh.
   */
  bool HasCachedMorphMesh(const bs::String& originalFileName);
}
