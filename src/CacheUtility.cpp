#include "CacheUtility.hpp"
#include "ImportSkeletalMesh.hpp"
#include "ImportStaticMesh.hpp"
#include "ImportTexture.hpp"
#include <Threading/BsTaskScheduler.h>
#include <vdfs/fileIndex.h>

using namespace BsZenLib;
using namespace bs;

static bool hasExtension(const bs::String& file, const bs::String& ext)
{
  return file.find(ext) != bs::String::npos;
}

static void cacheAll(const VDFS::FileIndex& vdfs, const bs::String& ext,
                     const std::function<void(const bs::String&)>& fn)
{
  bs::Vector<bs::SPtr<bs::Task>> tasks;

  for (std::string file_ : vdfs.getKnownFiles())
  {
    bs::String file = file_.c_str();
    bs::StringUtil::toUpperCase(file);

    if (hasExtension(file, ext))
    {
      tasks.push_back(bs::Task::create(file.c_str(), [fn, file]() { fn(file); }));
    }
  }

  for (auto& task : tasks)
  {
    bs::TaskScheduler::instance().addTask(task);
  }

  for (auto& task : tasks)
  {
    task->wait();
  }
}

void BsZenLib::CacheWholeVDFS(const VDFS::FileIndex& vdfs)
{
  auto textures = [&](const bs::String& file) {
    bs::String uncompiled = bs::StringUtil::replaceAll(file, "-C.TEX", ".TGA");

    if (!HasCachedTexture(uncompiled))
    {
      ImportAndCacheTexture(uncompiled, vdfs);
    }
  };

  auto staticMeshes = [&](const bs::String& file) {
    bs::String uncompiled = bs::StringUtil::replaceAll(file, ".MRM", ".3DS");

    if (!HasCachedStaticMesh(uncompiled))
    {
      ImportAndCacheStaticMesh(uncompiled, vdfs);
    }
  };

  auto skeletalMeshes = [&](const bs::String& file) {
    // MDS is not compiled, keep extension

    if (!HasCachedMDS(file))
    {
      ImportAndCacheMDS(file, vdfs);
    }
  };

  cacheAll(vdfs, ".TEX", textures);
  cacheAll(vdfs, ".MRM", staticMeshes);

  // FIXME: There seems to be a problem with SHEEP in Gothic 2, therefore MDS is commented out
  // cacheAll(vdfs, ".MDS", skeletalMeshes);
}
