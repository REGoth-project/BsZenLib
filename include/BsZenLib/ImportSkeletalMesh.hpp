#pragma once
#include <Material/BsMaterial.h>
#include <Mesh/BsMesh.h>
#include <Animation/BsSkeleton.h>
#include "ZenResources.hpp"
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
   * @param file .MDS-file to import. This function will try to find a matching .MDS-file
   *             and load that instead if it exists.
   * @param vdfs VDFS to load from.
   * 
   * @return Everything loaded from the given ModelScript-File.
   */
  Res::HModelScriptFile ImportAndCacheMDS(const bs::String& mdsFile, const VDFS::FileIndex& vdfs);

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
