#pragma once
#include <Material/BsMaterial.h>
#include <Mesh/BsMesh.h>

namespace ZenLoad
{
  class zCMesh;
}

namespace VDFS
{
  class FileIndex;
}

namespace BsZenLib
{
  /**
   * Import a Gothic zCMesh into bs:f.
   *
   * @param zenMesh  Mesh to grab the data from. Can be constructed from a ZenParser.
   *
   * @return bs:f-Mesh containing the Data from the Gothic mesh.
   */
  bs::HMesh ImportMeshFromZEN(/* const */ ZenLoad::zCMesh& zenMesh);

  /**
   * Generate bs:f materials for the materials stored inside the given zCMesh.
   *
   * @param zenMesh The Mesh to grab the list of materials from. Can be constructed from a ZenParser.
   * @param vdfs  Virtual Filesystem to load the textures and other resources from.
   *
   * @return List of bs:f materials matching the materials in the Gothic mesh.
   **/
  bs::Vector<bs::HMaterial> ImportMaterialsFromZENMesh(/* const */ ZenLoad::zCMesh& zenMesh,
                                                      const VDFS::FileIndex& vdfs);
}  // namespace BsZenLib
