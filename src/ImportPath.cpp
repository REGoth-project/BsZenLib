#include "ImportPath.hpp"
#include <FileSystem/BsFileSystem.h>

using namespace bs;

static const char* CACHE_DIRECTORY = "Cache";

bs::Path BsZenLib::GothicPathToCachedAsset(const String& virtualFilePath)
{
	return GetCacheDirectory() + Path(virtualFilePath + ".asset");
}

bs::Path BsZenLib::GothicPathToCachedManifest(const String& virtualFilePath)
{
	return GetCacheDirectory() + Path(virtualFilePath + ".manifest");
}

bs::Path BsZenLib::GothicPathToCachedWorld(const bs::String& worldName)
{
	return GetCacheDirectory() + Path("worlds") + Path(worldName + ".asset");
}

bs::Path BsZenLib::GetCacheDirectory()
{
	return Path(CACHE_DIRECTORY).makeAbsolute(FileSystem::getWorkingDirectoryPath());
}
