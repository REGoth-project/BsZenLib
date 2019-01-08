#include "ImportMaterial.hpp"
#include "ImportTexture.hpp"
#include "ImportPath.hpp"
#include <zenload/zTypes.h>
#include <FileSystem/BsFileSystem.h>
#include <Resources/BsBuiltinResources.h>
#include <Resources/BsResources.h>
#include <Material/BsMaterial.h>

using namespace bs;

static HTexture loadOrCacheTexture(const String& virtualFilePath, const VDFS::FileIndex& vdfs);

// - Implementation --------------------------------------------------------------------------------

HMaterial BsZenLib::ImportAndCacheMaterialWithTextures(const String& meshName,
                                                       const ZenLoad::zCMaterialData& material,
                                                       const VDFS::FileIndex& vdfs)
{
  HShader shader = gBuiltinResources().getBuiltinShader(BuiltinShader::Standard);

  HMaterial bsfMaterial = Material::create(shader);

  // Load Textures
  HTexture albedo = loadOrCacheTexture(material.texture.c_str(), vdfs);
  bsfMaterial->setTexture("gAlbedoTex", albedo);

  // Save to cache
  const bool overwrite = false;
  gResources().save(bsfMaterial, GothicPathToCachedMaterial(meshName), overwrite);

  return bsfMaterial;
}

static HTexture loadOrCacheTexture(const String& virtualFilePath, const VDFS::FileIndex& vdfs)
{
  if (BsZenLib::HasCachedTexture(virtualFilePath))
  {
    return BsZenLib::LoadCachedTexture(virtualFilePath);
  }
  else
  {
    return BsZenLib::ImportAndCacheTexture(virtualFilePath, vdfs);
  }
}

HMaterial BsZenLib::LoadCachedMaterial(const String& meshName)
{
  return gResources().load<Material>(GothicPathToCachedMaterial(meshName));
}

bool BsZenLib::HasCachedMaterial(const String& meshName)
{
  return FileSystem::isFile(GothicPathToCachedMaterial(meshName));
}