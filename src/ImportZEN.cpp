/**
 * Import ZEN-World
 * ================
 *
 * This Module can import a ZEN-World into bs:f.
 */

#include "ImportZEN.hpp"
#include "ImportStaticMesh.hpp"
#include <Components/BsCRenderable.h>
#include <Debug/BsDebug.h>
#include <Resources/BsBuiltinResources.h>
#include <Resources/BsResources.h>
#include <Scene/BsSceneObject.h>
#include <zenload/zCMesh.h>
#include <zenload/zCProgMeshProto.h>
#include <zenload/zenParser.h>

using namespace bs;

static HSceneObject addWorldMesh(ZenLoad::ZenParser& zenParser, const VDFS::FileIndex& vdfs);
static HSceneObject addStaticMeshObject(const String& file, const VDFS::FileIndex& vdfs);
static HSceneObject walkTree(const ZenLoad::zCVobData& root, const VDFS::FileIndex& vdfs);

// - Implementation --------------------------------------------------------------------------------

HSceneObject BsZenLib::ImportZEN(const std::string& zen, const VDFS::FileIndex& vdfs)
{
  HSceneObject worldSO = SceneObject::create(zen.c_str());

  ZenLoad::ZenParser zenParser(zen, vdfs);

  if (zenParser.getFileSize() == 0) return {};

  zenParser.readHeader();

  // TODO: Remove this, or use bs:f logging or something
  std::cout << "---- Loaded ZEN '" << zen << "' ----" << std::endl;
  std::cout << "Author: " << zenParser.getZenHeader().user << std::endl
            << "Date: " << zenParser.getZenHeader().date << std::endl
            << "Object-count (optional): " << zenParser.getZenHeader().objectCount << std::endl;

  // Read the rest of the ZEN-file
  ZenLoad::oCWorldData world;
  zenParser.readWorld(world);
  (void)world;  // TODO: Make use of this

  HSceneObject worldMeshSO = addWorldMesh(zenParser, vdfs);

  if (!worldMeshSO) return {};

  worldMeshSO->setParent(worldSO);

  for (const ZenLoad::zCVobData& child : world.rootVobs)
  {
    HSceneObject vobs = walkTree(child, vdfs);
    vobs->setParent(worldSO);
  }

  return worldSO;
}

static HSceneObject addWorldMesh(ZenLoad::ZenParser& zenParser, const VDFS::FileIndex& vdfs)
{
  ZenLoad::PackedMesh packedMesh;
  zenParser.getWorldMesh()->packMesh(packedMesh, 0.01f);

  HSceneObject so = BsZenLib::ImportStaticMeshWithMaterials("WorldMesh", packedMesh, vdfs);

  if (!so) return {};

  return so;
}

static HSceneObject walkTree(const ZenLoad::zCVobData& root, const VDFS::FileIndex& vdfs)
{
  HSceneObject rootSO;

  if (root.visual.find(".3DS") != std::string::npos)
  {
    std::string compiled = root.visual.substr(0, root.visual.length() - 4) + ".MRM";

    rootSO = addStaticMeshObject(compiled.c_str(), vdfs);
  }

  if (!rootSO)
  {
    rootSO = SceneObject::create(root.vobName.c_str());
  }

  rootSO->setPosition(Vector3(root.position.x, root.position.y, root.position.z) * 0.01f);

  for (const ZenLoad::zCVobData& child : root.childVobs)
  {
    HSceneObject childSO = walkTree(child, vdfs);

    childSO->setParent(rootSO);
  }

  return rootSO;
}

static HSceneObject addStaticMeshObject(const String& file, const VDFS::FileIndex& vdfs)
{
  HSceneObject so;

  // FIXME: Figure out how the resource manager works and register the so there
  static std::map<String, HSceneObject> s_cache;

  if (s_cache.find(file) != s_cache.end())
  {
    so = s_cache.at(file);
  }
  else
  {
    gDebug().logDebug("Loading zCProgMeshProto: " + file);

    ZenLoad::zCProgMeshProto progMesh(file.c_str(), vdfs);

    if (progMesh.getNumSubmeshes() == 0)
    {
      gDebug().logWarning("File not found (zCProgMeshProto): " + String(file.c_str()));
      return {};
    }

    ZenLoad::PackedMesh packedMesh;
    progMesh.packMesh(packedMesh, 0.01f);

    so = BsZenLib::ImportStaticMeshWithMaterials(file.c_str(), packedMesh, vdfs);

    s_cache[file] = so;
  }

  if (!so) return {};

  return so;
}
