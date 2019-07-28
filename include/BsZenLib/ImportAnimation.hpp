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
  struct EventsToImport
  {
    bs::Vector<ZenLoad::zCModelScriptEventSfx> sfx;
    bs::Vector<ZenLoad::zCModelScriptEventSfx> sfxGround;

    bs::Vector<ZenLoad::zCModelScriptEventPfx> pfx;
    bs::Vector<ZenLoad::zCModelScriptEventPfxStop> pfxStop;

    bs::Vector<ZenLoad::zCModelScriptEventTag> tag;
    bs::Vector<ZenLoad::zCModelScriptEventMMStartAni> mmStartAni;
  };

  struct AnimationToImport
  {
    bs::String fullAnimationName;
    bs::String manFileName;

    ZenLoad::zCModelScriptAni animation;
    EventsToImport events;
  };

  struct AnimationToAlias
  {
    bs::String fullAnimationName;
    bs::String fullAnimationNameOfAlias;

    ZenLoad::zCModelScriptAniAlias animation;
    ZenLoad::zCModelScriptAni animationSource;
  };

  struct AnimationToBlend
  {
    bs::String fullAnimationName;
    bs::String fullAnimationNameOfBlend;

    ZenLoad::zCModelScriptAniBlend animation;
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
   * Alias a single animation clip and give it a different name and properties.
   *
   * The animation to alias has to have been cached before.
   */
  Res::HZAnimation AliasAnimation(const ZenLoad::zCModelMeshLib& meshLib,
                                  const AnimationToAlias& def, const VDFS::FileIndex& vdfs);

  /**
   * Define a blend to the given animation.
   *
   * The animation to blend has to have been cached before.
   */
  Res::HZAnimation BlendAnimation(const AnimationToBlend& def);

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
