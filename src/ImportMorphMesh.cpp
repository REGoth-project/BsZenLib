#include "ImportMorphMesh.hpp"
#include "ImportStaticMesh.hpp"
#include <zenload/zCMorphMesh.h>

using namespace bs;
using namespace BsZenLib;

// - Implementation --------------------------------------------------------------------------------

Res::HMeshWithMaterials BsZenLib::ImportAndCacheMorphMesh(const bs::String& originalFileName,
                                                          const VDFS::FileIndex& vdfs)
{
  bs::String actualName = originalFileName;

  enum
  {
    CompareCaseSensitive = false,
    ConvertToLowerCase = true,
  };

  // Resolve uncompiled file extension
  if (bs::StringUtil::endsWith(originalFileName, ".mms", ConvertToLowerCase))
  {
    actualName = originalFileName.substr(0, originalFileName.size() - 4) + ".MMB";
  }

  ZenLoad::zCMorphMesh mesh(originalFileName.c_str(), vdfs);

  if (mesh.getMesh().getNumSubmeshes() == 0) return {};

  ZenLoad::PackedMesh packedMesh;
  mesh.getMesh().packMesh(packedMesh, 0.01f);

  return ImportAndCacheStaticMesh(originalFileName, packedMesh, vdfs);
}

Res::HMeshWithMaterials BsZenLib::LoadCachedMorphMesh(const bs::String& originalFileName)
{
  return LoadCachedStaticMesh(originalFileName);
}

bool BsZenLib::HasCachedMorphMesh(const bs::String& originalFileName)
{
  return HasCachedStaticMesh(originalFileName);
}
