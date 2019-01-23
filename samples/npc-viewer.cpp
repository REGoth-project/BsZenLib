#include <string>
#include "BsApplication.h"
#include <FileSystem/BsFileSystem.h>
#include "BsCameraZoomer.h"
#include "BsFPSCamera.h"
#include "BsObjectRotator.h"
#include <assert.h>
#include <BsZenLib/ImportAnimation.hpp>
#include <BsZenLib/ImportStaticMesh.hpp>
#include <BsZenLib/ImportSkeletalMesh.hpp>
#include <BsZenLib/ImportZEN.hpp>
#include <Components/BsCAnimation.h>
#include <Components/BsCCamera.h>
#include <Components/BsCLight.h>
#include <Components/BsCRenderable.h>
#include <GUI/BsCGUIWidget.h>
#include <GUI/BsGUILabel.h>
#include <GUI/BsGUIListBox.h>
#include <GUI/BsGUIPanel.h>
#include <Input/BsVirtualInput.h>
#include <Renderer/BsLight.h>
#include <Resources/BsBuiltinResources.h>
#include <Resources/BsResourceManifest.h>
#include <Resources/BsResources.h>
#include <Scene/BsSceneObject.h>
#include <vdfs/fileIndex.h>
#include <zenload/zCProgMeshProto.h>
#include <BsZenLib/ImportPath.hpp>

using namespace bs;
using namespace BsZenLib::Res;

/** Registers a common set of keys/buttons that are used for controlling the examples. */
static void setupInputConfig()
{
  using namespace bs;

  auto inputConfig = gVirtualInput().getConfiguration();

  inputConfig->registerButton("RotateObj", BC_MOUSE_LEFT);
  inputConfig->registerButton("RotateCam", BC_MOUSE_RIGHT);

  // Camera controls for axes (analog input, e.g. mouse or gamepad thumbstick)
  // These return values in [-1.0, 1.0] range.
  inputConfig->registerAxis("Horizontal", VIRTUAL_AXIS_DESC((UINT32)InputAxis::MouseX));
  inputConfig->registerAxis("Vertical", VIRTUAL_AXIS_DESC((UINT32)InputAxis::MouseY));
  inputConfig->registerAxis("Zoom", VIRTUAL_AXIS_DESC((UINT32)InputAxis::MouseZ));
}

static HSceneObject loadMesh(const String& file, const String& visual, const VDFS::FileIndex& vdfs)
{
}

