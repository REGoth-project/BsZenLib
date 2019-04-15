#include "ImportStaticMesh.hpp"
#include "ImportMaterial.hpp"
#include "ImportPath.hpp"
#include <Components/BsCRenderable.h>
#include <FileSystem/BsFileSystem.h>
#include <RenderAPI/BsVertexDataDesc.h>
#include <Resources/BsBuiltinResources.h>
#include <Resources/BsResources.h>
#include <Scene/BsPrefab.h>
#include <Scene/BsSceneObject.h>
#include <vdfs/fileIndex.h>
#include <zenload/zCMesh.h>
#include <zenload/zCProgMeshProto.h>

using namespace bs;

struct StaticMeshVertex
{
  Vector3 position;
  Vector3 normal;
  Vector2 texCoord;
  uint32_t color;
  Vector3 tangent;
  Vector3 bitangent;
};

static HPrefab cacheStaticMesh(const bs::String& originalFileName, HMesh mesh,
                               const Vector<HMaterial>& materials);
static SPtr<VertexDataDesc> makeVertexDataDescForZenLibVertex();
static MESH_DESC meshDescForPackedMesh(const ZenLoad::PackedMesh& packedMesh);
static Vector<StaticMeshVertex> transformVertices(const ZenLoad::PackedMesh& packedMesh);
static void fillMeshDataFromPackedMesh(HMesh target, const Vector<StaticMeshVertex>& vertices,
                                       const ZenLoad::PackedMesh& packedMesh);
static void transferVertices(SPtr<MeshData> target, const Vector<StaticMeshVertex>& vertices);
static void transferIndices(SPtr<MeshData> target, const ZenLoad::PackedMesh& packedMesh);

// - Implementation --------------------------------------------------------------------------------

BsZenLib::Res::HMeshWithMaterials BsZenLib::ImportAndCacheStaticMesh(
    const bs::String& originalFileName, const VDFS::FileIndex& vdfs)
{

	gDebug().logDebug("Caching Static Mesh: " + originalFileName);

  HMesh mesh = ImportAndCacheStaticMeshGeometry(originalFileName, vdfs);

  if (!mesh)
  {
    gDebug().logWarning("Load Failed (Mesh): " + originalFileName);
    return {};
  }

  Vector<HMaterial> materials = ImportAndCacheStaticMeshMaterials(originalFileName, vdfs);

  if (materials.empty())
  {
    gDebug().logWarning("Load Failed (Materials): " + originalFileName);
    return {};
  }

  Res::HMeshWithMaterials combined = Res::MeshWithMaterials::create(mesh, materials);

  if (!combined)
  {
    gDebug().logWarning("Load Failed (Combined): " + originalFileName);
    return {};
  }

  const bool overwrite = true;
  gResources().save(combined, GothicPathToCachedStaticMesh(originalFileName), overwrite);

  return combined;
}

BsZenLib::Res::HMeshWithMaterials BsZenLib::ImportAndCacheStaticMesh(
    const bs::String& originalFileName, const ZenLoad::PackedMesh& packedMesh,
    const VDFS::FileIndex& vdfs)
{

	gDebug().logDebug("Caching Static Mesh: " + originalFileName);

  HMesh mesh = ImportAndCacheStaticMeshGeometry(originalFileName, packedMesh);

  if (!mesh)
  {
    gDebug().logWarning("Load Failed (Mesh): " + originalFileName);
    return {};
  }

  Vector<HMaterial> materials = ImportAndCacheStaticMeshMaterials(originalFileName, packedMesh, vdfs);

  if (materials.empty())
  {
    gDebug().logWarning("Load Failed (Materials): " + originalFileName);
    return {};
  }

  Res::HMeshWithMaterials combined = Res::MeshWithMaterials::create(mesh, materials);

  if (!combined)
  {
    gDebug().logWarning("Load Failed (Combined): " + originalFileName);
    return {};
  }

  const bool overwrite = true;
  gResources().save(combined, GothicPathToCachedStaticMesh(originalFileName), overwrite);

  return combined;
}

BsZenLib::Res::HMeshWithMaterials BsZenLib::LoadCachedStaticMesh(const bs::String& originalFileName)
{
  return gResources().load<Res::MeshWithMaterials>(GothicPathToCachedStaticMesh(originalFileName));
}

bool BsZenLib::HasCachedStaticMesh(const bs::String& originalFileName)
{
  return FileSystem::isFile(GothicPathToCachedStaticMesh(originalFileName));
}

HMesh BsZenLib::ImportAndCacheStaticMeshGeometry(const bs::String& originalFileName,
                                                 const VDFS::FileIndex& vdfs)
{
  bs::String withoutExt = originalFileName.substr(0, originalFileName.find_last_of('.'));
  bs::String compiledExt = withoutExt + ".MRM";

  ZenLoad::zCProgMeshProto progMesh(compiledExt.c_str(), vdfs);

  if (progMesh.getNumSubmeshes() == 0) return {};

  ZenLoad::PackedMesh packedMesh;
  progMesh.packMesh(packedMesh, 0.01f);

  return ImportAndCacheStaticMeshGeometry(originalFileName, packedMesh);
}

bs::HMesh BsZenLib::ImportAndCacheStaticMeshGeometry(const bs::String& originalFileName,
                                                     const ZenLoad::PackedMesh& packedMesh)
{
  HMesh mesh = ImportStaticMeshGeometry(packedMesh);
  Path path = GothicPathToCachedStaticMesh(originalFileName + ".mesh");

  if (!mesh) return {};

  const bool overwrite = true;
  gResources().save(mesh, path, overwrite);

  return mesh;
}

