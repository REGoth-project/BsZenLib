#include "ImportMaterial.hpp"
#include "ImportTexture.hpp"
#include "ImportPath.hpp"
#include <zenload/zTypes.h>
#include <FileSystem/BsFileSystem.h>
#include <Resources/BsBuiltinResources.h>
#include <Resources/BsResources.h>
#include <Material/BsMaterial.h>
#include <Material/BsShader.h>
#include <Importer/BsImporter.h>

using namespace bs;

static HTexture loadOrCacheTexture(const String& virtualFilePath, const VDFS::FileIndex& vdfs);

// - Implementation --------------------------------------------------------------------------------

static HShader loadWorldShader()
{
  String shaderName = "World.bsl";
  Path cachedShader = BsZenLib::GothicPathToCachedMaterial(shaderName);

  if(FileSystem::isFile(cachedShader))
    return gResources().load<Shader>(cachedShader);

  HShader shader = gImporter().import<Shader>("data/shader/" + shaderName);

  // Save to cache
  const bool overwrite = false;
  gResources().save(shader, BsZenLib::GothicPathToCachedMaterial(shader->getName()), overwrite);

  return shader;
}

bs::String BsZenLib::BuildMaterialNameForSubmesh(const bs::String& meshName, UINT32 submesh)
{
  return meshName + "-material-" + toString(submesh);
}

HMaterial BsZenLib::ImportAndCacheMaterialWithTextures(const String& cacheName,
                                                       const ZenLoad::zCMaterialData& material,
                                                       const VDFS::FileIndex& vdfs)
{
  HShader shader = gBuiltinResources().getBuiltinShader(BuiltinShader::Standard);
  //Commented out: Doesn't work yet. -- HShader shader = loadWorldShader();

  HMaterial bsfMaterial = Material::create(shader);

  // Load Textures
  HTexture albedo = loadOrCacheTexture(material.texture.c_str(), vdfs);
  bsfMaterial->setTexture("gAlbedoTex", albedo);

  // Save to cache
  const bool overwrite = false;
  gResources().save(bsfMaterial, GothicPathToCachedMaterial(cacheName), overwrite);

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

HMaterial BsZenLib::LoadCachedMaterial(const String& cacheName)
{
  return gResources().load<Material>(GothicPathToCachedMaterial(cacheName));
}

bool BsZenLib::HasCachedMaterial(const String& cacheName)
{
  return FileSystem::isFile(GothicPathToCachedMaterial(cacheName));
}