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
#include <FileSystem/BsFileSystem.h>
#include "ImportPath.hpp"

using namespace bs;

static HSceneObject addWorldMesh(const bs::String& worldName, ZenLoad::ZenParser& zenParser, const VDFS::FileIndex& vdfs);
static HSceneObject addStaticMeshObject(const String& file, const VDFS::FileIndex& vdfs);
static HSceneObject walkTree(const ZenLoad::zCVobData& root, const VDFS::FileIndex& vdfs);

// - Implementation --------------------------------------------------------------------------------

bool BsZenLib::HasCachedZEN(const bs::String& zen)
{
	return FileSystem::isFile(GothicPathToCachedWorld(zen.c_str()));
}

bs::HPrefab BsZenLib::LoadCachedZEN(const bs::String& zen)
{
	return gResources().loadAsync<Prefab>(GothicPathToCachedWorld(zen));
}

bs::HPrefab BsZenLib::ImportAndCacheZEN(const std::string& zen, const VDFS::FileIndex& vdfs)
{
	HSceneObject worldSO = ImportZEN(zen, vdfs);

	if (!worldSO)
		return {};

	HPrefab worldPrefab = Prefab::create(worldSO);
	worldSO->destroy(true);

	const bool overwrite = false;
	gResources().save(worldPrefab, BsZenLib::GothicPathToCachedWorld(zen.c_str()), overwrite);

	return worldPrefab;
}

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

  HSceneObject worldMeshSO = addWorldMesh(zen.c_str(), zenParser, vdfs);

  if (!worldMeshSO) return {};

  worldMeshSO->setParent(worldSO);

  for (const ZenLoad::zCVobData& child : world.rootVobs)
  {
    HSceneObject vobs = walkTree(child, vdfs);
    vobs->setParent(worldSO);
  }

  return worldSO;
}

static HSceneObject addWorldMesh(const bs::String& worldName, ZenLoad::ZenParser& zenParser, const VDFS::FileIndex& vdfs)
{
  ZenLoad::PackedMesh packedMesh;
  zenParser.getWorldMesh()->packMesh(packedMesh, 0.01f);

  String meshFileName = worldName + ".worldmesh";

  HPrefab prefab;
  if (FileSystem::isFile(BsZenLib::GothicPathToCachedAsset(meshFileName)))
  {
	  prefab = BsZenLib::LoadCachedStaticMeshPrefab(meshFileName);
  }
  else
  {
	  prefab = BsZenLib::ImportAndCacheStaticMeshPrefab(meshFileName, packedMesh, vdfs);
  }

  if (!prefab)
	  return {};

  return prefab->instantiate();
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
	HPrefab prefab;

  if (FileSystem::isFile(BsZenLib::GothicPathToCachedAsset(file)))
  {
	  prefab = BsZenLib::LoadCachedStaticMeshPrefab(file);
  }
  else
  {
	  prefab = BsZenLib::ImportAndCacheStaticMeshPrefab(file, vdfs);
  }

  if (!prefab)
	  return {};

  return prefab->instantiate();
}
