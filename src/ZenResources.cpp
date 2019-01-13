#include "ZenResources.hpp"
#include <Resources/BsResources.h>
#include <Mesh/BsMesh.h>

using namespace BsZenLib::Res;

HModelScriptFile ModelScriptFile::create(bs::Vector<HMeshWithMaterials> meshes, bs::Vector<bs::HAnimationClip> clips)
{
  HModelScriptFile h = create();
  h->mMeshes = meshes;
  h->mAnimationClips = clips;

  return h;
}

void ModelScriptFile::getResourceDependencies(
    bs::FrameVector<bs::HResource>& dependencies) const
{
  for (auto mesh : mMeshes)
  {
    dependencies.push_back(mesh);
  }

  for (auto clip : mAnimationClips)
  {
    dependencies.push_back(clip);
  }
}

HModelScriptFile ModelScriptFile::create()
{
  using namespace bs;

  SPtr<ModelScriptFile> sptr = bs_core_ptr<ModelScriptFile>(bs_new<ModelScriptFile>());
  sptr->_setThisPtr(sptr);
  sptr->initialize();

  // Create a handle
  return static_resource_cast<ModelScriptFile>(bs::gResources()._createResourceHandle(sptr));
}

bs::RTTITypeBase* ModelScriptFile::getRTTIStatic() { return bs::AnimatedMeshRTTI::instance(); }

HMeshWithMaterials MeshWithMaterials::create(bs::HMesh mesh, bs::Vector<bs::HMaterial> materials)
{
  HMeshWithMaterials h = create();
  h->mMesh = mesh;
  h->mMaterials = materials;
  h->setName(mesh->getName());

  return h;
}

void MeshWithMaterials::getResourceDependencies(bs::FrameVector<bs::HResource>& dependencies) const
{
  dependencies.push_back(mMesh);

  for (auto material : mMaterials)
  {
    dependencies.push_back(material);
  }
}

HMeshWithMaterials MeshWithMaterials::create()
{
  using namespace bs;

  SPtr<MeshWithMaterials> sptr = bs_core_ptr<MeshWithMaterials>(bs_new<MeshWithMaterials>());
  sptr->_setThisPtr(sptr);
  sptr->initialize();

  // Create a handle
  return static_resource_cast<MeshWithMaterials>(gResources()._createResourceHandle(sptr));
}

bs::RTTITypeBase* MeshWithMaterials::getRTTIStatic()
{
  return bs::MeshWithMaterialsRTTI::instance();
}