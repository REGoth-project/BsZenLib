#include "ImportSkeletalMesh.hpp"
#include "ImportTexture.hpp"
#include <Components/BsCRenderable.h>
#include <RenderAPI/BsVertexDataDesc.h>
#include <Resources/BsBuiltinResources.h>
#include <Scene/BsSceneObject.h>
#include <vdfs/fileIndex.h>
#include <zenload/modelScriptParser.h>
#include <zenload/zCModelMeshLib.h>
#include <zenload/zTypes.h>
#include <zenload/zenParser.h>
#include <Resources/BsResourceManifest.h>
#include <Resources/BsResources.h>
#include "ImportPath.hpp"
#include <FileSystem/BsFileSystem.h>
#include <Scene/BsPrefab.h>

using namespace bs;

struct SkeletalVertex
{
  Vector3 position;
  Vector3 normal;
  Vector2 texCoord;
  Vector3 tangent;
  Vector3 bitangent;
  UINT8 boneIndices[4];
  float boneWeights[4];
};

static SPtr<VertexDataDesc> makeVertexDataDescForZenLibVertex();
static MESH_DESC meshDescForPackedMesh(const ZenLoad::PackedSkeletalMesh& packedMesh);
static Vector<Matrix4> makeBindPose(const std::vector<ZenLoad::ModelNode>& nodes);
static Matrix4 convertMatrix(const ZMath::Matrix& m);
static Vector3 transformToBindPose(const Vector<Matrix4>& bindPose,
                                   const ZenLoad::SkeletalVertex& vertex);
static Vector<SkeletalVertex> transformVertices(const Vector<Matrix4>& bindPose,
                                                const ZenLoad::PackedSkeletalMesh& packedMesh);
static void fillMeshDataFromPackedMesh(HMesh target, const Vector<SkeletalVertex>& vertices,
                                       const ZenLoad::PackedSkeletalMesh& packedMesh);
static void transferVertices(SPtr<MeshData> target, const Vector<SkeletalVertex>& vertices);
static void transferIndices(SPtr<MeshData> target, const ZenLoad::PackedSkeletalMesh& packedMesh);
static Vector<HMaterial> materialsFromSkeletalMesh(const ZenLoad::PackedSkeletalMesh& packedMesh,
                                                   const VDFS::FileIndex& vdfs);
static HPrefab cacheSkeletalMesh(const bs::String& virtualFilePath, HMesh mesh, const Vector<HMaterial>& materials);

// - Implementation --------------------------------------------------------------------------------

bool BsZenLib::HasCachedSkeletalMeshPrefab(const bs::String& virtualFilePath)
{
	return FileSystem::isFile(GothicPathToCachedAsset(virtualFilePath.c_str()));
}

bs::HPrefab BsZenLib::LoadCachedSkeletalMeshPrefab(const bs::String& virtualFilePath)
{
	return gResources().load<Prefab>(GothicPathToCachedAsset(virtualFilePath));
}

bs::HPrefab BsZenLib::ImportAndCacheSkeletalMeshPrefab(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs)
{
	HMesh mesh = ImportAndCacheSkeletalMesh(virtualFilePath, vdfs);

	if (!mesh)
		return {};

	Vector<HMaterial> materials = ImportAndCacheSkeletalMeshMaterials(virtualFilePath, vdfs);

	if (!mesh)
	{
		gDebug().logWarning("Load Failed (Mesh): " + virtualFilePath);
		return {};
	}

	if (materials.empty())
	{
		gDebug().logWarning("Load Failed (Materials): " + virtualFilePath);
		return {};
	}

	return cacheSkeletalMesh(virtualFilePath, mesh, materials);
}

static HPrefab cacheSkeletalMesh(const bs::String& virtualFilePath, HMesh mesh, const Vector<HMaterial>& materials)
{
	using namespace BsZenLib;

	HSceneObject so = SceneObject::create(virtualFilePath.c_str());

	HRenderable renderable = so->addComponent<CRenderable>();
	renderable->setMesh(mesh);
	renderable->setMaterials(materials);
	renderable->setC

	HPrefab prefab = Prefab::create(so, false);

	so->destroy(true);

	const bool overwrite = false;
	gResources().save(prefab, GothicPathToCachedAsset(virtualFilePath), overwrite);

	return prefab;
}


