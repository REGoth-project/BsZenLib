#pragma once
#include <BsCorePrerequisites.h>
#include <Reflection/BsRTTIType.h>
#include <Resources/BsResource.h>

namespace bs
{
  class AnimatedMeshRTTI;
  class MeshWithMaterialsRTTI;

  /**
   * Type IDs for RTTI (Asset saving)
   */
  enum ZenResourcesTypeIds
  {
    TID_AnimatedMesh = 400001,
    TID_MeshWithMaterials = 400002
  };
}  // namespace bs

namespace BsZenLib
{
  namespace Res
  {
    class MeshWithMaterials;
    class ModelScriptFile;

    typedef bs::ResourceHandle<MeshWithMaterials> HMeshWithMaterials;
    typedef bs::ResourceHandle<ModelScriptFile> HModelScriptFile;

    /**
     * Container which combines a mesh with a list of materials it shall use.
     *
     * In Gothic the Materials a mesh uses are stored inside the meshes file,
     * so they are strictly coupled with their mesh. It makes no sense to share
     * the materials between different meshes.
     *
     * bs::f however brings the mesh and the list of materials together inside
     * the Renderable-component, which cannot be saved as a resource.
     *
     * This custom Resource brings the ability to store which mesh and materials
     * belong together.
     *
     * Luckily, in bs::f the Mesh-Class allows the mesh to be either static
     * or to be used with skeletal/morph animations, so we can just use this
     * container class for both.
     */
    class MeshWithMaterials : public bs::Resource
    {
    public:
      /**
       * Create from a Mesh and a list of Materials
       */
      static HMeshWithMaterials create(bs::HMesh mesh, bs::Vector<bs::HMaterial> materials);

      /**
       * @return The imported mesh
       */
      bs::HMesh getMesh() const { return mMesh; }

      /**
       * @return The imported meshes materials
       */
      bs::Vector<bs::HMaterial> getMaterials() const { return mMaterials; }

      /** @copydoc Resource::getResourceDependencies */
      void getResourceDependencies(bs::FrameVector<bs::HResource>& dependencies) const override;

    private:
      /**
       * Create an empty resource
       */
      static HMeshWithMaterials create();

    public:
      MeshWithMaterials()
          : bs::Resource(/*requiresGpuInit*/ false)
      {
      }

      friend class bs::MeshWithMaterialsRTTI;
      static bs::RTTITypeBase* getRTTIStatic();
      bs::RTTITypeBase* getRTTI() const override { return getRTTIStatic(); }

    private:
      bs::HMesh mMesh;
      bs::Vector<bs::HMaterial> mMaterials;
    };

    /**
     * Container for a model animation script.
     *
     * A model animation script defines which animations a model can play
     * along with the events which happen along those animations, ie. footstep
     * sounds or particle effects.
     *
     * This custom resource contains a mesh and all animations that go with it.
     */
    class ModelScriptFile : public bs::Resource
    {
    public:
      /**
       * Create from a Mesh and a list of Materials
       */
      static HModelScriptFile create(bs::Vector<HMeshWithMaterials> meshes,
                                     bs::Vector<bs::HAnimationClip> clips);

      /**
       * @return The imported mesh
       */
      bs::Vector<HMeshWithMaterials> getMeshes() const { return mMeshes; }

      /**
       * @return The imported animation clips for this mesh
       */
      bs::Vector<bs::HAnimationClip> getAnimationClips() const { return mAnimationClips; }

      /**
       * Looks up the mesh with the given name.
       *
       * @param  name  Name of the mesh (*not* the filename!), UPPERCASE, without extension,
       *               for example: `DRAGON_FIRE_BODY`
       *
       * @return Handle of the found mesh. Invalid handle if the given mesh was not found.
       */
      HMeshWithMaterials getMeshByName(const bs::String& name)
      {
        auto it = mMeshesByName.find(name);

        if (it == mMeshesByName.end())
        {
          return {};
        }
        else
        {
          return it->second;
        }
      }

      /** @copydoc Resource::getResourceDependencies */
      void getResourceDependencies(bs::FrameVector<bs::HResource>& dependencies) const override;

    private:
      /**
       * Create an empty resource
       */
      static HModelScriptFile create();

    public:
      ModelScriptFile()
          : bs::Resource(/*requiresGpuInit*/ false)
      {
      }

      friend class bs::AnimatedMeshRTTI;
      static bs::RTTITypeBase* getRTTIStatic();
      bs::RTTITypeBase* getRTTI() const override { return getRTTIStatic(); }
      void _buildMeshesByNameMap();

    private:
      bs::Vector<HMeshWithMaterials> mMeshes;
      bs::Map<bs::String, HMeshWithMaterials> mMeshesByName;
      bs::Vector<bs::HAnimationClip> mAnimationClips;
    };

  }  // namespace Res
}  // namespace BsZenLib

// Need to extend namespace bs because of the RTTI-makros
namespace bs
{
  class AnimatedMeshRTTI
      : public bs::RTTIType<BsZenLib::Res::ModelScriptFile, Resource, AnimatedMeshRTTI>
  {
  public:
    BS_BEGIN_RTTI_MEMBERS
    BS_RTTI_MEMBER_REFL_ARRAY(mMeshes, 0)
    BS_RTTI_MEMBER_REFL_ARRAY(mAnimationClips, 1)
    BS_END_RTTI_MEMBERS

    void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
    {
      auto* modelScript = static_cast<BsZenLib::Res::ModelScriptFile*>(obj);

      modelScript->_buildMeshesByNameMap();
    }

    const String& getRTTIName() override
    {
      static String name = "ModelScriptFile";
      return name;
    }

    UINT32 getRTTIId() override { return TID_AnimatedMesh; }

    SPtr<IReflectable> newRTTIObject() override
    {
      return bs_shared_ptr_new<BsZenLib::Res::ModelScriptFile>();
    }
  };

  class MeshWithMaterialsRTTI
      : public RTTIType<BsZenLib::Res::MeshWithMaterials, Resource, MeshWithMaterialsRTTI>
  {
  public:
    BS_BEGIN_RTTI_MEMBERS
    BS_RTTI_MEMBER_REFL(mMesh, 0)
    BS_RTTI_MEMBER_REFL_ARRAY(mMaterials, 1)
    BS_END_RTTI_MEMBERS

    const String& getRTTIName() override
    {
      static String name = "MeshWithMaterials";
      return name;
    }

    UINT32 getRTTIId() override { return TID_MeshWithMaterials; }

    SPtr<IReflectable> newRTTIObject() override
    {
      return bs_shared_ptr_new<BsZenLib::Res::MeshWithMaterials>();
    }
  };

}  // namespace bs
