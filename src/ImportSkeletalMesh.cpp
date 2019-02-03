#include "ImportSkeletalMesh.hpp"
#include <optional>
#include "ImportMaterial.hpp"
#include "ImportPath.hpp"
#include <Animation/BsSkeleton.h>
#include <Components/BsCRenderable.h>
#include <FileSystem/BsFileSystem.h>
#include <RenderAPI/BsVertexDataDesc.h>
#include <Resources/BsBuiltinResources.h>
#include <Resources/BsResourceManifest.h>
#include <Resources/BsResources.h>
#include <Scene/BsPrefab.h>
#include <Scene/BsSceneObject.h>
#include <vdfs/fileIndex.h>
#include <zenload/modelScriptParser.h>
#include <zenload/zCModelMeshLib.h>
#include <zenload/zTypes.h>
#include <zenload/zenParser.h>
#include "ImportAnimation.hpp"

using namespace bs;
using namespace BsZenLib;
using namespace BsZenLib::Res;

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


// - Implementation --------------------------------------------------------------------------------

/**
 * Loads a mesh used with skeletal animation, stored inside .MDL or .MDM-files.
 */
class SkeletalMeshGeometryLoader
{
public:
  SkeletalMeshGeometryLoader(const String& mdlFile, Vector<Matrix4> bindPose,
                             SPtr<Skeleton> skeleton, const VDFS::FileIndex& vdfs)
      : mMdlFile(mdlFile)
      , mVDFS(vdfs)
      , mBindPose(bindPose)
      , mSkeleton(skeleton)
  {
    if (!loadMeshSkin())
    {
      BS_EXCEPT(InternalErrorException, "Could not load model skin: " + mdlFile);
    }

    packMesh();
    importAndCacheGeometry();
    importAndCacheSkeletalMeshMaterials();
  }

  HMesh getImportedMesh() const { return mImportedMesh; }
  Vector<HMaterial> getImportedMaterials() const { return mImportedMeshMaterials; }

private:
  /**
   * Loads the vertex data from the .MDL or MDM file into memory.
   */
  bool loadMeshSkin()
  {
    mMeshSkin = ZenLoad::zCModelMeshLib(mMdlFile.c_str(), mVDFS);

    return mMeshSkin.isValid();
  }

  void packMesh() { mMeshSkin.packMesh(mPackedMesh /*, 0.01f */); }

  void importAndCacheGeometry()
  {
    MESH_DESC desc = meshDescForPackedMesh();

    if (desc.numIndices == 0)
    {
      BS_EXCEPT(InternalErrorException, "Skeletal Mesh cannot have 0 indices!");
    }

    desc.skeleton = mSkeleton;

    HMesh mesh = Mesh::create(desc);

    mesh->setName(mMdlFile);

    Vector<SkeletalVertex> vertices = transformVertices();

    fillMeshDataFromPackedMesh(mesh, vertices);

    mImportedMesh = mesh;

    const bool overwrite = false;
    gResources().save(mesh, GothicPathToCachedSkeletalMesh(mMdlFile + "-geometry"), overwrite);
  }

  void importAndCacheSkeletalMeshMaterials()
  {
    for (size_t i = 0; i < mPackedMesh.subMeshes.size(); i++)
    {
      // TODO: Transfer more properties
      const auto& originalMaterial = mPackedMesh.subMeshes[i].material;

      HMaterial imported;

      String materialCacheFile = BuildMaterialNameForSubmesh(mMdlFile, (UINT32)i);

      if (HasCachedMaterial(materialCacheFile))
      {
        imported = LoadCachedMaterial(materialCacheFile);
      }
      else
      {
        imported = ImportAndCacheMaterialWithTextures(materialCacheFile, originalMaterial, mVDFS);
      }

      mImportedMeshMaterials.push_back(imported);
    }
  }