bs::HMesh BsZenLib::LoadCachedSkeletalMesh(const bs::String& virtualFilePath)
{
	Path path = GothicPathToCachedAsset(virtualFilePath + ".mesh");

	return gResources().load<Mesh>(path);
}

HMesh BsZenLib::ImportAndCacheSkeletalMesh(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs)
{
	HMesh mesh = ImportSkeletalMesh(virtualFilePath, vdfs);
	Path path = GothicPathToCachedAsset(virtualFilePath + ".mesh");

	if (!mesh)
		return {};

	const bool overwrite = true;
	gResources().save(mesh, path, overwrite);

	return gResources().load<Mesh>(path);
}

Vector<HMaterial> BsZenLib::ImportAndCacheSkeletalMeshMaterials(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs)
{
	ZenLoad::zCModelMeshLib meshLib(virtualFilePath.c_str(), vdfs);

	if (!meshLib.isValid()) return {};

	ZenLoad::PackedSkeletalMesh packedMesh;
	meshLib.packMesh(packedMesh);

	HShader shader = gBuiltinResources().getBuiltinShader(BuiltinShader::Standard);

	Vector<HMaterial> materials;
	for (size_t i = 0; i < packedMesh.subMeshes.size(); i++)
	{
		// TODO: Transfer more properties
		const auto& originalMaterial = packedMesh.subMeshes[i].material;

		HTexture albedo;

		if (BsZenLib::HasCachedTexture(originalMaterial.texture.c_str()))
		{
			albedo = BsZenLib::LoadCachedTexture(originalMaterial.texture.c_str());
		}
		else
		{
			albedo = BsZenLib::ImportAndCacheTexture(originalMaterial.texture.c_str(), vdfs);
		}

		Path materialPath = GothicPathToCachedAsset(virtualFilePath + ".material-" + toString(i));

		HMaterial m = Material::create(shader);
		m->setTexture("gAlbedoTex", albedo);

		const bool overwrite = false;
		gResources().save(m, materialPath, overwrite);

		HMaterial loaded = gResources().load<Material>(materialPath);
		materials.push_back(loaded);
	}

	return materials;
}

Vector<HMaterial> BsZenLib::ImportMaterialsFromSkeletalMesh(const String& virtualFilePath,
                                                  const VDFS::FileIndex& vdfs)
{
  ZenLoad::zCModelMeshLib meshLib(virtualFilePath.c_str(), vdfs);

  if (!meshLib.isValid()) return {};

  Vector<Matrix4> bindPose = makeBindPose(meshLib.getNodes());

  ZenLoad::PackedSkeletalMesh packedMesh;
  meshLib.packMesh(packedMesh);

  return materialsFromSkeletalMesh(packedMesh, vdfs);
}

bs::HSceneObject BsZenLib::ImportSkeletalMeshWithMaterials(const String& virtualFilePath,
                                                           const VDFS::FileIndex& vdfs)
{
  ZenLoad::zCModelMeshLib meshLib(virtualFilePath.c_str(), vdfs);

  if (!meshLib.isValid()) return {};

  Vector<Matrix4> bindPose = makeBindPose(meshLib.getNodes());

  ZenLoad::PackedSkeletalMesh packedMesh;
  meshLib.packMesh(packedMesh);

  HMesh mesh = ImportSkeletalMesh(bindPose, packedMesh);
  Vector<HMaterial> materials = materialsFromSkeletalMesh(packedMesh, vdfs);

  if (!mesh)
  {
    gDebug().logWarning("Load Failed (Mesh): " + String(virtualFilePath.c_str()));
    return {};
  }

  if (materials.empty())
  {
    gDebug().logWarning("Load Failed (Materials): " + String(virtualFilePath.c_str()));
    return {};
  }

  HSceneObject so = SceneObject::create(virtualFilePath.c_str());

  HRenderable renderable = so->addComponent<CRenderable>();
  renderable->setMesh(mesh);
  renderable->setMaterials(materials);

  return so;
}

