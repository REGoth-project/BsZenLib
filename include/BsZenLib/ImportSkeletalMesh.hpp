#pragma once
#include <Material/BsMaterial.h>
#include <Mesh/BsMesh.h>

namespace ZenLoad
{
  class PackedSkeletalMesh;
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
  bs::HMesh ImportSkeletalMesh(const bs::Vector<bs::Matrix4>& bindPose,
                               const ZenLoad::PackedSkeletalMesh& packedMesh);
  bs::HMesh ImportSkeletalMesh(const std::string& virtualFilePath, const VDFS::FileIndex& vdfs);
  bs::Vector<bs::Matrix4> getBindPose(const std::string& virtualFilePath,
                                      const VDFS::FileIndex& vdfs);

  bs::HSceneObject ImportSkeletalMeshWithMaterials(const std::string& virtualFilePath,
                                                   const VDFS::FileIndex& vdfs);
}  // namespace BsZenLib
