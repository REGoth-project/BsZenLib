#pragma once
#include <Scene/BsPrefab.h>

namespace VDFS
{
  class FileIndex;
}

namespace BsZenLib
{
  /**
   * Import a Gothic ZEN-World into bs:f.
   *
   * @param zen ZEN-File to load.
   *
   * @return bs:f-Mesh containing the Data from the Gothic mesh.
   */
  bs::HSceneObject ImportZEN(const std::string& zen, const VDFS::FileIndex& vdfs);
}  // namespace BsZenLib