  /**
   * Converts the vertices from ZenLib into a format for bs::f.
   */
  Vector<SkeletalVertex> transformVertices()
  {
    Vector<SkeletalVertex> v;

    for (const ZenLoad::SkeletalVertex& oldVertex : mPackedMesh.vertices)
    {
      SkeletalVertex newVertex = {};

      newVertex.position = transformToBindPose(oldVertex);
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

  Vector3 transformToBindPose(const ZenLoad::SkeletalVertex& vertex)
  {
    Vector3 transformed = Vector3(0.0f, 0.0f, 0.0f);

    for (size_t i = 0; i < 4; i++)
    {
      Vector3 localPosition = Vector3(vertex.LocalPositions[i].x, vertex.LocalPositions[i].y,
                                      vertex.LocalPositions[i].z);

      Vector4 tmp = mBindPose[vertex.BoneIndices[i]].multiply(Vector4(localPosition, 1.0f));
      transformed += Vector3(tmp) * vertex.Weights[i];
    }

    return transformed;
  }

  MESH_DESC meshDescForPackedMesh()
  {
    MESH_DESC desc = {};

    for (const auto& submesh : mPackedMesh.subMeshes)
    {
      desc.subMeshes.emplace_back();
      desc.subMeshes.back().drawOp = DrawOperationType::DOT_TRIANGLE_LIST;
      desc.subMeshes.back().indexCount = (UINT32)submesh.indices.size();
      desc.subMeshes.back().indexOffset = desc.numIndices;

      desc.numIndices += (UINT32)submesh.indices.size();
    }

    desc.indexType = IndexType::IT_32BIT;
    desc.numVertices = (UINT32)mPackedMesh.vertices.size();

    desc.vertexDesc = makeVertexDataDescForZenLibVertex();
    desc.usage = MU_CPUCACHED;  // To create our physics mesh later

    return desc;
  }

  SPtr<VertexDataDesc> makeVertexDataDescForZenLibVertex()
  {
    SPtr<VertexDataDesc> vertexDataDesc = VertexDataDesc::create();
    vertexDataDesc->addVertElem(VET_FLOAT3, VES_POSITION);
    vertexDataDesc->addVertElem(VET_FLOAT3, VES_NORMAL);
    vertexDataDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);
    vertexDataDesc->addVertElem(VET_FLOAT3, VES_TANGENT);
    vertexDataDesc->addVertElem(VET_FLOAT3, VES_BITANGENT);
    vertexDataDesc->addVertElem(VET_UBYTE4, VES_BLEND_INDICES);
    vertexDataDesc->addVertElem(VET_FLOAT4, VES_BLEND_WEIGHTS);
    // vertexDataDesc->addVertElem(VET_UBYTE4, VES_TEXCOORD, 1);
    // vertexDataDesc->addVertElem(VET_FLOAT4, VES_TEXCOORD, 2);

    assert(vertexDataDesc->getVertexStride() == sizeof(SkeletalVertex));

    return vertexDataDesc;
  }

  void fillMeshDataFromPackedMesh(HMesh target, const Vector<SkeletalVertex>& vertices)
  {
    // Allocate a buffer big enough to hold what we specified in the MESH_DESC
    SPtr<MeshData> meshData = target->allocBuffer();

    transferVertices(meshData, vertices);
    transferIndices(meshData);

    target->writeData(meshData, false);
  }

  void transferVertices(SPtr<MeshData> target, const Vector<SkeletalVertex>& vertices)
  {
    assert(target->getNumVertices() == vertices.size());

    UINT8* pVertices = target->getElementData(VES_POSITION);
    memcpy(pVertices, vertices.data(), sizeof(SkeletalVertex) * vertices.size());
  }

  void transferIndices(SPtr<MeshData> target)
  {
    UINT32* pIndices = target->getIndices32();
    size_t writtenSoFar = 0;

    for (const auto& submesh : mPackedMesh.subMeshes)
    {
      memcpy(&pIndices[writtenSoFar], submesh.indices.data(),
             sizeof(UINT32) * submesh.indices.size());
      writtenSoFar += submesh.indices.size();
    }
  }
  SPtr<Skeleton> mSkeleton;
  Vector<Matrix4> mBindPose;
  String mMdlFile;
  const VDFS::FileIndex& mVDFS;
  ZenLoad::zCModelMeshLib mMeshSkin;
  ZenLoad::PackedSkeletalMesh mPackedMesh;
  HMesh mImportedMesh;
  Vector<HMaterial> mImportedMeshMaterials;
};

/**
 * Import a model script file and all of its dependencies.
 *
 * A ModelScriptFile can come in two different variations: ASCII and Binary.
 * While Gothic I uses only ASCII files, most if not all of those got coverted into a
 * binary format for Gothic II.
 *
 * Both variants contain the same information:
 *
 *  - Which node hierarchy (bones) file to use
 *  - A list of supported meshes
 *  - Definitions for each animation to be played
 *
 * To load all of this, we need to go through several stages:
 *
 *  1. Load the definitions stored inside the model script
 *  2. Load the node hierarchy
 *  3. Generate the Bind-Pose Node tree
 *  4. Load all supported meshes
 *
 * With the Bind-Pose in place, we can further convert the meshes from the game into a format
 * bs::f understands: At some time during skeletal animation, the vertices need to be in
 * node-local space. PiranhaBytes decided to store their meshes vertices in that space to save
 * as step when animating. This implies that the mesh cannot be used without an animation.
 *
 * To convert it to a "normal" looking mesh again, one can just transform each vertex with
 * the Bind-Pose Node tree.
 */
class ModelScriptFileImporter
{
public:
  ModelScriptFileImporter(const bs::String& modelScriptFile, const VDFS::FileIndex& vdfs)
      : mModelScriptFile(modelScriptFile)
      , mVDFS(vdfs)
  {
    useMsbFileIfPossible();

    mHierarchyFile = getHierarchyFile();

    if (!loadModelScript())
    {
      BS_EXCEPT(InternalErrorException, "Could not load model script: " + modelScriptFile);
    }

    if (!loadHierarchy())
    {
      BS_EXCEPT(InternalErrorException, "Could not load model hierarchy: " + getHierarchyFile());
    }

    mBindPose = makeBindPose();
    mSkeleton = makeSkeleton();

    for (const auto& mesh : mModelScriptParser->meshesASC())
    {
      String meshFile = findMatchingMeshFile(mesh.c_str());

      if (!meshFile.empty())
      {
        SkeletalMeshGeometryLoader loader(meshFile, mBindPose, mSkeleton, mVDFS);

        HMeshWithMaterials imported =
            MeshWithMaterials::create(loader.getImportedMesh(), loader.getImportedMaterials());

        const bool overwrite = false;
        gResources().save(imported, GothicPathToCachedSkeletalMesh(meshFile), overwrite);

        mMeshes.push_back(imported);
      }
    }

    for (const auto& ani : mAnimationsToImport)
    {
      HAnimationClip clip = ImportMAN(mMeshHierarchy, ani, mVDFS);

      mAnimationClips.push_back(clip);
    }
  }