bs::HMesh BsZenLib::ImportSkeletalMesh(const String& virtualFilePath,
                                       const VDFS::FileIndex& vdfs)
{
  ZenLoad::zCModelMeshLib meshLib(virtualFilePath.c_str(), vdfs);

  if (!meshLib.isValid()) return {};

  Vector<Matrix4> bindPose = makeBindPose(meshLib.getNodes());

  ZenLoad::PackedSkeletalMesh packedMesh;
  meshLib.packMesh(packedMesh);

  return ImportSkeletalMesh(bindPose, packedMesh);
}

static Vector<Matrix4> makeBindPose(const std::vector<ZenLoad::ModelNode>& nodes)
{
  Vector<Matrix4> bindPose(nodes.size());
  Vector<Matrix4> nodeTransforms;

  for (const auto& node : nodes)
  {
    nodeTransforms.push_back(convertMatrix(node.transformLocal).transpose());
  }

  // Calculate actual node matrices
  for (size_t i = 0; i < nodes.size(); i++)
  {
    // TODO: There is a flag indicating whether the animation root should translate the vob position
    // Move root node to (0,0,0)
    if (i == 0)
    {
      Vector3 position;
      Quaternion rotation;
      Vector3 scale;

      nodeTransforms[i].decomposition(position, rotation, scale);

      scale *= 0.01;  // Scale centimeters -> meters
      nodeTransforms[i].setTRS(Vector3(0.0f, 0.0f, 0.0f), rotation, scale);
    }

    if (nodes[i].parentValid())
    {
      bindPose[i] = bindPose[nodes[i].parentIndex] * nodeTransforms[i];
    }
    else
    {
      bindPose[i] = nodeTransforms[i];
    }
  }

  return bindPose;
}

static Matrix4 convertMatrix(const ZMath::Matrix& m)
{
  return {m.mv[0], m.mv[1], m.mv[2],  m.mv[3],  m.mv[4],  m.mv[5],  m.mv[6],  m.mv[7],
          m.mv[8], m.mv[9], m.mv[10], m.mv[11], m.mv[12], m.mv[13], m.mv[14], m.mv[15]};
}

bs::Vector<bs::Matrix4> BsZenLib::getBindPose(const std::string& virtualFilePath,
                                              const VDFS::FileIndex& vdfs)
{
  ZenLoad::zCModelMeshLib meshLib(virtualFilePath, vdfs);

  if (!meshLib.isValid()) return {};

  Vector<Matrix4> bindPose = makeBindPose(meshLib.getNodes());

  return bindPose;
}

bs::HMesh BsZenLib::ImportSkeletalMesh(const Vector<Matrix4>& bindPose,
                                       const ZenLoad::PackedSkeletalMesh& packedMesh)
{
  MESH_DESC desc = meshDescForPackedMesh(packedMesh);

  if (desc.numIndices == 0)
	  return {};

  HMesh mesh = Mesh::create(desc);

  Vector<SkeletalVertex> vertices = transformVertices(bindPose, packedMesh);
  fillMeshDataFromPackedMesh(mesh, vertices, packedMesh);

  return mesh;
}

static Vector<SkeletalVertex> transformVertices(const Vector<Matrix4>& bindPose,
                                                const ZenLoad::PackedSkeletalMesh& packedMesh)
{
  Vector<SkeletalVertex> v;

  for (const ZenLoad::SkeletalVertex& oldVertex : packedMesh.vertices)
  {
    SkeletalVertex newVertex = {};

    newVertex.position = transformToBindPose(bindPose, oldVertex);
    newVertex.normal = Vector3(oldVertex.Normal.x, oldVertex.Normal.y, oldVertex.Normal.z);
    newVertex.texCoord = Vector2(oldVertex.TexCoord.x, oldVertex.TexCoord.y);

	newVertex.tangent = Vector3();
	newVertex.bitangent = Vector3();

    newVertex.boneWeights[0] = oldVertex.Weights[0];
    newVertex.boneWeights[1] = oldVertex.Weights[1];
    newVertex.boneWeights[2] = oldVertex.Weights[2];
    newVertex.boneWeights[3] = oldVertex.Weights[3];

    newVertex.boneIndices[0] = oldVertex.BoneIndices[0];
    newVertex.boneIndices[1] = oldVertex.BoneIndices[1];
    newVertex.boneIndices[2] = oldVertex.BoneIndices[2];
    newVertex.boneIndices[3] = oldVertex.BoneIndices[3];

    v.push_back(newVertex);
  }

  return v;
}

