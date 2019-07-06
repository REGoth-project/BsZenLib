#include "ImportAnimation.hpp"
#include "ImportPath.hpp"
#include "ImportSkeletalMesh.hpp"
#include "ResourceManifest.hpp"
#include <Animation/BsAnimationClip.h>
#include <Animation/BsAnimationUtility.h>
#include <FileSystem/BsFileSystem.h>
#include <Resources/BsResources.h>
#include <vdfs/fileIndex.h>
#include <zenload/modelAnimationParser.h>
#include <zenload/modelScriptParser.h>
#include <zenload/zCModelAni.h>
#include <zenload/zCModelMeshLib.h>
#include <zenload/zenParser.h>

using namespace bs;
using namespace BsZenLib;
using namespace BsZenLib::Res;

struct AnimationToImport
{
  String fullAnimationName;

  ZenLoad::zCModelScriptAni animation;

  Vector<ZenLoad::zCModelScriptEventSfx> eventsSfx;
  Vector<ZenLoad::zCModelScriptEventSfx> eventsSfxGround;

  Vector<ZenLoad::zCModelScriptEventPfx> eventsPfx;
  Vector<ZenLoad::zCModelScriptEventPfxStop> eventsPfxStop;

  Vector<ZenLoad::zCModelScriptEventTag> eventsTag;
};

static Matrix3 createMatrixFromQuaternion(const Quaternion& quat);
static Matrix4 convertMatrix(const ZMath::Matrix& m);
static int32_t scaleFrameToHeaderFrameRate(const ZenLoad::zCModelScriptAni& ani, size_t frame,
                                           size_t numFramesTotal);

struct AnimationCurvesWithRootMotion
{
  bs::AnimationCurves curves;
  bs::RootMotion rootMotion;
};

static AnimationCurvesWithRootMotion importAnimationSamples(const String& virtualFilePath,
                                                            const ZenLoad::zCModelMeshLib& meshLib,
                                                            const ZenLoad::zCModelScriptAni& def,
                                                            const VDFS::FileIndex& vdfs);
static AnimationCurvesWithRootMotion convertSamples(const std::vector<ZenLoad::ModelNode>& nodes,
                                                    const ZenLoad::zCModelScriptAni& def,
                                                    const ZenLoad::ModelAnimationParser& parser);

bool BsZenLib::HasCachedMAN(const bs::String& fullAnimationName)
{
  return HasCachedResource(GothicPathToCachedZAnimation(fullAnimationName));
}

Res::HZAnimation BsZenLib::LoadCachedAnimation(const bs::String& fullAnimationName)
{
  return bs::gResources().load<ZAnimationClip>(GothicPathToCachedZAnimation(fullAnimationName));
}