  Vector<HMeshWithMaterials> getMeshes() const { return mMeshes; }

  Vector<HAnimationClip> getAnimations() const
  {
    return mAnimationClips;
  }

private:
  /**
   * Removes the extension from a given file name or path.
   *
   * Given a string of "sample.ext", this will return "sample".
   */
  String stripExtension(const String& in) const { return in.substr(0, in.length() - 4); }

  /**
   * Strips the extension from the model script file and returns the part
   * without the extension (and without the ".")
   */
  String getModelScriptName() const { return stripExtension(mModelScriptFile); }

  /**
   * Uses the supplied model script file (MDS, MSB) to get the matching hierarchy file (MDH).
   *
   * The matching hierarchy file will have the same name as the model script, just with a
   * .MDH extension instead.
   */
  String getHierarchyFile() const { return getModelScriptName() + ".MDH"; }

  /**
   * Tries to find a .MDL or .MDM file which matches the given .ASC-Name.
   *
   * Some meshes are embedded inside a .MDL-file, which combines hierarchy and
   * mesh skin but not all do. For instance, the Humans don't have one and have to have their
   * hierarchy and mesh loaded seperatly as .MDH and .MDM-files.
   *
   * @param meshASC String like "SomeMesh.ASC".
   *
   * @return Name of the matching mesh file or empty string if none was found.
   */
  String findMatchingMeshFile(const String& meshASC) const
  {
    String asMDL = stripExtension(meshASC) + ".MDL";
    String asMDM = stripExtension(meshASC) + ".MDM";

    if (mVDFS.hasFile(asMDL.c_str())) return asMDL;
    if (mVDFS.hasFile(asMDM.c_str())) return asMDM;

    return "";
  }

