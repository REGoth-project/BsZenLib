/** \file
 * Import and cache animations.
 */

#pragma once
#include <BsCorePrerequisites.h>
#include <Animation/BsAnimation.h>
#include <Animation/BsAnimationClip.h>
#include <BsZenLib/ZenResources.hpp>
#include <zenload/modelScriptParser.h>

namespace ZenLoad
{
  struct zCMaterialData;
  class zCModelMeshLib;
}

namespace VDFS
{
  class FileIndex;
}

namespace BsZenLib
{
  struct AnimationToImport
  {
    bs::String fullAnimationName;

    ZenLoad::zCModelScriptAni animation;

    bs::Vector<ZenLoad::zCModelScriptEventSfx> eventsSfx;
    bs::Vector<ZenLoad::zCModelScriptEventSfx> eventsSfxGround;

    bs::Vector<ZenLoad::zCModelScriptEventPfx> eventsPfx;
    bs::Vector<ZenLoad::zCModelScriptEventPfxStop> eventsPfxStop;

    bs::Vector<ZenLoad::zCModelScriptEventTag> eventsTag;
  };

  bs::HAnimationClip ImportMAN(const ZenLoad::zCModelMeshLib& meshLib,
                             const AnimationToImport& def, const VDFS::FileIndex& vdfs);

  bs::HAnimation ImportAndCacheAnimation(const bs::String& virtualFilePath,
                                         const bs::String& modelMeshLibPath,
                                         const VDFS::FileIndex& vdfs);
}  // namespace BsZenLib
