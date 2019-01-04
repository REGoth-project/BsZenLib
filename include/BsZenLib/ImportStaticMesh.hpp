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
	bs::HPrefab LoadCachedStaticMeshPrefab(const bs::String& virtualFilePath);
	bool HasCachedStaticMeshPrefab(const bs::String& virtualFilePath);

	bs::HPrefab ImportAndCacheStaticMeshPrefab(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs);
	bs::HPrefab ImportAndCacheStaticMeshPrefab(const bs::String& virtualFilePath, const ZenLoad::PackedMesh& packedMesh, const VDFS::FileIndex& vdfs);

	bs::HMesh LoadCachedStaticMesh(const bs::String& virtualFilePath);
	bs::HMesh ImportAndCacheStaticMesh(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs);
	bs::HMesh ImportAndCacheStaticMesh(const bs::String& virtualFilePath, const ZenLoad::PackedMesh& packedMesh, const VDFS::FileIndex& vdfs);

	bs::Vector<bs::HMaterial> ImportAndCacheStaticMeshMaterials(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs);
	bs::Vector<bs::HMaterial> ImportAndCacheStaticMeshMaterials(const bs::String& virtualFilePath, const ZenLoad::PackedMesh& packedMesh, const VDFS::FileIndex& vdfs);

  bs::HMesh ImportStaticMesh(const ZenLoad::PackedMesh& packedMesh);
  bs::HSceneObject ImportStaticMeshWithMaterials(const std::string& name,
                                                 const ZenLoad::PackedMesh& packedMesh,
                                                 const VDFS::FileIndex& vdfs);
}  // namespace BsZenLib
