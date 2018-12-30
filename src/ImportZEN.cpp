/**
 * Import ZEN-World
 * ================
 *
 * This Module can import a ZEN-World into bs:f.
 */

#include "ImportZEN.hpp"
#include "ImportWorldMesh.hpp"
#include <Components/BsCRenderable.h>
#include <Scene/BsSceneObject.h>
#include <zenload/zenParser.h>
#include <Debug/BsDebug.h>

static bs::HSceneObject addWorldMesh(ZenLoad::ZenParser& zenParser, const VDFS::FileIndex& vdfs);

// - Implementation --------------------------------------------------------------------------------

bs::HSceneObject BsZenLib::ImportZEN(const std::string& zen, const VDFS::FileIndex& vdfs)
{
  using namespace bs;

  HSceneObject worldSO = SceneObject::create(zen.c_str());

  ZenLoad::ZenParser zenParser(zen, vdfs);

  if (zenParser.getFileSize() == 0)
    return {};

  zenParser.readHeader();

  // TODO: Remove this, or use bs:f logging or something
  std::cout << "---- Loaded ZEN '" << zen << "' ----" << std::endl;
  std::cout << "Author: " << zenParser.getZenHeader().user << std::endl
            << "Date: " << zenParser.getZenHeader().date << std::endl
            << "Object-count (optional): " << zenParser.getZenHeader().objectCount << std::endl;

  // Read the rest of the ZEN-file
  ZenLoad::oCWorldData world;
  zenParser.readWorld(world);
  (void)world; // TODO: Make use of this

  HSceneObject worldMeshSO = addWorldMesh(zenParser, vdfs);

  if (!worldMeshSO)
    return {};

  worldMeshSO->setParent(worldSO);

  return worldSO;
}

static bs::HSceneObject addWorldMesh(ZenLoad::ZenParser& zenParser, const VDFS::FileIndex& vdfs)
{
  using namespace bs;

  HMesh worldMesh = BsZenLib::ImportMeshFromZEN(*zenParser.getWorldMesh());
  Vector<HMaterial> materials = BsZenLib::ImportMaterialsFromZENMesh(*zenParser.getWorldMesh(), vdfs);

  HSceneObject worldMeshSO = SceneObject::create("WorldMesh");
  HRenderable worldMeshRenderable = worldMeshSO->addComponent<CRenderable>();

  worldMeshRenderable->setMesh(worldMesh);
  worldMeshRenderable->setMaterials(materials);

  return worldMeshSO;
}
