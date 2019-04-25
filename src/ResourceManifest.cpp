#include "ResourceManifest.hpp"
#include <Resources/BsResource.h>
#include "ImportPath.hpp"
#include <FileSystem/BsFileSystem.h>
#include <Resources/BsResourceManifest.h>
#include <Resources/BsResources.h>

constexpr auto GOTHIC_CACHE_MANIFEST_NAME = "gothic-cache";

static bs::SPtr<bs::ResourceManifest> s_GothicCache;

namespace BsZenLib
{
  void LoadResourceManifest()
  {
    bs::Path manifestPath = BsZenLib::GothicPathToCachedManifest(GOTHIC_CACHE_MANIFEST_NAME);

    if (bs::FileSystem::exists(manifestPath))
    {
      s_GothicCache = bs::ResourceManifest::load(manifestPath, BsZenLib::GetCacheDirectory());

      bs::gResources().registerResourceManifest(s_GothicCache);
    }
    else
    {
      s_GothicCache = bs::ResourceManifest::create(GOTHIC_CACHE_MANIFEST_NAME);
    }
  }

  void AddToResourceManifest(bs::HResource resource, const bs::Path& filePath)
  {
    if (!s_GothicCache)
    {
      LoadResourceManifest();
    }

    s_GothicCache->registerResource(resource.getUUID(), filePath);
  }

  void SaveResourceManifest()
  {
    if (!s_GothicCache)
    {
      LoadResourceManifest();
    }

    bs::Path manifestPath = BsZenLib::GothicPathToCachedManifest(GOTHIC_CACHE_MANIFEST_NAME);
    bs::ResourceManifest::save(s_GothicCache, manifestPath, BsZenLib::GetCacheDirectory());
  }

}  // namespace BsZenLib
