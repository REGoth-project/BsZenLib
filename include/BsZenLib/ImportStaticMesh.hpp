#pragma once
#include <Material/BsMaterial.h>
#include <Components/BsCRenderable.h>
#include <Mesh/BsMesh.h>
#include "ZenResources.hpp"

namespace ZenLoad
{
  struct PackedMesh;
  class zCMesh;
}  // namespace ZenLoad

namespace VDFS
{
  class FileIndex;
}

namespace BsZenLib
{
	bool HasCachedStaticMesh(const bs::String& virtualFilePath);
	Res::HMeshWithMaterials LoadCachedStaticMesh(const bs::String& virtualFilePath);
	Res::HMeshWithMaterials ImportAndCacheStaticMesh(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs);
	Res::HMeshWithMaterials ImportAndCacheStaticMesh(const bs::String& virtualFilePath, const ZenLoad::PackedMesh& packedMesh, const VDFS::FileIndex& vdfs);

	bs::HMesh ImportAndCacheStaticMeshGeometry(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs);
	bs::HMesh ImportAndCacheStaticMeshGeometry(const bs::String& virtualFilePath, const ZenLoad::PackedMesh& packedMesh, const VDFS::FileIndex& vdfs);

	bs::Vector<bs::HMaterial> ImportAndCacheStaticMeshMaterials(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs);
	bs::Vector<bs::HMaterial> ImportAndCacheStaticMeshMaterials(const bs::String& virtualFilePath, const ZenLoad::PackedMesh& packedMesh, const VDFS::FileIndex& vdfs);

  bs::HMesh ImportStaticMeshGeometry(const ZenLoad::PackedMesh& packedMesh);
}  // namespace BsZenLib
