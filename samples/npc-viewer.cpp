#include <string>
#include "BsApplication.h"
#include "BsFPSCamera.h"
#include "BsObjectRotator.h"
#include "BsCameraZoomer.h"
#include <assert.h>
#include <BsZenLib/ImportStaticMesh.hpp>
#include <BsZenLib/ImportZEN.hpp>
#include <Components/BsCCamera.h>
#include <Components/BsCLight.h>
#include <Components/BsCRenderable.h>
#include <GUI/BsCGUIWidget.h>
#include <GUI/BsGUILabel.h>
#include <GUI/BsGUIListBox.h>
#include <GUI/BsGUIPanel.h>
#include <Input/BsVirtualInput.h>
#include <Resources/BsBuiltinResources.h>
#include <Scene/BsSceneObject.h>
#include <vdfs/fileIndex.h>
#include <zenload/zCProgMeshProto.h>
#include <Renderer/BsLight.h>

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
  ZenLoad::zCProgMeshProto progMesh(file.c_str(), vdfs);

  if (progMesh.getNumSubmeshes() == 0) return {};

  ZenLoad::PackedMesh packedMesh;
  progMesh.packMesh(packedMesh, 0.01f);

  HSceneObject so = BsZenLib::ImportStaticMeshWithMaterials(file.c_str(), packedMesh, vdfs);

  if (!so) return {};

  return so;
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

  VDFS::FileIndex vdf;
  vdf.loadVDF(dataDir + "/Meshes.vdf");
  vdf.loadVDF(dataDir + "/Textures.vdf");
  vdf.finalizeLoad();

  if (vdf.getKnownFiles().empty())
  {
    std::cout << "No files loaded into the VDFS - is the datapath correct?" << std::endl;
    return -1;
  }

  VideoMode videoMode(1280, 720);
  Application::startUp(videoMode, "npc-viewer", false);

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

  // Add shown mesh
  HSceneObject shownMeshSO = SceneObject::create("default");
  shownMeshSO->addComponent<CRenderable>()->setMesh(gBuiltinResources().getMesh(BuiltinMesh::Box));
  shownMeshSO->addComponent<ObjectRotator>();

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
    if (f.find(".MRM") != std::string::npos)
    {
      listBoxElements.push_back(HString(f.c_str()));
    }
  }

  GUIListBox* listBox = mainPanel->addNewElement<GUIListBox>(listBoxElements);

  listBox->onSelectionToggled.connect([&](UINT32 idx, bool enabled) {
    String newMesh = listBoxElements[idx].getValue();

    gDebug().logDebug("User selected element: \"" + newMesh + "\"");

    HSceneObject newSO = loadMesh(newMesh, vdf);

    shownMeshSO->destroy(true);

    shownMeshSO = newSO;

    Sphere bounds = newSO->getComponent<CRenderable>()->getBounds().getSphere();
    sceneCameraSO->setPosition(bounds.getCenter() +
                               Vector3(2.0f, 1.0f, 2.0f).normalize() * bounds.getRadius() * 0.2f);
    newSO->addComponent<ObjectRotator>();
  });

  listBox->setPosition(10, 10);
  listBox->setWidth(200);

  setupInputConfig();

  Application::instance().runMainLoop();
  Application::shutDown();
  return 0;
}
