#pragma once
#include <BsPrerequisites.h>

namespace BsZenLib
{
  /**
   * Loads a resource manifest written by a previous cache operation which
   * lists all cached original gothic assets.
   *
   * If none exists, nothing happens.
   *
   * For information about resource manifests, see:
   * https://www.bsframework.io/docs/saving_scene.html
   */
  void LoadResourceManifest();

  /**
   * Adds the given resource to the resource manifest used for caching the
   * original gothic assets.
   *
   * This will also save the manifest automatically to circumvent issues
   * when a crash happens during caching time.
   *
   * @note This is not threadsave!
   */
  void AddToResourceManifest(bs::HResource resource, const bs::Path& filePath);

  /**
   * Saves the resource manifest to disk.
   *
   * If no resource manifest has been loaded, yet, an empty one will be created
   * and saved. Be careful as this will replace the existing manfest should none exist!
   *
   * Also note that saving a manifest is rather slow so don't call this too often.
   */
  void SaveResourceManifest();

}  // namespace BsZenLib
