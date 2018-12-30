/**
 * # Import World Mesh
 *
 * This Module is able to take a zCMesh produced by ZenLib and turn it into a bs::Mesh.
 */

#include "ImportWorldMesh.hpp"

#include "ImportTexture.hpp"
#include <RenderAPI/BsVertexDataDesc.h>
#include <Resources/BsBuiltinResources.h>
#include <vdfs/fileIndex.h>
#include <zenload/zCMesh.h>

static bs::SPtr<bs::VertexDataDesc> makeVertexDataDescForZenLibVertex();
static bs::MESH_DESC meshDescForPackedMesh(const ZenLoad::PackedMesh& packedMesh);
static void fillMeshDataFromPackedMesh(bs::HMesh target, const ZenLoad::PackedMesh& packedMesh);
static void transferVertices(bs::SPtr<bs::MeshData> target, const ZenLoad::PackedMesh& packedMesh);
static void transferIndices(bs::SPtr<bs::MeshData> target, const ZenLoad::PackedMesh& packedMesh);

// - Implementation --------------------------------------------------------------------------------

bs::HMesh BsZenLib::ImportMeshFromZEN(/* const */ ZenLoad::zCMesh& zenMesh)
{
  using namespace bs;

  ZenLoad::PackedMesh packedMesh;
  zenMesh.packMesh(packedMesh, 0.01f);

  MESH_DESC desc = meshDescForPackedMesh(packedMesh);

  HMesh mesh = Mesh::create(desc);

  fillMeshDataFromPackedMesh(mesh, packedMesh);

  return mesh;
}

static bs::MESH_DESC meshDescForPackedMesh(const ZenLoad::PackedMesh& packedMesh)
{
  using namespace bs;

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
  desc.usage = MU_CPUCACHED; // To create our physics mesh later

  return desc;
}

static bs::SPtr<bs::VertexDataDesc> makeVertexDataDescForZenLibVertex()
{
  using namespace bs;

  SPtr<VertexDataDesc> vertexDataDesc = VertexDataDesc::create();
  vertexDataDesc->addVertElem(VET_FLOAT3, VES_POSITION);
  vertexDataDesc->addVertElem(VET_FLOAT3, VES_NORMAL);
  vertexDataDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);
  vertexDataDesc->addVertElem(VET_COLOR, VES_COLOR);

  assert(vertexDataDesc->getVertexStride() == sizeof(ZenLoad::WorldVertex));

  return vertexDataDesc;
}

static void fillMeshDataFromPackedMesh(bs::HMesh target, const ZenLoad::PackedMesh& packedMesh)
{
  using namespace bs;

  // Allocate a buffer big enough to hold what we specified in the MESH_DESC
  SPtr<MeshData> meshData = target->allocBuffer();

  transferVertices(meshData, packedMesh);
  transferIndices(meshData, packedMesh);

  target->writeData(meshData, false);
}

static void transferVertices(bs::SPtr<bs::MeshData> target, const ZenLoad::PackedMesh& packedMesh)
{
  using namespace bs;

  assert(target->getNumVertices() == packedMesh.vertices.size());

  UINT8* vertices = target->getElementData(VES_POSITION);
  memcpy(vertices, packedMesh.vertices.data(),
         sizeof(ZenLoad::WorldVertex) * packedMesh.vertices.size());
}

static void transferIndices(bs::SPtr<bs::MeshData> target, const ZenLoad::PackedMesh& packedMesh)
{
  using namespace bs;

  UINT32* pIndices = target->getIndices32();
  size_t writtenSoFar = 0;

  for (const auto& submesh : packedMesh.subMeshes)
  {
    memcpy(&pIndices[writtenSoFar], submesh.indices.data(), sizeof(UINT32) * submesh.indices.size());
    writtenSoFar += submesh.indices.size();
  }
}

bs::Vector<bs::HMaterial> BsZenLib::ImportMaterialsFromZENMesh(/* const */ ZenLoad::zCMesh& zenMesh,
                                                               const VDFS::FileIndex& vdfs)
{
  using namespace bs;

  // FIXME: This reorders the list of materials and puts them into the submeshes
  //        We don't want to pack the mesh twice, so either somehow don't reorder
  //        or get the reordered list without packing...
  ZenLoad::PackedMesh packedMesh;
  zenMesh.packMesh(packedMesh, 0.01f, false);

  HShader shader = gBuiltinResources().getBuiltinShader(BuiltinShader::Standard);

  bs::Vector<HMaterial> materials;
  for (const auto& m : packedMesh.subMeshes)
  {
    // TODO: Transfer more properties

    materials.push_back(Material::create(shader));

    HTexture albedo = ImportTexture(m.material.texture, vdfs);

    materials.back()->setTexture("gAlbedoTex", albedo);
  }

  return materials;
}
