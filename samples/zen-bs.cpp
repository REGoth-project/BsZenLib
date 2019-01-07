#include <string>
#include "BsApplication.h"
#include "BsFPSCamera.h"
#include <assert.h>
#include <BsZenLib/ImportZEN.hpp>
#include <Components/BsCCamera.h>
#include <Components/BsCLight.h>
#include <Components/BsCRenderable.h>
#include <Input/BsVirtualInput.h>
#include <Scene/BsSceneObject.h>
#include <Scene/BsPrefab.h>
#include <vdfs/fileIndex.h>
#include <Scene/BsSceneObject.h>
#include <Resources/BsResources.h>
#include <BsZenLib/ImportPath.hpp>
#include <Resources/BsResourceManifest.h>
#include <FileSystem/BsFileSystem.h>

/** Registers a common set of keys/buttons that are used for controlling the examples. */
static void setupInputConfig()
{
  using namespace bs;

  auto inputConfig = gVirtualInput().getConfiguration();

  // Camera controls for buttons (digital 0-1 input, e.g. keyboard or gamepad button)
  inputConfig->registerButton("Forward", BC_W);
  inputConfig->registerButton("Back", BC_S);
  inputConfig->registerButton("Left", BC_A);
  inputConfig->registerButton("Right", BC_D);
  inputConfig->registerButton("Forward", BC_UP);
  inputConfig->registerButton("Back", BC_DOWN);
  inputConfig->registerButton("Left", BC_LEFT);
  inputConfig->registerButton("Right", BC_RIGHT);
  inputConfig->registerButton("FastMove", BC_LSHIFT);
  inputConfig->registerButton("Rotate", BC_MOUSE_LEFT);

  // Camera controls for axes (analog input, e.g. mouse or gamepad thumbstick)
  // These return values in [-1.0, 1.0] range.
  inputConfig->registerAxis("Horizontal", VIRTUAL_AXIS_DESC((UINT32)InputAxis::MouseX));
  inputConfig->registerAxis("Vertical", VIRTUAL_AXIS_DESC((UINT32)InputAxis::MouseY));
}

int main(int argc, char** argv)
{
  using namespace bs;

  VDFS::FileIndex::initVDFS(argv[0]);

  if (argc < 3)
  {
    std::cout << "Usage: zen-bs <path/to/gothic/data> <zenfile.zen>" << std::endl;
    return -1;
  }

  const String dataDir = argv[1];
  const String zenFile = argv[2];

  VDFS::FileIndex vdfs;
  vdfs.loadVDF((dataDir + "/Worlds.vdf").c_str());
  vdfs.loadVDF((dataDir + "/Textures.vdf").c_str());
  vdfs.loadVDF((dataDir + "/Meshes.vdf").c_str());
  vdfs.finalizeLoad();

  if (vdfs.getKnownFiles().empty())
  {
    std::cout << "No files loaded into the VDFS - is the datapath correct?" << std::endl;
    return -1;
  }

  // for (auto& s : vdfs.getKnownFiles())
  // {
  //   gDebug().logDebug(s.c_str());
  // }

  VideoMode videoMode(1280, 720);
  Application::startUp(videoMode, "zen-bs", false);
  
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

  rs->screenSpaceReflections.enabled = false;
  rs->ambientOcclusion.enabled = true;
  rs->enableIndirectLighting = true;
  rs->enableFXAA = true;
  rs->enableHDR = false;
  rs->enableTonemapping = false;

  sceneCamera->setRenderSettings(rs);

  // Position the camera
  sceneCameraSO->setPosition(Vector3(0.0f, 20.0f, 0.0f));
  sceneCameraSO->lookAt(Vector3(0, 0, 0));
  sceneCameraSO->addComponent<FPSCamera>();
  
  // Import a Gothic ZEN
  HPrefab worldPrefab;
  if (BsZenLib::HasCachedZEN(zenFile))
  {
	  worldPrefab = BsZenLib::LoadCachedZEN(zenFile);
  }
  else
  {
	  worldPrefab = BsZenLib::ImportAndCacheZEN(zenFile.c_str(), vdfs);
  }
   
  if (!worldPrefab)
  {
	  gDebug().logError("Failed to load ZEN: " + zenFile);
	  return -1;
  }

  worldPrefab.blockUntilLoaded();
  worldPrefab->instantiate();

  setupInputConfig();

  Application::instance().runMainLoop();

  SPtr<ResourceManifest> manifest = gResources().getResourceManifest("Default");
  ResourceManifest::save(manifest, BsZenLib::GothicPathToCachedManifest("resources"), BsZenLib::GetCacheDirectory());

  Application::shutDown();
  return 0;
}
