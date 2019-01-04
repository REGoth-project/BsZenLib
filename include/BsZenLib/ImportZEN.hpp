#pragma once
#include <Scene/BsPrefab.h>

namespace VDFS
{
  class FileIndex;
}

namespace BsZenLib
{
	bs::HPrefab LoadCachedZEN(const bs::String& zen);
	bool HasCachedZEN(const bs::String& zen);
	bs::HSceneObject ImportZEN(const std::string& zen, const VDFS::FileIndex& vdfs);
	bs::HPrefab ImportAndCacheZEN(const std::string& zen, const VDFS::FileIndex& vdfs);
}  // namespace BsZenLib