  /**
   * Loads the node hierarchy for the model script.
   * This includes a tree of all nodes, their names and local positions.
   */
  bool loadHierarchy()
  {
    mMeshHierarchy = ZenLoad::zCModelMeshLib(getHierarchyFile().c_str(), mVDFS);

    return mMeshHierarchy.isValid();
  }

  void useMsbFileIfPossible()
  {
    String msb = stripExtension(mModelScriptFile) + ".MSB";

    if (mVDFS.hasFile(msb.c_str()))
    {
      mModelScriptFile = msb;
    }
    else
    {
      // keep as it is
    }
  }

  /**
   * Loads the model script containing all further information like supported meshes
   * and animations.
   */
  bool loadModelScript()
  {
    using namespace ZenLoad;

    ZenParser zen(mModelScriptFile.c_str(), mVDFS);

    if (zen.getFileSize() == 0)
      return false;

    mModelScriptParser = bs_unique_ptr_new<ZenLoad::ModelScriptBinParser>(zen);

    ModelScriptParser& p = *mModelScriptParser;

    ModelScriptParser::EChunkType type;
    while ((type = p.parse()) != ModelScriptParser::CHUNK_EOF)
    {
      switch (type)
      {
        case ModelScriptParser::CHUNK_ANI:
        {
          String qname = getModelScriptName() + '-' + p.ani().m_Name.c_str();

          AnimationToImport import = {};
          import.fullAnimationName = qname;
          import.animation = p.ani();

          // In case this was an ASCII-File, these will be filled. Binary files have single ones
          // stored in chunks handled below
          for (auto& sfx : p.sfx())
          {
            import.eventsSfx.push_back(sfx);
          }
          p.sfx().clear();

          for (auto& sfx : p.sfxGround())
          {
            import.eventsSfxGround.push_back(sfx);
          }
          p.sfxGround().clear();

          for (auto& tag : p.tag())
          {
            import.eventsTag.push_back(tag);
          }
          p.tag().clear();
          for (auto& pfx : p.pfx())
          {
            import.eventsPfx.push_back(pfx);
          }
          p.pfx().clear();
          for (auto& pfxStop : p.pfxStop())
          {
            import.eventsPfxStop.push_back(pfxStop);
          }
          p.pfxStop().clear();

          mAnimationsToImport.push_back(import);
        }
        break;

        // This will be only called on binary files, with exactly one sfx-entry!
        case ModelScriptParser::CHUNK_EVENT_SFX:
        {
          // animations.back().eventsSfx.push_back(p.sfx().back());
          // p.sfx().clear();
        }
        break;

        // This will be only called on binary files, with exactly one sfx-entry!
        case ModelScriptParser::CHUNK_EVENT_SFX_GRND:
        {
          // animations.back().eventsSfxGround.push_back(p.sfxGround().back());
          // p.sfxGround().clear();
        }
        break;

        case ModelScriptParser::CHUNK_EVENT_PFX:
        {
          // animations.back().eventsPfx.push_back(p.pfx().back());
          // p.pfx().clear();
        }
        break;

        case ModelScriptParser::CHUNK_EVENT_PFX_STOP:
        {
          // animations.back().eventsPfxStop.push_back(p.pfxStop().back());
          // p.pfxStop().clear();
        }
        break;

        case ModelScriptParser::CHUNK_ERROR:
          assert(false);
          return false;
      }
    }

    return true;
  }

