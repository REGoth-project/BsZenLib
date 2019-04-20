#include "ImportMaterial.hpp"
#include "ImportPath.hpp"
#include "ImportTexture.hpp"
#include "ResourceManifest.hpp"
#include <FileSystem/BsFileSystem.h>
#include <Importer/BsImporter.h>
#include <Material/BsMaterial.h>
#include <Material/BsShader.h>
#include <Resources/BsBuiltinResources.h>
#include <Resources/BsResources.h>
#include <zenload/zTypes.h>

using namespace bs;

/**
 * Stores shaders to be used if the importer encouters a specific kind of material.
 *
 * See BsZenLib::SetShaderFor().
 */
bs::Map<BsZenLib::ShaderKind, bs::HShader> s_ShadersByKind;

static HTexture loadOrCacheTexture(const String& virtualFilePath, const VDFS::FileIndex& vdfs);
static BsZenLib::ShaderKind guessShaderKindFromZMaterial(const ZenLoad::zCMaterialData& mat);

// - Implementation --------------------------------------------------------------------------------

static HShader getShaderForZMaterial(const ZenLoad::zCMaterialData& mat)
{
  using namespace BsZenLib;

  ShaderKind kind = guessShaderKindFromZMaterial(mat);

  auto it = s_ShadersByKind.find(kind);

  if (it == s_ShadersByKind.end())
  {
    return gBuiltinResources().getBuiltinShader(BuiltinShader::Standard);
  }
  else
  {
    return it->second;
  }
}

static BsZenLib::ShaderKind guessShaderKindFromZMaterial(const ZenLoad::zCMaterialData& mat)
{
  // TODO: Implement
  return BsZenLib::ShaderKind::Opaque;
}

static HShader loadWorldShader()
{
  String shaderName = "World.bsl";
  Path cachedShader = BsZenLib::GothicPathToCachedMaterial(shaderName);

  if (FileSystem::isFile(cachedShader)) return gResources().load<Shader>(cachedShader);

  HShader shader = gImporter().import<Shader>("content/shaders/" + shaderName);

  // Save to cache
  const bool overwrite = true;
  gResources().save(shader, BsZenLib::GothicPathToCachedMaterial(shaderName), overwrite);
  BsZenLib::AddToResourceManifest(shader, BsZenLib::GothicPathToCachedMaterial(shaderName));

  return shader;
}

void BsZenLib::SetShaderFor(ShaderKind kind, bs::HShader shader)
{
  //
  s_ShadersByKind[kind] = shader;
}

bs::String BsZenLib::BuildMaterialNameForSubmesh(const bs::String& meshName, UINT32 submesh)
{
  return meshName + "-material-" + toString(submesh);
}

HMaterial BsZenLib::ImportAndCacheMaterialWithTextures(const String& cacheName,
                                                       const ZenLoad::zCMaterialData& material,
                                                       const VDFS::FileIndex& vdfs)
{
  HShader shader = getShaderForZMaterial(material);

  HMaterial bsfMaterial = Material::create(shader);

  // Load Textures
  HTexture albedo = loadOrCacheTexture(material.texture.c_str(), vdfs);
  bsfMaterial->setTexture("gAlbedoTex", albedo);

  // Save to cache
  const bool overwrite = true;
  gResources().save(bsfMaterial, GothicPathToCachedMaterial(cacheName), overwrite);
  AddToResourceManifest(bsfMaterial, GothicPathToCachedMaterial(cacheName));

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
