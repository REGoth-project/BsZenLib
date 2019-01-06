#include <string>
#include "BsApplication.h"
#include "BsFPSCamera.h"
#include "BsObjectRotator.h"
#include "BsCameraZoomer.h"
#include <assert.h>
#include <BsZenLib/ImportSkeletalMesh.hpp>
#include <BsZenLib/ImportStaticMesh.hpp>
#include <BsZenLib/ImportZEN.hpp>
#include <Components/BsCCamera.h>
#include <Components/BsCLight.h>
#include <Components/BsCRenderable.h>
#include <Debug/BsDebugDraw.h>
#include <GUI/BsCGUIWidget.h>
#include <GUI/BsGUILabel.h>
#include <GUI/BsGUIListBox.h>
#include <GUI/BsGUIPanel.h>
#include <Input/BsVirtualInput.h>
#include <Renderer/BsLight.h>
#include <Resources/BsBuiltinResources.h>
#include <Scene/BsSceneObject.h>
#include <vdfs/fileIndex.h>
#include <zenload/zCModelMeshLib.h>
#include <zenload/zCProgMeshProto.h>
#include <Resources/BsResources.h>
#include <BsZenLib/ImportPath.hpp>
#include <Resources/BsResourceManifest.h>
#include <FileSystem/BsFileSystem.h>
#include <BsZenLib/ImportTexture.hpp>

using namespace bs;


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

static HSceneObject loadMesh(const String& file, const VDFS::FileIndex& vdfs)
{
	using namespace bs;

  if (file.find(".MRM") != String::npos)
  {
	  HPrefab mesh;
	  if (FileSystem::isFile(BsZenLib::GothicPathToCachedAsset(file.c_str())))
	  {
		  mesh = BsZenLib::LoadCachedStaticMeshPrefab(file.c_str());
	  }
	  else
	  {
		  mesh = BsZenLib::ImportAndCacheStaticMeshPrefab(file.c_str(), vdfs);
	  }

	  if (!mesh)
		  return {};

    return mesh->instantiate();
  }
  else if (file.find(".MDL") != String::npos)
  {
	  HPrefab mesh;
	  if (FileSystem::isFile(BsZenLib::GothicPathToCachedAsset(file.c_str())))
	  {
		  mesh = BsZenLib::LoadCachedSkeletalMeshPrefab(file.c_str());
	  }
	  else
	  {
		  mesh = BsZenLib::ImportAndCacheSkeletalMeshPrefab(file.c_str(), vdfs);
	  }

	  if (!mesh)
		  return {};

	  return mesh->instantiate();
  }
  else
  {
    return {};
  }
}

int main(int argc, char** argv)
{
  VDFS::FileIndex::initVDFS(argv[0]);

  if (argc < 2)
  {
    std::cout << "Usage: mesh-viewer <path/to/gothic/data>" << std::endl;
    return -1;
  }

  const std::string dataDir = argv[1];

  VDFS::FileIndex vdf;
  vdf.loadVDF(dataDir + "/Meshes.vdf");
  vdf.loadVDF(dataDir + "/Textures.vdf");
  vdf.loadVDF(dataDir + "/Anims.vdf");
  vdf.finalizeLoad();

  if (vdf.getKnownFiles().empty())
  {
    std::cout << "No files loaded into the VDFS - is the datapath correct?" << std::endl;
    return -1;
  }

  VideoMode videoMode(1280, 720);
  Application::startUp(videoMode, "mesh-viewer", false);

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
  sceneCameraSO->setPosition(Vector3(3.0f, 2.0f, 3.0f) * 16.0f);
  sceneCameraSO->lookAt(Vector3(0, 0, 0));
  sceneCameraSO->addComponent<CameraZoomer>();

  // Add shown mesh
  HSceneObject shownMeshSO = SceneObject::create("default");
  shownMeshSO->addComponent<CRenderable>()->setMesh(gBuiltinResources().getMesh(BuiltinMesh::Box));

  // Add GUI
  HSceneObject guiSO = SceneObject::create("GUI");

  float guiScale = 1.0f;
  guiSO->setScale(Vector3(guiScale, guiScale, guiScale));
  HGUIWidget gui = guiSO->addComponent<CGUIWidget>(sceneCamera);
  gui->setSkin(BuiltinResources::instance().getGUISkin());

  GUIPanel* mainPanel = gui->getPanel();

  Vector<HString> listBoxElements;

  
  for (const std::string& f : vdf.getKnownFiles())
  {
    if (f.find(".MDL") != std::string::npos)
    {
      listBoxElements.push_back(HString(f.c_str()));

	  //gDebug().logDebug("Caching: " + String(f.c_str()));
	  //BsZenLib::ImportAndCacheSkeletalMesh(f.c_str(), vdf);
    }

    if (f.find(".MRM") != std::string::npos)
    {
      listBoxElements.push_back(HString(f.c_str()));
    }
  }


  GUIListBox* listBox = mainPanel->addNewElement<GUIListBox>(listBoxElements);

  auto showMesh = [&](String newMesh) {
    gDebug().logDebug("User selected element: \"" + newMesh + "\"");

    HSceneObject newSO = loadMesh(newMesh, vdf);

    if (newSO)
    {
      Sphere bounds = newSO->getComponent<CRenderable>()->getBounds().getSphere();
      sceneCameraSO->setPosition(bounds.getCenter() +
                                 Vector3(2.0f, 1.0f, 2.0f).normalize() * bounds.getRadius() * 0.5f);
	  newSO->addComponent<ObjectRotator>();
      newSO->addComponent<ObjectRotator>();
    }

    if (shownMeshSO)
    {
      shownMeshSO->destroy(true);
    }

    shownMeshSO = newSO;
  };

  showMesh("SHEEP_BODY.MDL");

  listBox->onSelectionToggled.connect([&](UINT32 idx, bool enabled) {
    String newMesh = listBoxElements[idx].getValue();

    showMesh(newMesh);
  });

  listBox->setPosition(10, 10);
  listBox->setWidth(200);

  setupInputConfig();
  Application::instance().runMainLoop();

  SPtr<ResourceManifest> manifest = gResources().getResourceManifest("Default");
  ResourceManifest::save(manifest, BsZenLib::GothicPathToCachedManifest("resources"), BsZenLib::GetCacheDirectory());

  Application::shutDown();
  return 0;
}
