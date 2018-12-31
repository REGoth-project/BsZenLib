#include "ImportStaticMesh.hpp"
#include "ImportTexture.hpp"
#include <Components/BsCRenderable.h>
#include <RenderAPI/BsVertexDataDesc.h>
#include <Resources/BsBuiltinResources.h>
#include <Scene/BsPrefab.h>
#include <Scene/BsSceneObject.h>
#include <vdfs/fileIndex.h>
#include <zenload/zCMesh.h>
#include <zenload/zCProgMeshProto.h>

using namespace bs;

static SPtr<VertexDataDesc> makeVertexDataDescForZenLibVertex();
static MESH_DESC meshDescForPackedMesh(const ZenLoad::PackedMesh& packedMesh);
static void fillMeshDataFromPackedMesh(HMesh target, const ZenLoad::PackedMesh& packedMesh);
static void transferVertices(SPtr<MeshData> target, const ZenLoad::PackedMesh& packedMesh);
static void transferIndices(SPtr<MeshData> target, const ZenLoad::PackedMesh& packedMesh);
static Vector<HMaterial> materialsFromStaticMesh(const ZenLoad::PackedMesh& packedMesh,
                                                 const VDFS::FileIndex& vdfs);

// - Implementation --------------------------------------------------------------------------------

HSceneObject BsZenLib::ImportStaticMeshWithMaterials(const std::string& name,
                                                     const ZenLoad::PackedMesh& packedMesh,
                                                     const VDFS::FileIndex& vdfs)
{
  HMesh mesh = ImportStaticMesh(packedMesh);
  Vector<HMaterial> materials = materialsFromStaticMesh(packedMesh, vdfs);

  if (!mesh)
  {
    gDebug().logWarning("Load Failed (Mesh): " + String(name.c_str()));
    return {};
  }

  if (materials.empty())
  {
    gDebug().logWarning("Load Failed (Materials): " + String(name.c_str()));
    return {};
  }

  HSceneObject so = SceneObject::create(name.c_str());

  HRenderable renderable = so->addComponent<CRenderable>();
  renderable->setMesh(mesh);
  renderable->setMaterials(materials);

  return so;
}

HMesh BsZenLib::ImportStaticMesh(const ZenLoad::PackedMesh& packedMesh)
{
  MESH_DESC desc = meshDescForPackedMesh(packedMesh);

  HMesh mesh = Mesh::create(desc);

  fillMeshDataFromPackedMesh(mesh, packedMesh);

  return mesh;
}

static MESH_DESC meshDescForPackedMesh(const ZenLoad::PackedMesh& packedMesh)
{
  MESH_DESC desc = {};

  for (const auto& submesh : packedMesh.subMeshes)
  {
    desc.subMeshes.emplace_back();
    desc.subMeshes.back().drawOp = DrawOperationType::DOT_TRIANGLE_LIST;
    desc.subMeshes.back().indexCount = submesh.indices.size();
    desc.subMeshes.back().indexOffset = desc.numIndices;

    desc.numIndices += submesh.indices.size();
  }

  desc.indexType = IndexType::IT_32BIT;
  desc.numVertices = packedMesh.vertices.size();

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

  assert(vertexDataDesc->getVertexStride() == sizeof(ZenLoad::WorldVertex));

  return vertexDataDesc;
}

static void fillMeshDataFromPackedMesh(HMesh target, const ZenLoad::PackedMesh& packedMesh)
{
  // Allocate a buffer big enough to hold what we specified in the MESH_DESC
  SPtr<MeshData> meshData = target->allocBuffer();

  transferVertices(meshData, packedMesh);
  transferIndices(meshData, packedMesh);

  target->writeData(meshData, false);
}

static void transferVertices(SPtr<MeshData> target, const ZenLoad::PackedMesh& packedMesh)
{
  assert(target->getNumVertices() == packedMesh.vertices.size());

  UINT8* vertices = target->getElementData(VES_POSITION);
  memcpy(vertices, packedMesh.vertices.data(),
         sizeof(ZenLoad::WorldVertex) * packedMesh.vertices.size());
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

static Vector<HMaterial> materialsFromStaticMesh(const ZenLoad::PackedMesh& packedMesh,
                                                 const VDFS::FileIndex& vdfs)
{
  HShader shader = gBuiltinResources().getBuiltinShader(BuiltinShader::Standard);

  Vector<HMaterial> materials;
  for (const auto& m : packedMesh.subMeshes)
  {
    // TODO: Transfer more properties

    materials.push_back(Material::create(shader));

    HTexture albedo = BsZenLib::ImportTexture(m.material.texture, vdfs);

    materials.back()->setTexture("gAlbedoTex", albedo);
  }

  return materials;
}
