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
#include <vdfs/fileIndex.h>
#include <Scene/BsSceneObject.h>

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
  inputConfig->registerButton("RotateObj", BC_MOUSE_LEFT);
  inputConfig->registerButton("RotateCam", BC_MOUSE_RIGHT);

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

  const std::string dataDir = argv[1];
  const std::string zenFile = argv[2];

  VDFS::FileIndex vdf;
  vdf.loadVDF(dataDir + "/Worlds.vdf");
  vdf.loadVDF(dataDir + "/Textures.vdf");
  vdf.finalizeLoad();

  if (vdf.getKnownFiles().empty())
  {
    std::cout << "No files loaded into the VDFS - is the datapath correct?" << std::endl;
    return -1;
  }

  VideoMode videoMode(1280, 720);
  Application::startUp(videoMode, "zen-bs", false);

  // Add a scene object containing a camera component
  HSceneObject sceneCameraSO = SceneObject::create("SceneCamera");
  HCamera sceneCamera = sceneCameraSO->addComponent<CCamera>();
  sceneCamera->setMain(true);
  sceneCamera->setMSAACount(1);

  // Disable some fancy rendering
  auto rs = sceneCamera->getRenderSettings();

  rs->screenSpaceReflections.enabled = false;
  rs->ambientOcclusion.enabled = false;
  rs->enableIndirectLighting = true;
  rs->enableFXAA = false;
  rs->enableHDR = false;
  rs->enableTonemapping = false;

  sceneCamera->setRenderSettings(rs);

  // Position the camera
  sceneCameraSO->setPosition(Vector3(0.0f, 20.0f, 0.0f));
  sceneCameraSO->lookAt(Vector3(0, 0, 0));
  sceneCameraSO->addComponent<FPSCamera>();

  // Import a Gothic ZEN
  HSceneObject worldSO = BsZenLib::ImportZEN(zenFile, vdf);

  setupInputConfig();

  Application::instance().runMainLoop();
  Application::shutDown();
  return 0;
}
