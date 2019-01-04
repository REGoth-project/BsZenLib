#pragma once
#include <BsCorePrerequisites.h>

namespace BsZenLib
{
	/**
	 * Convert a resource path in the original data to a cache-path.
	 */
	bs::Path GothicPathToCachedAsset( const bs::String& virtualFilePath);
	bs::Path GothicPathToCachedManifest(const bs::String& virtualFilePath);
	bs::Path GothicPathToCachedWorld(const bs::String& worldName);
	bs::Path GetCacheDirectory();
}
