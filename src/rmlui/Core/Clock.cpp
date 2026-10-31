#include "Clock.h"
#include "RmlUi/Core/Core.h"
#include "RmlUi/Core/SystemInterface.h"

namespace Rml {

RMLUICORE_API double Clock::GetElapsedTime()
{
	SystemInterface* system_interface = GetSystemInterface();
	if (system_interface != nullptr)
		return system_interface->GetElapsedTime();
	else
		return 0;
}

} // namespace Rml
