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
  bs::HMesh ImportSkeletalMesh(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs);
  bs::HMesh LoadCachedSkeletalMesh(const bs::String& virtualFilePath);
  bs::Vector<bs::Matrix4> getBindPose(const std::string& virtualFilePath,
                                      const VDFS::FileIndex& vdfs);

  bs::HSceneObject ImportSkeletalMeshWithMaterials(const bs::String& virtualFilePath,
                                                   const VDFS::FileIndex& vdfs);
  bs::Vector<bs::HMaterial> ImportMaterialsFromSkeletalMesh(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs);

  bs::HMesh ImportAndCacheSkeletalMesh(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs);
  bs::Vector<bs::HMaterial> ImportAndCacheSkeletalMeshMaterials(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs);

  bool HasCachedSkeletalMeshPrefab(const bs::String& virtualFilePath);
  bs::HPrefab LoadCachedSkeletalMeshPrefab(const bs::String& virtualFilePath);
  bs::HPrefab ImportAndCacheSkeletalMeshPrefab(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs);
}  // namespace BsZenLib