Res::HZAnimation BsZenLib::ImportMAN(const ZenLoad::zCModelMeshLib& meshLib,
                                     const AnimationToImport& def, const VDFS::FileIndex& vdfs)
{
  gDebug().logDebug("Caching Animation: " + def.fullAnimationName);

  // Import keyframes and convert them to curves (From .MAN-file)
  AnimationCurvesWithRootMotion samples =
      importAnimationSamples(def.fullAnimationName + ".MAN", meshLib, def.animation, vdfs);

  SPtr<RootMotion> pRootMotion = bs::bs_shared_ptr_new<RootMotion>(samples.rootMotion);
  SPtr<AnimationCurves> pCurves = bs::bs_shared_ptr_new<AnimationCurves>(samples.curves);

  HAnimationClip clip = AnimationClip::create(pCurves, false, 1, pRootMotion);
  HZAnimation anim = ZAnimationClip::create();

  anim->mClip = clip;

  anim->mShouldQueueIntoLayer = (def.animation.m_Flags & ZenLoad::MSB_QUEUE_ANI) != 0;
  anim->mShouldRotateModel = (def.animation.m_Flags & ZenLoad::MSB_ROTATE_MODEL) != 0;
  anim->mIsFlyingAnimation = (def.animation.m_Flags & ZenLoad::MSB_FLY) != 0;
  anim->mShouldMoveModel = (def.animation.m_Flags & ZenLoad::MSB_MOVE_MODEL) != 0;
  anim->mIsIdleAnimation = (def.animation.m_Flags & ZenLoad::MSB_IDLE) != 0;
  anim->mIsLooping = def.animation.m_Next == def.animation.m_Name;

  // Gothics default layer is 1, while bsf uses 0. Therefore, subtract 1 here.
  anim->mLayer = def.animation.m_Layer - 1;

  switch (def.animation.m_Dir)
  {
    case ZenLoad::MSB_BACKWARD:
      anim->mDirection = ZAnimationClip::Direction::Reverse;
      break;

    default:
    case ZenLoad::MSB_FORWARD:
      anim->mDirection = ZAnimationClip::Direction::Forward;
      break;
  }

  Vector<AnimationEvent> events;

  // Notify about the next animation if needed
  if (!anim->mIsLooping)
  {
    if (def.animation.m_Next.empty())
    {
      bs::String command = "STOP";

      float endOfAnimation = clip->getLength();
      events.emplace_back(command, endOfAnimation);
    }
    else
    {
      bs::String command = "PLAYCLIP:" + bs::String(def.animation.m_Next.c_str());

      float endOfAnimation = clip->getLength();
      events.emplace_back(command, endOfAnimation);
    }
  }

  clip->setEvents(events);

  // TODO: clip->setEvents(...) for sounds/particles/other

  clip->setName(def.fullAnimationName);
  anim->setName(def.fullAnimationName);

  const bool overwrite = true;
  gResources().save(clip, GothicPathToCachedAnimationClip(def.fullAnimationName), overwrite);
  AddToResourceManifest(clip, GothicPathToCachedAnimationClip(def.fullAnimationName));

  gResources().save(anim, GothicPathToCachedZAnimation(def.fullAnimationName), overwrite);
  AddToResourceManifest(anim, GothicPathToCachedZAnimation(def.fullAnimationName));

  return anim;
}

BsZenLib::Res::HZAnimation BsZenLib::AliasAnimation(const AnimationToAlias& def)
{
  gDebug().logDebug("Aliasing Animation: " + def.fullAnimationName + " to " +
                    def.fullAnimationNameOfAlias);

  HZAnimation anim = ZAnimationClip::create();

  HZAnimation toAlias = LoadCachedAnimation(def.fullAnimationNameOfAlias);

  if (!toAlias)
  {
    BS_EXCEPT(InvalidStateException, "Animation to alias has to have been cached before!");
  }

  anim->mClip = toAlias->mClip;

  anim->mShouldQueueIntoLayer = (def.animation.m_Flags & ZenLoad::MSB_QUEUE_ANI) != 0;
  anim->mShouldRotateModel = (def.animation.m_Flags & ZenLoad::MSB_ROTATE_MODEL) != 0;
  anim->mIsFlyingAnimation = (def.animation.m_Flags & ZenLoad::MSB_FLY) != 0;
  anim->mShouldMoveModel = (def.animation.m_Flags & ZenLoad::MSB_MOVE_MODEL) != 0;
  anim->mIsIdleAnimation = (def.animation.m_Flags & ZenLoad::MSB_IDLE) != 0;
  anim->mIsLooping = def.animation.m_Next == def.animation.m_Name;

  // Gothics default layer is 1, while bsf uses 0. Therefore, subtract 1 here.
  anim->mLayer = def.animation.m_Layer - 1;

  switch (def.animation.m_Dir)
  {
    case ZenLoad::MSB_BACKWARD:
      anim->mDirection = ZAnimationClip::Direction::Reverse;
      break;

    default:
    case ZenLoad::MSB_FORWARD:
      anim->mDirection = ZAnimationClip::Direction::Forward;
      break;
  }

  anim->setName(def.fullAnimationName);

  const bool overwrite = true;
  gResources().save(anim, GothicPathToCachedZAnimation(def.fullAnimationName), overwrite);
  AddToResourceManifest(anim, GothicPathToCachedZAnimation(def.fullAnimationName));

  return anim;
}

