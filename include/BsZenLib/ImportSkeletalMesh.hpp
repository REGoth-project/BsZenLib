/** \file
 * Import and cache animated meshes.
 */

#pragma once
#include "ZenResources.hpp"
#include <Animation/BsSkeleton.h>
#include <Material/BsMaterial.h>
#include <Mesh/BsMesh.h>
#include <zenload/zCModelMeshLib.h>

namespace ZenLoad
{
  struct PackedSkeletalMesh;
}  // namespace ZenLoad

namespace VDFS
{
  class FileIndex;
}

namespace BsZenLib
{
  /**
   * Imports a model script file.
   *
   * A ModelScriptFile can come in two different variations: ASCII and Binary.
   * While Gothic I uses only ASCII files, most if not all of those got coverted into a
   * binary format for Gothic II.
   *
   * Both variants contain the same information:
   *
   *  - Which node hierarchy (bones) file to use
   *  - A list of supported meshes
   *  - Definitions for each animation to be played
   *
   * @note  This function will also import .MDL files and put them into simple Model Scripts.
   *
   * @param file .MDS-file to import. This function will try to find a matching .MDS-file
   *             and load that instead if it exists.
   * @param vdfs VDFS to load from.
   *
   * @return Everything loaded from the given ModelScript-File.
   */
  Res::HModelScriptFile ImportAndCacheMDS(const bs::String& mdsFile, const VDFS::FileIndex& vdfs);

  /**
   * Loads the visuals attached to the bones by default of the given model file.
   *
   * @param mdlFile  .MDL-File to look up the attachments from
   * @param vdfs     VDFS to load the .MDL-file from
   *
   * @return Map of Node-Name -> Visual-File.
   */
  bs::Map<bs::String, Res::HMeshWithMaterials> ImportAndCacheNodeAttachments(
      const bs::String& mdlFile, const VDFS::FileIndex& vdfs);

  /**
   * Whether the given .MDS-file has been cached.
   *
   * @param mdsfile .MDS-file to look after. Always supply the .MDS-file, even when
   *                a matching .MSB-file exists!
   *
   * @return Whether the .MDS-file has been cached.
   */
  bool HasCachedMDS(const bs::String& mdsFile);

  /**
   * Loads a .MDS-file from cache.
   *
   * @param mdsfile .MDS-file to look after. Always supply the .MDS-file, even when
   *                a matching .MSB-file exists!
   *
   * @return The cached .MDS-file.
   */
  Res::HModelScriptFile LoadCachedMDS(const bs::String& mdsFile);

}  // namespace BsZenLib
