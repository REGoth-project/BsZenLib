#pragma once

#include "BsPrerequisites.h"
#include "Scene/BsComponent.h"
#include "Math/BsMath.h"
#include "Input/BsVirtualInput.h"

namespace bs
{
    /** Component that controls zoom of its scene object. */
    class ObjectZoomer : public Component
	{
	public:
        ObjectZoomer(const HSceneObject& parent);

		/** Triggered once per frame. Allows the component to handle input and move. */
		void update() override;

	private:
        float dist; /**< Distance of the obj to the camera. */

        VirtualAxis mZoomAxis; /**< Input device axis used for controlling object's zoom. */

        static const float ZOOM_STEP; /**< Determines scale factor for zooming. */
	};
}