BsZenLib::Res::HZAnimation BsZenLib::BlendAnimation(const AnimationToBlend& def)
{
  gDebug().logDebug("Blending Animation: " + def.fullAnimationName + " to " +
                    def.fullAnimationNameOfBlend);

  HZAnimation anim = ZAnimationClip::create();

  HZAnimation toAlias = LoadCachedAnimation(def.fullAnimationNameOfBlend);

  if (!toAlias)
  {
    BS_EXCEPT(InvalidStateException, "Animation to alias has to have been cached before!");
  }

  anim->mClip = toAlias->mClip;

  anim->mShouldQueueIntoLayer = toAlias->mShouldQueueIntoLayer;
  anim->mShouldRotateModel = toAlias->mShouldRotateModel;
  anim->mIsFlyingAnimation = toAlias->mIsFlyingAnimation;
  anim->mShouldMoveModel = toAlias->mShouldMoveModel;
  anim->mIsIdleAnimation = toAlias->mIsIdleAnimation;
  anim->mIsLooping = toAlias->mIsLooping;
  anim->mDirection = toAlias->mDirection;

  // Gothics default layer is 1, while bsf uses 0. Therefore, subtract 1 here.
  bs::INT32 layer = anim->mLayer = def.animation.m_Layer - 1;

  anim->setName(def.fullAnimationName);

  const bool overwrite = true;
  gResources().save(anim, GothicPathToCachedZAnimation(def.fullAnimationName), overwrite);
  AddToResourceManifest(anim, GothicPathToCachedZAnimation(def.fullAnimationName));

  return anim;
}

static AnimationCurvesWithRootMotion importAnimationSamples(const String& manFile,
                                                            const ZenLoad::zCModelMeshLib& meshLib,
                                                            const ZenLoad::zCModelScriptAni& def,
                                                            const VDFS::FileIndex& vdfs)
{
  ZenLoad::ZenParser zen(manFile.c_str(), vdfs);
  ZenLoad::ModelAnimationParser parser(zen);

  parser.setScale(1.0f / 100.0f);  // Centimeters -> Meters

  ZenLoad::zCModelAniHeader header;
  std::vector<ZenLoad::zCModelAniSample> samples;
  std::vector<uint32_t> nodeIndexList;

  ZenLoad::ModelAnimationParser::EChunkType type;
  while ((type = parser.parse()) != ZenLoad::ModelAnimationParser::CHUNK_EOF)
  {
    switch (type)
    {
      case ZenLoad::ModelAnimationParser::CHUNK_ERROR:
        return {};

      default:
        // Continue
        break;
    }
  }

  return convertSamples(meshLib.getNodes(), def, parser);
}

