#pragma once
#include <BsCorePrerequisites.h>

namespace VDFS
{
  class FileIndex;
}

namespace BsZenLib
{
  /**
   * Will try to cache every file loaded into the given VDFS.
   *
   * @param vdfs The VDFS to go through.
   */
  void CacheWholeVDFS(const VDFS::FileIndex& vdfs);

}  // namespace BsZenLib
