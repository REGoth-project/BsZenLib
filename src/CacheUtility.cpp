#include "CacheUtility.hpp"
#include "ImportSkeletalMesh.hpp"
#include "ImportStaticMesh.hpp"
#include "ImportTexture.hpp"
#include <vdfs/fileIndex.h>

using namespace BsZenLib;
using namespace bs;

static bool hasExtension(const std::string& file, const std::string& ext)
{
  return file.find(ext) != std::string::npos;
}

void BsZenLib::CacheWholeVDFS(const VDFS::FileIndex& vdfs)
{
  for (std::string file : vdfs.getKnownFiles())
  {
    if (hasExtension(file, ".TEX"))
    {
      ImportAndCacheTexture(String(file.c_str()), vdfs);
    }
    else if (hasExtension(file, ".MRM"))
    {
      ImportAndCacheStaticMesh(String(file.c_str()), vdfs);
    }
    else if (hasExtension(file, ".MSB") || hasExtension(file, ".MDS"))
    {
      ImportAndCacheMDS(String(file.c_str()), vdfs);
    }
  }
}
