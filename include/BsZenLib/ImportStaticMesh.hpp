#pragma once
#include <Material/BsMaterial.h>
#include <Mesh/BsMesh.h>

namespace ZenLoad
{
  class PackedMesh;
  class zCMesh;
}  // namespace ZenLoad

namespace VDFS
{
  class FileIndex;
}

namespace BsZenLib
{
  /**
   * @return bs:f-Mesh containing the Data from the Gothic mesh.
   */
  bs::HMesh ImportStaticMesh(const ZenLoad::PackedMesh& packedMesh);
  bs::HSceneObject ImportStaticMeshWithMaterials(const std::string& name,
                                                 const ZenLoad::PackedMesh& packedMesh,
                                                 const VDFS::FileIndex& vdfs);
}  // namespace BsZenLib