static AnimationCurvesWithRootMotion convertSamples(const std::vector<ZenLoad::ModelNode>& nodes,
                                                    const ZenLoad::zCModelScriptAni& def,
                                                    const ZenLoad::ModelAnimationParser& parser)
{
  AnimationCurvesWithRootMotion result = {};

  size_t numFramesTotal = parser.getSamples().size() / nodes.size();
  size_t startFrame = def.m_FirstFrame;
  size_t lastFrame = def.m_LastFrame >= 0 ? def.m_LastFrame : numFramesTotal;
  size_t numFrames = lastFrame - startFrame + 1;
  size_t numNodes = parser.getNodeIndex().size();
  float speed = def.m_Speed;

  // Some animations seem to be empty? Is this a bug or are they supposed to be empty?
  if (numFramesTotal == 0 || numFrames == 0)
  {
    return {};
  }

  if (startFrame > lastFrame ||           // This sometimes happens?
      startFrame + 1 > numFramesTotal ||  // Can be a huge number...
      lastFrame + 1 > numFramesTotal)     // This as well
  {
    startFrame = 0;
    lastFrame = numFramesTotal - 1;
    numFrames = numFramesTotal;
  }

  assert(startFrame < numFramesTotal);
  assert(lastFrame < numFramesTotal);
  assert(numFrames <= numFramesTotal);

  Vector<bool> animatedNodes(nodes.size(), false);
  for (size_t nodeIdx = 0; nodeIdx < numNodes; nodeIdx++)
  {
    size_t realNodeIdx = parser.getNodeIndex()[nodeIdx];

    // if (realNodeIdx != 0)
    // continue;

    assert(realNodeIdx < nodes.size());

    if (realNodeIdx >= nodes.size()) return {};

    const ZenLoad::ModelNode& node = nodes[realNodeIdx];

    Vector<TKeyframe<Vector3>> positionKeyframes(numFrames);
    Vector<TKeyframe<Quaternion>> rotationKeyframes(numFrames);

    for (size_t frame = 0; frame < numFrames; frame++)
    {
      // One Frame is stored as a continuous array of samples for each node.
      // That means, that for each frame, there are as many samples as there are
      // nodes. To get all samples for only one node, you have to skip all the others
      // to get to the next frame.
      size_t realFrame = frame + startFrame;
      size_t sampleIdx = numNodes * realFrame + nodeIdx;
      const ZenLoad::zCModelAniSample& sample = parser.getSamples()[sampleIdx];

      Vector4 sdfsd;
      Vector3 position = Vector3(sample.position.x,   // ...
                                 sample.position.y,   // ...
                                 sample.position.z);  // ...

      Quaternion rotation = Quaternion(sample.rotation.w,    // ...
                                       -sample.rotation.x,   // ...
                                       -sample.rotation.y,   // ...
                                       -sample.rotation.z);  // ...

      // Matrix3 workaround = createMatrixFromQuaternion(rotation);
      // rotation.fromRotationMatrix(workaround);

      Vector3 localPosition;
      Quaternion localRotation;
      Vector3 localScale;

      convertMatrix(node.transformLocal).decomposition(localPosition, localRotation, localScale);

      // TODO: Find out whether these are seconds, milliseconds, ...?
      float keyframeTime = frame / parser.getHeader().fpsRate;

      positionKeyframes[frame].inTangent = Vector3(BsZero);
      positionKeyframes[frame].outTangent = Vector3(BsZero);
      positionKeyframes[frame].value = position;
      positionKeyframes[frame].time = keyframeTime;

      rotationKeyframes[frame].inTangent = Quaternion(BsZero);
      rotationKeyframes[frame].outTangent = localRotation;
      rotationKeyframes[frame].value = rotation;
      rotationKeyframes[frame].time = keyframeTime;
    }

    AnimationUtility::calculateTangents(positionKeyframes);
    AnimationUtility::calculateTangents(rotationKeyframes);

    TAnimationCurve<Vector3> positionCurve(positionKeyframes);
    TAnimationCurve<Quaternion> rotationCurve(rotationKeyframes);

    if (realNodeIdx == 0)
    {
      result.rootMotion = RootMotion(positionCurve, rotationCurve);

      // It's important that this is not set as 'animated node' so the node
      // will be filled with dummy data later on
    }
    else
    {
      result.curves.addPositionCurve(node.name.c_str(), positionCurve);
      result.curves.addRotationCurve(node.name.c_str(), rotationCurve);

      animatedNodes[realNodeIdx] = true;
    }
  }

  // Add all non-animated nodes
  for (size_t i = 0; i < nodes.size(); i++)
  {
    // Does this node already have animation data?
    if (animatedNodes[i]) continue;

    const ZenLoad::ModelNode& node = nodes[i];

    Vector3 position;
    Quaternion rotation;
    Vector3 scale;

    convertMatrix(node.transformLocal).decomposition(position, rotation, scale);

    Vector<TKeyframe<Vector3>> positionKeyframes(1);
    Vector<TKeyframe<Quaternion>> rotationKeyframes(1);

    positionKeyframes[0].inTangent = Vector3(BsZero);
    positionKeyframes[0].outTangent = Vector3(BsZero);
    positionKeyframes[0].value = position;
    positionKeyframes[0].time = 0.0f;

    rotationKeyframes[0].inTangent = Quaternion(BsZero);
    rotationKeyframes[0].outTangent = Quaternion(BsZero);
    rotationKeyframes[0].value = rotation;
    rotationKeyframes[0].time = 0.0f;

    AnimationUtility::calculateTangents(positionKeyframes);
    AnimationUtility::calculateTangents(rotationKeyframes);

    TAnimationCurve<Vector3> positionCurve(positionKeyframes);
    TAnimationCurve<Quaternion> rotationCurve(rotationKeyframes);

    result.curves.addPositionCurve(node.name.c_str(), positionCurve);
    result.curves.addRotationCurve(node.name.c_str(), rotationCurve);
  }

  return result;
}

