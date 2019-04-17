#include "ImportMaterial.hpp"
#include "ImportPath.hpp"
#include "ImportTexture.hpp"
#include <FileSystem/BsFileSystem.h>
#include <Importer/BsImporter.h>
#include <Material/BsMaterial.h>
#include <Material/BsShader.h>
#include <Resources/BsBuiltinResources.h>
#include <Resources/BsResources.h>
#include <zenload/zTypes.h>

using namespace bs;

static HTexture loadOrCacheTexture(const String& virtualFilePath, const VDFS::FileIndex& vdfs);

// - Implementation --------------------------------------------------------------------------------

static HShader loadWorldShader()
{
  String shaderName = "World.bsl";
  Path cachedShader = BsZenLib::GothicPathToCachedMaterial(shaderName);

  if (FileSystem::isFile(cachedShader)) return gResources().load<Shader>(cachedShader);

  HShader shader = gImporter().import<Shader>("content/shaders/" + shaderName);

  // Save to cache
  const bool overwrite = true;
  gResources().save(shader, BsZenLib::GothicPathToCachedMaterial(shaderName), overwrite);

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
  // Commented out: May want to fall back to the default shader later
  HShader shader = gBuiltinResources().getBuiltinShader(BuiltinShader::Standard);
  // Commented out: Doesn't work yet. -- HShader shader = loadWorldShader();

  HMaterial bsfMaterial = Material::create(shader);

  // Load Textures
  HTexture albedo = loadOrCacheTexture(material.texture.c_str(), vdfs);
  bsfMaterial->setTexture("gAlbedoTex", albedo);

  // Save to cache
  const bool overwrite = true;
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
