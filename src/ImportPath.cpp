#include "ImportPath.hpp"
#include <FileSystem/BsFileSystem.h>

using namespace bs;

static const char* CACHE_DIRECTORY = "cache";

bs::Path BsZenLib::GothicPathToCachedAsset(const String& virtualFilePath)
{
	return GetCacheDirectory() + Path(virtualFilePath + ".asset");
}

bs::Path BsZenLib::GothicPathToCachedTexture( const bs::String& virtualFilePath)
{
	return GetCacheDirectory() + Path("textures") + Path(virtualFilePath + ".asset");
}

bs::Path BsZenLib::GothicPathToCachedMaterial( const bs::String& virtualFilePath)
{
	return GetCacheDirectory() + Path("materials") + Path(virtualFilePath + ".asset");
}

bs::Path BsZenLib::GothicPathToCachedStaticMesh( const bs::String& virtualFilePath)
{
	return GetCacheDirectory() + Path("static-meshes") + Path(virtualFilePath + ".asset");
}

bs::Path BsZenLib::GothicPathToCachedSkeletalMesh( const bs::String& virtualFilePath)
{
	return GetCacheDirectory() + Path("skeletal-meshes") + Path(virtualFilePath + ".asset");
}

bs::Path BsZenLib::GothicPathToCachedAnimationClip( const bs::String& virtualFilePath)
{
	return GetCacheDirectory() + Path("animations") + Path(virtualFilePath + ".asset");
}

bs::Path BsZenLib::GothicPathToCachedModelScript( const bs::String& virtualFilePath)
{
	return GetCacheDirectory() + Path("model-scripts") + Path(virtualFilePath + ".asset");
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