  /**
   * Calculates the bind-pose from the hierarchy stored inside the ModelMeshLib.
   */
  Vector<Matrix4> makeBindPose()
  {
    const auto& nodes = mMeshHierarchy.getNodes();
    const auto numNodes = nodes.size();

    Vector<Matrix4> bindPose(numNodes);
    Vector<Matrix4> nodeTransforms;

    for (const auto& node : nodes)
    {
      nodeTransforms.push_back(convertMatrix(node.transformLocal));
    }

    // Calculate actual node matrices
    for (auto i = 0; i < numNodes; i++)
    {
      // TODO: There is a flag indicating whether the animation root should translate the vob
      // position Move root node to (0,0,0)
      if (i == 0)
      {
        Vector3 position;
        Quaternion rotation;
        Vector3 scale;

        nodeTransforms[i].decomposition(position, rotation, scale);

        // scale *= 0.01;  // Scale centimeters -> meters
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

  Matrix4 convertMatrix(const ZMath::Matrix& m)
  {
    Matrix4 bs = {m.mv[0], m.mv[1], m.mv[2],  m.mv[3],  m.mv[4],  m.mv[5],  m.mv[6],  m.mv[7],
                  m.mv[8], m.mv[9], m.mv[10], m.mv[11], m.mv[12], m.mv[13], m.mv[14], m.mv[15]};

    return bs.transpose();
  }

  /**
   * Generates a bs::f skeleton from the loaded hierarchy.
   */
  bs::SPtr<bs::Skeleton> makeSkeleton()
  {
    Vector<BONE_DESC> bones;

    for (size_t i = 0; i < mMeshHierarchy.getNodes().size(); i++)
    {
      const ZenLoad::ModelNode& node = mMeshHierarchy.getNodes()[i];

      bones.emplace_back();
      bones.back().name = node.name.c_str();
      bones.back().parent = node.parentValid() ? node.parentIndex : UINT32_MAX;
      bones.back().invBindPose = mBindPose[i].inverse();

      Vector3 position;
      Quaternion rotation;
      Vector3 scale;

      convertMatrix(node.transformLocal).decomposition(position, rotation, scale);

      // position *= 0.01f;  // Scale centimeters -> meters

      bones.back().localTfrm = Transform(position, rotation, scale);
    }

    return Skeleton::create(&bones[0], (UINT32)bones.size());
  }

  String mModelScriptFile;
  String mHierarchyFile;
  Vector<String> mAnimationFiles;
  Vector<AnimationToImport> mAnimationsToImport;
  Vector<HAnimationClip> mAnimationClips;
  Vector<HMeshWithMaterials> mMeshes;
  bs::SPtr<bs::Skeleton> mSkeleton;
  const VDFS::FileIndex& mVDFS;
  ZenLoad::zCModelMeshLib mMeshHierarchy;
  bs::Vector<bs::Matrix4> mBindPose;
  SPtr<ZenLoad::ModelScriptParser> mModelScriptParser;
};

HModelScriptFile BsZenLib::ImportAndCacheMDS(const bs::String& mdsFile, const VDFS::FileIndex& vdfs)
{
  ModelScriptFileImporter importer(mdsFile, vdfs);

  HModelScriptFile mds = ModelScriptFile::create(importer.getMeshes(), importer.getAnimations());

  const bool overwrite = false;
  gResources().save(mds, GothicPathToCachedModelScript(mdsFile), overwrite);

  return mds;
}

bool BsZenLib::HasCachedMDS(const bs::String& mdsFile)
{
  return FileSystem::isFile(GothicPathToCachedModelScript(mdsFile));
}

HModelScriptFile BsZenLib::LoadCachedMDS(const bs::String& mdsFile)
{
  return gResources().load<ModelScriptFile>(GothicPathToCachedModelScript(mdsFile));
}
