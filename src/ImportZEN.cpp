/**
 * Import ZEN-World
 * ================
 *
 * This Module can import a ZEN-World into bs:f.
 */

#include "ImportZEN.hpp"
#include "ImportPath.hpp"
#include "ImportStaticMesh.hpp"
#include <Components/BsCRenderable.h>
#include <Debug/BsDebug.h>
#include <FileSystem/BsFileSystem.h>
#include <Resources/BsBuiltinResources.h>
#include <Resources/BsResources.h>
#include <Scene/BsSceneObject.h>
#include <zenload/zCMesh.h>
#include <zenload/zCProgMeshProto.h>
#include <zenload/zenParser.h>

using namespace bs;
using namespace BsZenLib;
using namespace BsZenLib::Res;

static HSceneObject addWorldMesh(const bs::String& worldName, ZenLoad::ZenParser& zenParser,
                                 const VDFS::FileIndex& vdfs);
static HSceneObject addStaticMeshObject(const String& file, const VDFS::FileIndex& vdfs);
static HSceneObject walkTree(const ZenLoad::zCVobData& root, const VDFS::FileIndex& vdfs);

// - Implementation --------------------------------------------------------------------------------

Vector<UUID> GetDependenciesRecursive(UUID uuid)
{
  Path filePath;

  if (!gResources().getFilePathFromUUID(uuid, filePath)) return {};

  Vector<UUID> deps = gResources().getDependencies(filePath);

  for (UUID a : deps)
  {
    Vector<UUID> childDeps = GetDependenciesRecursive(a);

    for (UUID b : childDeps)
    {
      deps.push_back(b);
    }
  }

  return deps;
}

bool BsZenLib::HasCachedZEN(const bs::String& zen)
{
  return FileSystem::isFile(GothicPathToCachedWorld(zen.c_str()));
}

bs::HPrefab BsZenLib::LoadCachedZEN(const bs::String& zen)
{
  return gResources().loadAsync<Prefab>(GothicPathToCachedWorld(zen));
}

bs::HSceneObject BsZenLib::ImportAndCacheZEN(const std::string& zen, const VDFS::FileIndex& vdfs)
{
  HSceneObject worldSO = ImportZEN(zen, vdfs);

  if (!worldSO) return {};

  HPrefab worldPrefab = Prefab::create(worldSO);

  const bool overwrite = false;
  gResources().save(worldPrefab, BsZenLib::GothicPathToCachedWorld(zen.c_str()), overwrite);

  return worldSO;
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

static HSceneObject addWorldMesh(const bs::String& worldName, ZenLoad::ZenParser& zenParser,
                                 const VDFS::FileIndex& vdfs)
{
  String meshFileName = worldName + ".worldmesh";

  HMeshWithMaterials mesh;
  if (HasCachedStaticMesh(meshFileName))
  {
    mesh = BsZenLib::LoadCachedStaticMesh(meshFileName);
  }
  else
  {
    ZenLoad::PackedMesh packedMesh;
    zenParser.getWorldMesh()->packMesh(packedMesh, 0.01f);

    mesh = BsZenLib::ImportAndCacheStaticMesh(meshFileName, packedMesh, vdfs);
  }

  if (!mesh) return {};

  HSceneObject meshSO = SceneObject::create(meshFileName);
  HRenderable renderable = meshSO->addComponent<CRenderable>();
  renderable->setMesh(mesh->getMesh());
  renderable->setMaterials(mesh->getMaterials());

  return meshSO;
}

Matrix4 convertMatrix(const ZMath::Matrix& m)
{
  Matrix4 bs = {m.mv[0], m.mv[1], m.mv[2],  m.mv[3],  m.mv[4],  m.mv[5],  m.mv[6],  m.mv[7],
                m.mv[8], m.mv[9], m.mv[10], m.mv[11], m.mv[12], m.mv[13], m.mv[14], m.mv[15]};

  return bs.transpose();
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

  Matrix4 worldMatrix = convertMatrix(root.worldMatrix);
  Quaternion rotation;
  rotation.fromRotationMatrix(worldMatrix.get3x3());

  rootSO->setRotation(rotation);
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
  HMeshWithMaterials mesh;
  if (FileSystem::isFile(BsZenLib::GothicPathToCachedStaticMesh(file.c_str())))
  {
    mesh = BsZenLib::LoadCachedStaticMesh(file.c_str());
  }
  else
  {
    mesh = BsZenLib::ImportAndCacheStaticMesh(file.c_str(), vdfs);
  }

  if (!mesh) return {};

  HSceneObject meshSO = SceneObject::create(file);
  HRenderable renderable = meshSO->addComponent<CRenderable>();
  renderable->setMesh(mesh->getMesh());
  renderable->setMaterials(mesh->getMaterials());

  return meshSO;
}