Vector<HMaterial> BsZenLib::ImportAndCacheStaticMeshMaterials(const bs::String& originalFileName,
                                                              const VDFS::FileIndex& vdfs)
{
  bs::String withoutExt = originalFileName.substr(0, originalFileName.find_last_of('.'));
  bs::String compiledExt = withoutExt + ".MRM";

  ZenLoad::zCProgMeshProto progMesh(compiledExt.c_str(), vdfs);

  if (progMesh.getNumSubmeshes() == 0) return {};

  ZenLoad::PackedMesh packedMesh;
  progMesh.packMesh(packedMesh, 0.01f);

  return ImportAndCacheStaticMeshMaterials(originalFileName, packedMesh, vdfs);
}

Vector<HMaterial> BsZenLib::ImportAndCacheStaticMeshMaterials(const bs::String& originalFileName,
                                                              const ZenLoad::PackedMesh& packedMesh,
                                                              const VDFS::FileIndex& vdfs)
{
  HShader shader = gBuiltinResources().getBuiltinShader(BuiltinShader::Standard);

  Vector<HMaterial> materials;
  for (size_t i = 0; i < packedMesh.subMeshes.size(); i++)
  {
    // TODO: Transfer more properties
    const auto& originalMaterial = packedMesh.subMeshes[i].material;

    HMaterial loaded;
    String materialCacheFile = BuildMaterialNameForSubmesh(originalFileName, (UINT32)i);

    if (HasCachedMaterial(materialCacheFile))
    {
      loaded = LoadCachedMaterial(materialCacheFile);
    }
    else
    {
      loaded = ImportAndCacheMaterialWithTextures(materialCacheFile, originalMaterial, vdfs);
    }

    materials.push_back(loaded);
  }

  return materials;
}

HMesh BsZenLib::ImportStaticMeshGeometry(const ZenLoad::PackedMesh& packedMesh)
{
  MESH_DESC desc = meshDescForPackedMesh(packedMesh);

  HMesh mesh = Mesh::create(desc);

  Vector<StaticMeshVertex> vertices = transformVertices(packedMesh);
  fillMeshDataFromPackedMesh(mesh, vertices, packedMesh);

  return mesh;
}

static MESH_DESC meshDescForPackedMesh(const ZenLoad::PackedMesh& packedMesh)
{
  MESH_DESC desc = {};

  for (const auto& submesh : packedMesh.subMeshes)
  {
    desc.subMeshes.emplace_back();
    desc.subMeshes.back().drawOp = DrawOperationType::DOT_TRIANGLE_LIST;
    desc.subMeshes.back().indexCount = (UINT32)submesh.indices.size();
    desc.subMeshes.back().indexOffset = desc.numIndices;

    desc.numIndices += (UINT32)submesh.indices.size();
  }

  desc.indexType = IndexType::IT_32BIT;
  desc.numVertices = (UINT32)packedMesh.vertices.size();

  desc.vertexDesc = makeVertexDataDescForZenLibVertex();
  desc.usage = MU_CPUCACHED;  // To create our physics mesh later

  return desc;
}

static SPtr<VertexDataDesc> makeVertexDataDescForZenLibVertex()
{
  SPtr<VertexDataDesc> vertexDataDesc = VertexDataDesc::create();
  vertexDataDesc->addVertElem(VET_FLOAT3, VES_POSITION);
  vertexDataDesc->addVertElem(VET_FLOAT3, VES_NORMAL);
  vertexDataDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);
  vertexDataDesc->addVertElem(VET_COLOR, VES_COLOR);
  vertexDataDesc->addVertElem(VET_FLOAT3, VES_TANGENT);
  vertexDataDesc->addVertElem(VET_FLOAT3, VES_BITANGENT);

  return vertexDataDesc;
}

static void fillMeshDataFromPackedMesh(HMesh target, const Vector<StaticMeshVertex>& vertices,
                                       const ZenLoad::PackedMesh& packedMesh)
{
  // Allocate a buffer big enough to hold what we specified in the MESH_DESC
  SPtr<MeshData> meshData = target->allocBuffer();

  transferVertices(meshData, vertices);
  transferIndices(meshData, packedMesh);

  target->writeData(meshData, false);
}

static Vector<StaticMeshVertex> transformVertices(const ZenLoad::PackedMesh& packedMesh)
{
  Vector<StaticMeshVertex> v;

  for (const ZenLoad::WorldVertex& oldVertex : packedMesh.vertices)
  {
    StaticMeshVertex newVertex = {};

    newVertex.position = Vector3(oldVertex.Position.x, oldVertex.Position.y, oldVertex.Position.z);
    newVertex.normal = Vector3(oldVertex.Normal.x, oldVertex.Normal.y, oldVertex.Normal.z);
    newVertex.texCoord = Vector2(oldVertex.TexCoord.x, oldVertex.TexCoord.y);

    newVertex.tangent = Vector3();
    newVertex.bitangent = Vector3();

    v.push_back(newVertex);
  }

  return v;
}

static void transferVertices(SPtr<MeshData> target, const Vector<StaticMeshVertex>& vertices)
{
  assert(target->getNumVertices() == vertices.size());

  UINT8* pVertices = target->getElementData(VES_POSITION);
  memcpy(pVertices, vertices.data(), sizeof(StaticMeshVertex) * vertices.size());
}

static void transferIndices(SPtr<MeshData> target, const ZenLoad::PackedMesh& packedMesh)
{
  UINT32* pIndices = target->getIndices32();
  size_t writtenSoFar = 0;

  for (const auto& submesh : packedMesh.subMeshes)
  {
    memcpy(&pIndices[writtenSoFar], submesh.indices.data(), sizeof(UINT32) * submesh.indices.size());
    writtenSoFar += submesh.indices.size();
  }
}
