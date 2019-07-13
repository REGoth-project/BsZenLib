#pragma once
#include <BsCorePrerequisites.h>
#include <Reflection/BsRTTIType.h>
#include <Resources/BsResource.h>

namespace BsZenLib
{
  namespace Res
  {
    /**
     * Type IDs for RTTI (Asset saving)
     */
    enum ZenResourcesTypeIds
    {
      TID_ModelScriptFile = 400001,
      TID_MeshWithMaterials = 400002,
      TID_ZAnimation = 400003,
    };

    class MeshWithMaterials;
    class MeshWithMaterialsRTTI;
    class ModelScriptFile;
    class ModelScriptFileRTTI;

    struct ZAnimationClip;
    class ZAnimationClipRTTI;

    typedef bs::ResourceHandle<MeshWithMaterials> HMeshWithMaterials;
    typedef bs::ResourceHandle<ModelScriptFile> HModelScriptFile;
    typedef bs::ResourceHandle<ZAnimationClip> HZAnimation;

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
      static HMeshWithMaterials create(bs::HMesh mesh, bs::Vector<bs::HMaterial> materials,
                                       bs::Map<bs::String, HMeshWithMaterials> attachments);

      /**
       * @return The imported mesh
       */
      bs::HMesh getMesh() const { return mMesh; }

      /**
       * @return The imported meshes materials
       */
      bs::Vector<bs::HMaterial> getMaterials() const { return mMaterials; }

      /**
       * @return Map of node names -> attached mesh
       */
      bs::Map<bs::String, HMeshWithMaterials> getNodeAttachments() const
      {
        bs::Map<bs::String, HMeshWithMaterials> map;

        assert(mAttachmentNodeNames.size() == mNodeAttachments.size());

        for (auto i = 0; i < mAttachmentNodeNames.size(); i++)
        {
          map[mAttachmentNodeNames[i]] = mNodeAttachments[i];
        }

        return map;
      }

    private:
      /**
       * Create an empty resource
       */
      static HMeshWithMaterials create();

      /**
       * Create empty object to be filled via RTTI.
       */
      static bs::SPtr<MeshWithMaterials> createEmpty();

    public:
      MeshWithMaterials()
          : bs::Resource(/*requiresGpuInit*/ false)
      {
      }

      friend class MeshWithMaterialsRTTI;
      static bs::RTTITypeBase* getRTTIStatic();
      bs::RTTITypeBase* getRTTI() const override { return getRTTIStatic(); }

    private:
      bs::HMesh mMesh;
      bs::Vector<bs::HMaterial> mMaterials;

      // This could be stored as a pair, but two vectors are easier to serialize.
      // At index i, both store the name of a node and the Mesh attached to that node.
      bs::Vector<bs::String> mAttachmentNodeNames;
      bs::Vector<HMeshWithMaterials> mNodeAttachments;
    };

    /**
     * Stores all parameters every animation will have, regardless of whether it's
     * a standard animation, blend or alias.
     */
    struct ZAnimationClip : public bs::Resource
    {
      /**
       * Create empty object to be filled via RTTI.
       */
      static HZAnimation create();

      bs::HAnimationClip mClip;

      /** Next animation to play. If this is empty, the mIsLooping-flag is
       *  checked. In the original game, if this were a looping animation,
       *  the name of this very animation would be put into mNext as well.
       *  However, looping works better if we use bsf's own mechanism for looping.
       */
      bs::String mNext;

      float mBlendIn = 0.0f;
      float mBlendOut = 0.0f;

      bs::UINT32 mLayer = 0;

      /** Animation moves model in world space */
      bool mShouldMoveModel = false;

      /** Animation rotates model in world space */
      bool mShouldRotateModel = false;

      /** Animation is queued after the current any on layer instead of started immediately */
      bool mShouldQueueIntoLayer = false;

      /** Don't stick to ground */
      bool mIsFlyingAnimation = false;

      /** Idle animation */
      bool mIsIdleAnimation = false;

      /** Whether this is a looping animation. Only evaluated if mNext is empty. */
      bool mIsLooping = false;

      enum class Direction
      {
        Forward,
        Reverse,
      };

      Direction mDirection = Direction::Forward;

    protected:
      ZAnimationClip()
          : bs::Resource(/*requiresGpuInit*/ false)
      {
      }

      /**
       * Create empty object to be filled via RTTI.
       */
      static bs::SPtr<ZAnimationClip> createEmpty();

      friend class ZAnimationClipRTTI;

    public:
      static bs::RTTITypeBase* getRTTIStatic();
      bs::RTTITypeBase* getRTTI() const override { return getRTTIStatic(); }
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
                                     bs::Vector<HZAnimation> animations);

      /**
       * @return The imported mesh
       */
      bs::Vector<HMeshWithMaterials> getMeshes() const { return mMeshes; }

