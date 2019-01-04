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
#include "ImportPath.hpp"
#include <Resources/BsResources.h>
#include <Scene/BsPrefab.h>
#include <FileSystem/BsFileSystem.h>
#include "ImportPath.hpp"

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

static HPrefab cacheStaticMesh(const bs::String& virtualFilePath, HMesh mesh, const Vector<HMaterial>& materials);
static SPtr<VertexDataDesc> makeVertexDataDescForZenLibVertex();
static MESH_DESC meshDescForPackedMesh(const ZenLoad::PackedMesh& packedMesh);
static Vector<StaticMeshVertex> transformVertices(const ZenLoad::PackedMesh& packedMesh);
static void fillMeshDataFromPackedMesh(HMesh target, const Vector<StaticMeshVertex>& vertices, const ZenLoad::PackedMesh& packedMesh);
static void transferVertices(SPtr<MeshData> target, const Vector<StaticMeshVertex>& vertices);
static void transferIndices(SPtr<MeshData> target, const ZenLoad::PackedMesh& packedMesh);
static Vector<HMaterial> materialsFromStaticMesh(const ZenLoad::PackedMesh& packedMesh,
                                                 const VDFS::FileIndex& vdfs);

// - Implementation --------------------------------------------------------------------------------


bool BsZenLib::HasCachedStaticMeshPrefab(const bs::String& virtualFilePath)
{
	return FileSystem::isFile(GothicPathToCachedAsset(virtualFilePath.c_str()));
}

bs::HPrefab BsZenLib::LoadCachedStaticMeshPrefab(const bs::String& virtualFilePath)
{
	return gResources().load<Prefab>(GothicPathToCachedAsset(virtualFilePath));
}

bs::HPrefab BsZenLib::ImportAndCacheStaticMeshPrefab(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs)
{
	HMesh mesh = ImportAndCacheStaticMesh(virtualFilePath, vdfs);

	if (!mesh)
		return {};

	Vector<HMaterial> materials = ImportAndCacheStaticMeshMaterials(virtualFilePath, vdfs);

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

	return cacheStaticMesh(virtualFilePath, mesh, materials);
}


HPrefab BsZenLib::ImportAndCacheStaticMeshPrefab(const bs::String& name, const ZenLoad::PackedMesh& packedMesh, const VDFS::FileIndex& vdfs)
{
	HMesh mesh = ImportAndCacheStaticMesh(name, packedMesh, vdfs);

	if (!mesh)
		return {};

	Vector<HMaterial> materials = ImportAndCacheStaticMeshMaterials(name, packedMesh, vdfs);

	if (!mesh)
	{
		gDebug().logWarning("Load Failed (Mesh): " + name);
		return {};
	}

	if (materials.empty())
	{
		gDebug().logWarning("Load Failed (Materials): " + name);
		return {};
	}

	return cacheStaticMesh(name, mesh, materials);
}

static HPrefab cacheStaticMesh(const bs::String& virtualFilePath, HMesh mesh, const Vector<HMaterial>& materials)
{
	using namespace BsZenLib;

	HSceneObject so = SceneObject::create(virtualFilePath.c_str());

	HRenderable renderable = so->addComponent<CRenderable>();
	renderable->setMesh(mesh);
	renderable->setMaterials(materials);

	HPrefab prefab = Prefab::create(so, false);
	
	so->destroy(true);

	const bool overwrite = false;
	gResources().save(prefab, GothicPathToCachedAsset(virtualFilePath), overwrite);

	return prefab;
}



bs::HMesh BsZenLib::LoadCachedStaticMesh(const bs::String& virtualFilePath)
{
	Path path = GothicPathToCachedAsset(virtualFilePath + ".mesh");

	return gResources().load<Mesh>(path);
}

HMesh BsZenLib::ImportAndCacheStaticMesh(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs)
{
	ZenLoad::zCProgMeshProto progMesh(virtualFilePath.c_str(), vdfs);

	if (progMesh.getNumSubmeshes() == 0) return {};

	ZenLoad::PackedMesh packedMesh;
	progMesh.packMesh(packedMesh, 0.01f);

	return ImportAndCacheStaticMesh(virtualFilePath, packedMesh, vdfs);
}

bs::HMesh BsZenLib::ImportAndCacheStaticMesh(const bs::String& virtualFilePath, const ZenLoad::PackedMesh& packedMesh, const VDFS::FileIndex& vdfs)
{
	HMesh mesh = ImportStaticMesh(packedMesh);
	Path path = GothicPathToCachedAsset(virtualFilePath + ".mesh");

	if (!mesh)
		return {};

	const bool overwrite = true;
	gResources().save(mesh, path, overwrite);

	return mesh;
}

Vector<HMaterial> BsZenLib::ImportAndCacheStaticMeshMaterials(const bs::String& virtualFilePath, const VDFS::FileIndex& vdfs)
{
	ZenLoad::zCProgMeshProto progMesh(virtualFilePath.c_str(), vdfs);

	if (progMesh.getNumSubmeshes() == 0) return {};

	ZenLoad::PackedMesh packedMesh;
	progMesh.packMesh(packedMesh, 0.01f);

	return ImportAndCacheStaticMeshMaterials(virtualFilePath, packedMesh, vdfs);
}

bs::Vector<bs::HMaterial> BsZenLib::ImportAndCacheStaticMeshMaterials(const bs::String& virtualFilePath, const ZenLoad::PackedMesh& packedMesh, const VDFS::FileIndex& vdfs)
{
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

		HMaterial loaded = m; // FIXME: Doesn't load again?! gResources().load<Material>(materialPath);
		materials.push_back(loaded);
	}

	return materials;
}

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
  vertexDataDesc->addVertElem(VET_FLOAT3, VES_TANGENT);
  vertexDataDesc->addVertElem(VET_FLOAT3, VES_BITANGENT);

  assert(vertexDataDesc->getVertexStride() == sizeof(ZenLoad::WorldVertex));

  return vertexDataDesc;
}

static void fillMeshDataFromPackedMesh(HMesh target, const Vector<StaticMeshVertex>& vertices, const ZenLoad::PackedMesh& packedMesh)
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
  assert(target->getNumVertices() == packedMesh.vertices.size());

  UINT8* pVertices = target->getElementData(VES_POSITION);
  memcpy(pVertices, vertices.data(),
         sizeof(StaticMeshVertex) * vertices.size());
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