static Vector3 transformToBindPose(const Vector<Matrix4>& bindPose,
                                   const ZenLoad::SkeletalVertex& vertex)
{
  Vector3 transformed = Vector3(0.0f, 0.0f, 0.0f);

  for (size_t i = 0; i < 4; i++)
  {
    Vector3 localPosition =
        Vector3(vertex.LocalPositions[i].x, vertex.LocalPositions[i].y, vertex.LocalPositions[i].z);

    Vector4 tmp = bindPose[vertex.BoneIndices[i]].multiply(Vector4(localPosition, 1.0f));
    transformed += Vector3(tmp) * vertex.Weights[i];
  }

  return transformed;
}

static MESH_DESC meshDescForPackedMesh(const ZenLoad::PackedSkeletalMesh& packedMesh)
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
  vertexDataDesc->addVertElem(VET_FLOAT3, VES_TANGENT);
  vertexDataDesc->addVertElem(VET_FLOAT3, VES_BITANGENT);
  // vertexDataDesc->addVertElem(VET_UBYTE4, VES_BLEND_INDICES);
  // vertexDataDesc->addVertElem(VET_FLOAT4, VES_BLEND_WEIGHTS);
  vertexDataDesc->addVertElem(VET_UBYTE4, VES_TEXCOORD, 1);
  vertexDataDesc->addVertElem(VET_FLOAT4, VES_TEXCOORD, 2);

  assert(vertexDataDesc->getVertexStride() == sizeof(SkeletalVertex));

  return vertexDataDesc;
}

static void fillMeshDataFromPackedMesh(HMesh target, const Vector<SkeletalVertex>& vertices,
                                       const ZenLoad::PackedSkeletalMesh& packedMesh)
{
  // Allocate a buffer big enough to hold what we specified in the MESH_DESC
  SPtr<MeshData> meshData = target->allocBuffer();

  transferVertices(meshData, vertices);
  transferIndices(meshData, packedMesh);

  target->writeData(meshData, false);
}

static void transferVertices(SPtr<MeshData> target, const Vector<SkeletalVertex>& vertices)
{
  assert(target->getNumVertices() == vertices.size());

  UINT8* pVertices = target->getElementData(VES_POSITION);
  memcpy(pVertices, vertices.data(), sizeof(SkeletalVertex) * vertices.size());
}

static void transferIndices(SPtr<MeshData> target, const ZenLoad::PackedSkeletalMesh& packedMesh)
{
  UINT32* pIndices = target->getIndices32();
  size_t writtenSoFar = 0;

  for (const auto& submesh : packedMesh.subMeshes)
  {
    memcpy(&pIndices[writtenSoFar], submesh.indices.data(), sizeof(UINT32) * submesh.indices.size());
    writtenSoFar += submesh.indices.size();
  }
}

static Vector<HMaterial> materialsFromSkeletalMesh(const ZenLoad::PackedSkeletalMesh& packedMesh,
                                                   const VDFS::FileIndex& vdfs)
{
  HShader shader = gBuiltinResources().getBuiltinShader(BuiltinShader::Standard);

  Vector<HMaterial> materials;
  for (const auto& m : packedMesh.subMeshes)
  {
    // TODO: Transfer more properties

    materials.push_back(Material::create(shader));

	HTexture albedo;

	if (BsZenLib::HasCachedTexture(m.material.texture.c_str()))
	{
		albedo = BsZenLib::LoadCachedTexture(m.material.texture.c_str());
	}
	else
	{
		albedo = BsZenLib::ImportAndCacheTexture(m.material.texture.c_str(), vdfs);
	}

    materials.back()->setTexture("gAlbedoTex", albedo);
  }

  return materials;
}
