#pragma once
#include <BsCorePrerequisites.h>

namespace BsZenLib
{
  /**
   * Convert a resource path in the original data to a cache-path.
   */
  bs::Path GothicPathToCachedAsset(const bs::String& virtualFilePath);
  bs::Path GothicPathToCachedTexture(const bs::String& virtualFilePath);
  bs::Path GothicPathToCachedMaterial(const bs::String& virtualFilePath);
  bs::Path GothicPathToCachedStaticMesh(const bs::String& virtualFilePath);
  bs::Path GothicPathToCachedSkeletalMesh(const bs::String& virtualFilePath);
  bs::Path GothicPathToCachedAnimationClip(const bs::String& virtualFilePath);
  bs::Path GothicPathToCachedModelScript(const bs::String& virtualFilePath);
  bs::Path GothicPathToCachedZAnimation(const bs::String& virtualFilePath);
  bs::Path GothicPathToCachedManifest(const bs::String& virtualFilePath);
  bs::Path GothicPathToCachedShader(const bs::String& shaderName);
  bs::Path GothicPathToCachedWorld(const bs::String& worldName);
  bs::Path GothicPathToCachedFont(const bs::String& virtualFilePath);
  bs::Path GetCacheDirectory();
}  // namespace BsZenLib
