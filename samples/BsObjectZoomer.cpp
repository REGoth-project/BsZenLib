#include "BsObjectZoomer.h"
#include "Math/BsVector3.h"
#include "Utility/BsTime.h"
#include "Math/BsMath.h"
#include "Scene/BsSceneObject.h"
#include "Platform/BsCursor.h"

namespace bs
{
    const float ObjectZoomer::ZOOM_STEP = 0.01f;

    ObjectZoomer::ObjectZoomer(const HSceneObject& parent)
        : Component(parent)
        , dist(-2.f)
	{
		// Set a name for the component, so we can find it later if needed
        setName("ObjectZoomer");

		// Get handles for key bindings. Actual keys attached to these bindings will be registered during app start-up.
        mZoomAxis = VirtualAxis("Zoom");
	}

    void ObjectZoomer::update()
	{
        float delta = gVirtualInput().getAxisValue(mZoomAxis) * ZOOM_STEP;
        if(delta != 0.f)
        {
            dist -= delta;
            const Transform &transform = SO()->getTransform();
            Vector3 objPos = transform.getPosition();
            Vector3 camFwd(1.f, 0.f, 1.f);
            objPos = camFwd * dist;
            SO()->setPosition(objPos);
        }
    }
}