int main(int argc, char** argv)
{
  VDFS::FileIndex::initVDFS(argv[0]);

  if (argc < 2)
  {
    std::cout << "Usage: npc-viewer <path/to/gothic/data>" << std::endl;
    return -1;
  }

  const std::string dataDir = argv[1];

  VDFS::FileIndex vdfs;
  vdfs.loadVDF(dataDir + "/Meshes.vdf");
  vdfs.loadVDF(dataDir + "/Textures.vdf");
  vdfs.loadVDF(dataDir + "/Anims.vdf");
  vdfs.finalizeLoad();

  if (vdfs.getKnownFiles().empty())
  {
    std::cout << "No files loaded into the VDFS - is the datapath correct?" << std::endl;
    return -1;
  }

  VideoMode videoMode(1280, 720);
  Application::startUp(videoMode, "npc-viewer", false);

  if (FileSystem::exists(BsZenLib::GothicPathToCachedManifest("resources")))
  {
	  auto prevManifest = ResourceManifest::load(BsZenLib::GothicPathToCachedManifest("resources"), BsZenLib::GetCacheDirectory());
	  gResources().registerResourceManifest(prevManifest);
  }

  // Add a scene object containing a camera component
  HSceneObject sceneCameraSO = SceneObject::create("SceneCamera");
  HCamera sceneCamera = sceneCameraSO->addComponent<CCamera>();
  sceneCamera->setMain(true);
  sceneCamera->setMSAACount(1);

  // Disable some fancy rendering
  auto rs = sceneCamera->getRenderSettings();

  // rs->screenSpaceReflections.enabled = false;
  // rs->ambientOcclusion.enabled = false;
  // rs->enableIndirectLighting = true;
  // rs->enableFXAA = false;
  rs->enableHDR = false;
  rs->enableTonemapping = false;

  sceneCamera->setRenderSettings(rs);

  // Pick a prettier background color
  float clearIntensity = 0.01f;
  Color gray = Color(clearIntensity, clearIntensity, clearIntensity);
  sceneCamera->getViewport()->setClearColorValue(gray);

  // Position the camera
  sceneCameraSO->setPosition(Vector3(3.0f, 2.0f, 3.0f));
  sceneCameraSO->lookAt(Vector3(0, 0, 0));
  sceneCameraSO->addComponent<CameraZoomer>();

  // Load a model and its animations
  HModelScriptFile model;

  const String file = "HUMANS.MDS";
  const String visual = "HUM_BODY_NAKED0.ASC";
  
  if (BsZenLib::HasCachedMDS(file))
  {
    model = BsZenLib::LoadCachedMDS(file);
  } 
  else
  {
    model = BsZenLib::ImportAndCacheMDS(file, vdfs);
  }

  if (!model || model->getMeshes().empty())
  {
    gDebug().logError("Failed to load model or animations: " + file + "/" + visual);
    return -1;
  }

  HSceneObject shownMeshSO = SceneObject::create(visual);
  shownMeshSO->addComponent<ObjectRotator>();

  HSceneObject meshSO = SceneObject::create(visual + "-draw");
  meshSO->setParent(shownMeshSO);

  HRenderable renderable = meshSO->addComponent<CRenderable>();

  HMeshWithMaterials sheepVisual = model->getMeshes()[0];
  renderable->setMesh(sheepVisual->getMesh());
  renderable->setMaterials(sheepVisual->getMaterials());

  HAnimation animation = meshSO->addComponent<CAnimation>();
  animation->setWrapMode(AnimWrapMode::Loop);

  //if (!model->getAnimationClips().empty())
  //{

    //for (size_t i = 0; i < model->getAnimationClips().size(); i++)
    //{
      //gDebug().logDebug(toString(i) + ": " + model->getAnimationClips()[i]->getName());
    //}

    //HAnimationClip clip = model->getAnimationClips()[2];

    //gDebug().logDebug("Playing animation: " + clip->getName());

    //animation->setWrapMode(AnimWrapMode::Loop);
    //animation->play(clip);
  //}

  shownMeshSO->setScale(Vector3(0.01f, 0.01f, 0.01f));

  // Add GUI
  HSceneObject guiSO = SceneObject::create("GUI");

  float guiScale = 1.0f;
  guiSO->setScale(Vector3(guiScale, guiScale, guiScale));
  HGUIWidget gui = guiSO->addComponent<CGUIWidget>(sceneCamera);
  gui->setSkin(BuiltinResources::instance().getGUISkin());

  GUIPanel* mainPanel = gui->getPanel();

  Vector<HString> listBoxElementsAnims;
  Vector<HString> listBoxElementsMeshes;

  for (const auto& ani : model->getAnimationClips())
  {
    listBoxElementsAnims.push_back(HString(ani->getName()));
  }

  for (const auto& mesh : model->getMeshes())
  {
    listBoxElementsMeshes.push_back(HString(mesh->getName()));
  }

  GUIListBox* listBoxAnims = mainPanel->addNewElement<GUIListBox>(listBoxElementsAnims);

  listBoxAnims->onSelectionToggled.connect([&](UINT32 idx, bool enabled) {
    animation->play(model->getAnimationClips()[idx]);
  });

  listBoxAnims->setPosition(10, 10);
  listBoxAnims->setWidth(200);

  GUIListBox* listBoxMeshes = mainPanel->addNewElement<GUIListBox>(listBoxElementsMeshes);

  listBoxMeshes->onSelectionToggled.connect([&](UINT32 idx, bool enabled) {
    HMeshWithMaterials mesh = model->getMeshes()[idx];

    renderable->setMesh(mesh->getMesh());
    renderable->setMaterials(mesh->getMaterials());
  });

  listBoxMeshes->setPosition(220, 10);
  listBoxMeshes->setWidth(200);


  setupInputConfig();

  SPtr<ResourceManifest> manifest = gResources().getResourceManifest("Default");
  ResourceManifest::save(manifest, BsZenLib::GothicPathToCachedManifest("resources"), BsZenLib::GetCacheDirectory());

  Application::instance().runMainLoop();

  manifest = gResources().getResourceManifest("Default");
  ResourceManifest::save(manifest, BsZenLib::GothicPathToCachedManifest("resources"), BsZenLib::GetCacheDirectory());

  Application::shutDown();
  return 0;
}
