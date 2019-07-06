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
}  // namespace ZenLoad

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

  /**
   * Import a single animation clip.
   *
   * Converts the animation samples in the given .MAN-File for bs::f and creates an animation clip
   * out of it.
   *
   * bs::f does support animation events, but the only data you can give to it are a string and a
   * time. For that reason, a simple text based command format is used. Commands are structured like
   * `command:action`. For example, `PLAYANIM:S_RUNL` will play an animation called `S_RUNL`.
   */
  Res::HZAnimation ImportMAN(const ZenLoad::zCModelMeshLib& meshLib, const AnimationToImport& def,
                             const VDFS::FileIndex& vdfs);

  /**
   * Checks whether the given Animation clip has been cached.
   *
   * @param  fullAnimationName  `fullAnimationName`-field from AnimationToImport-structure.
   *
   * @return Whether the Animation has been cached.
   */
  bool HasCachedMAN(const bs::String& fullAnimationName);

  /**
   * Loads the animation with the given name from cache.
   *
   * @param  fullAnimationName  `fullAnimationName`-field from AnimationToImport-structure.
   *
   * @return Hande to the cached animiation clip.
   */
  Res::HZAnimation LoadCachedAnimation(const bs::String& fullAnimationName);
}  // namespace BsZenLib
