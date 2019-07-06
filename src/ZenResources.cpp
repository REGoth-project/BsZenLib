#include "ZenResources.hpp"
#include <Mesh/BsMesh.h>
#include <Resources/BsResources.h>

using namespace BsZenLib::Res;

bs::RTTITypeBase* ZAnimationClip::getRTTIStatic() { return ZAnimationClipRTTI::instance(); }

bs::SPtr<ZAnimationClip> ZAnimationClip::createEmpty()
{
  using namespace bs;

  SPtr<ZAnimationClip> sptr = bs_core_ptr<ZAnimationClip>(new (bs_alloc<ZAnimationClip>()) ZAnimationClip());
  sptr->_setThisPtr(sptr);

  return sptr;
}

HZAnimation ZAnimationClip::create()
{
  using namespace bs;

  SPtr<ZAnimationClip> sptr = createEmpty();
  sptr->initialize();

  // Create a handle
  return static_resource_cast<ZAnimationClip>(bs::gResources()._createResourceHandle(sptr));
}

HModelScriptFile ModelScriptFile::create(bs::Vector<HMeshWithMaterials> meshes,
                                         bs::Vector<HZAnimation> animations)
{
  HModelScriptFile h = create();
  h->mMeshes = meshes;
  h->mAnimations = animations;

  // Needs to be also called after deserialization!
  h->_buildMeshesByNameMap();

  return h;
}

void ModelScriptFile::_buildMeshesByNameMap()
{
  // These are set again
  for (auto& m : mMeshes)
  {
    if (!m || !m.isLoaded())
    {
      bs::gDebug().logWarning("[ZenResources] Empty mesh found while loading ModelScript " +
                              getName());
      continue;
    }

    bs::String nameUpperCase = m->getName();
    bs::StringUtil::toUpperCase(nameUpperCase);

    bs::String noExtension = nameUpperCase.substr(0, nameUpperCase.find_first_of('.'));

    mMeshesByName[noExtension] = m;
  }
}

void ModelScriptFile::getResourceDependencies(bs::FrameVector<bs::HResource>& dependencies) const
{
  for (auto mesh : mMeshes)
  {
    dependencies.push_back(mesh);
  }

  for (auto clip : mAnimations)
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

bs::SPtr<ModelScriptFile> ModelScriptFile::createEmpty()
{
  using namespace bs;

  SPtr<ModelScriptFile> sptr =
      bs_core_ptr<ModelScriptFile>(new (bs_alloc<ModelScriptFile>()) ModelScriptFile());
  sptr->_setThisPtr(sptr);

  return sptr;
}

bs::RTTITypeBase* ModelScriptFile::getRTTIStatic() { return ModelScriptFileRTTI::instance(); }

HMeshWithMaterials MeshWithMaterials::create(bs::HMesh mesh, bs::Vector<bs::HMaterial> materials)
{
  HMeshWithMaterials h = create();
  h->mMesh = mesh;
  h->mMaterials = materials;
  h->setName(mesh->getName());

  return h;
}
HMeshWithMaterials MeshWithMaterials::create(bs::HMesh mesh, bs::Vector<bs::HMaterial> materials,
                                             bs::Map<bs::String, HMeshWithMaterials> attachments)
{
  HMeshWithMaterials h = create(mesh, materials);

  for (const auto& a : attachments)
  {
    h->mAttachmentNodeNames.push_back(a.first);
    h->mNodeAttachments.push_back(a.second);
  }

  return h;
}

bs::SPtr<MeshWithMaterials> MeshWithMaterials::createEmpty()
{
  using namespace bs;

  SPtr<MeshWithMaterials> sptr =
      bs_core_ptr<MeshWithMaterials>(new (bs_alloc<MeshWithMaterials>()) MeshWithMaterials());
  sptr->_setThisPtr(sptr);

  return sptr;
}

void MeshWithMaterials::getResourceDependencies(bs::FrameVector<bs::HResource>& dependencies) const
{
  dependencies.push_back(mMesh);

  for (auto material : mMaterials)
  {
    dependencies.push_back(material);
  }

  for (auto mesh : mNodeAttachments)
  {
    dependencies.push_back(mesh);
  }
}

HMeshWithMaterials MeshWithMaterials::create()
{
  using namespace bs;

  SPtr<MeshWithMaterials> sptr = bs_core_ptr<MeshWithMaterials>(bs_new<MeshWithMaterials>());
  sptr->_setThisPtr(sptr);
  sptr->initialize();

  // Create a handle
  return static_resource_cast<MeshWithMaterials>(bs::gResources()._createResourceHandle(sptr));
}

bs::RTTITypeBase* MeshWithMaterials::getRTTIStatic()
{
  return MeshWithMaterialsRTTI::instance();
}