/**
 * The quaternions supplied by Gothic seem to be a little bit different...
 */
static Matrix3 createMatrixFromQuaternion(const Quaternion& quat)
{
  Matrix3 m = Matrix3::IDENTITY;

  Vector3 cols[3];
  cols[0].x = quat.w * quat.w + quat.x * quat.x - quat.y * quat.y - quat.z * quat.z;
  cols[0].y = 2.0f * (quat.x * quat.y + quat.w * quat.z);
  cols[0].z = 2.0f * (quat.x * quat.z - quat.w * quat.y);
  cols[1].x = 2.0f * (quat.x * quat.y - quat.w * quat.z);
  cols[1].y = quat.w * quat.w - quat.x * quat.x + quat.y * quat.y - quat.z * quat.z;
  cols[1].z = 2.0f * (quat.y * quat.z + quat.w * quat.x);
  cols[2].x = 2.0f * (quat.x * quat.z + quat.w * quat.y);
  cols[2].y = 2.0f * (quat.y * quat.z - quat.w * quat.x);
  cols[2].z = quat.w * quat.w - quat.x * quat.x - quat.y * quat.y + quat.z * quat.z;

  m.setColumn(0, cols[0]);
  m.setColumn(1, cols[1]);
  m.setColumn(2, cols[2]);

  return m;
}

static Matrix4 convertMatrix(const ZMath::Matrix& m)
{
  Matrix4 bs = {m.mv[0], m.mv[1], m.mv[2],  m.mv[3],  m.mv[4],  m.mv[5],  m.mv[6],  m.mv[7],
                m.mv[8], m.mv[9], m.mv[10], m.mv[11], m.mv[12], m.mv[13], m.mv[14], m.mv[15]};

  return bs.transpose();
}

/**
 * This weird function scales the frame when an event occurs according to m_FrameCount (header)
 * FIXME there are cases when m_LastFrame, m_FirstFrame and other values are obviously incorrect
 * (high values) I checked for overflows/weird arithmetic with unsigned/signed stuff but the numbers
 * are not even close to maximum/minimum numbers. m_FrameCount doesn't have this Problem, thats why
 * animations are played "correctly" ingame. In many cases m_LastFrame, m_FirstFrame and m_FrameCount
 * don't match.
 */
static int32_t scaleFrameToHeaderFrameRate(const ZenLoad::zCModelScriptAni& ani, size_t frame,
                                           size_t numFramesTotal)
{
  if (ani.m_LastFrame < ani.m_FirstFrame) return frame;

  auto diff = ani.m_LastFrame - ani.m_FirstFrame;
  float pos = 0.0f;

  if (diff == 0) return numFramesTotal;

  pos = frame / (float)diff;
  return static_cast<int32_t>(pos * numFramesTotal);
}