      /**
       * @return The imported animation clips for this mesh
       */
      bs::Vector<HZAnimation> getAnimations() const { return mAnimations; }

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

    private:
      /**
       * Create an empty resource
       */
      static HModelScriptFile create();

      /**
       * Create empty object to be filled via RTTI.
       */
      static bs::SPtr<ModelScriptFile> createEmpty();

    public:
      ModelScriptFile()
          : bs::Resource(/*requiresGpuInit*/ false)
      {
      }

      friend class ModelScriptFileRTTI;
      static bs::RTTITypeBase* getRTTIStatic();
      bs::RTTITypeBase* getRTTI() const override { return getRTTIStatic(); }
      void _buildMeshesByNameMap();

    private:
      bs::Vector<HMeshWithMaterials> mMeshes;
      bs::Map<bs::String, HMeshWithMaterials> mMeshesByName;
      bs::Vector<HZAnimation> mAnimations;
    };

    class ZAnimationClipRTTI : public bs::RTTIType<BsZenLib::Res::ZAnimationClip, bs::Resource, ZAnimationClipRTTI>
    {
    public:
      BS_BEGIN_RTTI_MEMBERS
      BS_RTTI_MEMBER_REFL(mClip, 0)
      BS_RTTI_MEMBER_PLAIN(mNext, 1)
      BS_RTTI_MEMBER_PLAIN(mBlendIn, 2)
      BS_RTTI_MEMBER_PLAIN(mBlendOut, 3)
      BS_RTTI_MEMBER_PLAIN(mLayer, 4)
      BS_RTTI_MEMBER_PLAIN(mShouldMoveModel, 5)
      BS_RTTI_MEMBER_PLAIN(mShouldRotateModel, 6)
      BS_RTTI_MEMBER_PLAIN(mShouldQueueIntoLayer, 7)
      BS_RTTI_MEMBER_PLAIN(mIsFlyingAnimation, 8)
      BS_RTTI_MEMBER_PLAIN(mIsIdleAnimation, 9)
      BS_RTTI_MEMBER_PLAIN(mIsLooping, 10)
      BS_RTTI_MEMBER_PLAIN(mDirection, 11)
      BS_END_RTTI_MEMBERS

      const bs::String& getRTTIName() override
      {
        static bs::String name = "AnimationBase";
        return name;
      }

      bs::UINT32 getRTTIId() override { return TID_ZAnimation; }

      bs::SPtr<bs::IReflectable> newRTTIObject() override
      {
        return BsZenLib::Res::ZAnimationClip::createEmpty();
      }
    };

    class ModelScriptFileRTTI
        : public bs::RTTIType<BsZenLib::Res::ModelScriptFile, bs::Resource, ModelScriptFileRTTI>
    {
    public:
      using UINT32 = bs::UINT32;

      BS_BEGIN_RTTI_MEMBERS
      BS_RTTI_MEMBER_REFL_ARRAY(mMeshes, 0)
      BS_RTTI_MEMBER_REFL_ARRAY(mAnimations, 1)
      BS_END_RTTI_MEMBERS

      void onDeserializationEnded(bs::IReflectable* obj, bs::SerializationContext* context) override
      {
        auto* modelScript = static_cast<BsZenLib::Res::ModelScriptFile*>(obj);

        modelScript->_buildMeshesByNameMap();
      }

      const bs::String& getRTTIName() override
      {
        static bs::String name = "ModelScriptFile";
        return name;
      }

      UINT32 getRTTIId() override { return TID_ModelScriptFile; }

      bs::SPtr<bs::IReflectable> newRTTIObject() override
      {
        return BsZenLib::Res::ModelScriptFile::createEmpty();
      }
    };

    class MeshWithMaterialsRTTI
        : public bs::RTTIType<BsZenLib::Res::MeshWithMaterials, bs::Resource, MeshWithMaterialsRTTI>
    {
    public:
      using UINT32 = bs::UINT32;

      BS_BEGIN_RTTI_MEMBERS
      BS_RTTI_MEMBER_REFL(mMesh, 0)
      BS_RTTI_MEMBER_REFL_ARRAY(mMaterials, 1)
      BS_RTTI_MEMBER_PLAIN_ARRAY(mAttachmentNodeNames, 2)
      BS_RTTI_MEMBER_REFL_ARRAY(mNodeAttachments, 3)
      BS_END_RTTI_MEMBERS

      const bs::String& getRTTIName() override
      {
        static bs::String name = "MeshWithMaterials";
        return name;
      }

      UINT32 getRTTIId() override { return TID_MeshWithMaterials; }

      bs::SPtr<bs::IReflectable> newRTTIObject() override
      {
        return BsZenLib::Res::MeshWithMaterials::createEmpty();
      }
    };
  }  // namespace Res
}  // namespace BsZenLib
