#include "LogDefault.h"
#include "RmlUi/Core/StringUtilities.h"

#include <stdio.h>

namespace Rml {

bool LogDefault::LogMessage(Log::Type /*type*/, const String& message)
{
	#ifdef RMLUI_PLATFORM_EMSCRIPTEN
	puts(message.c_str());
	#else
	fprintf(stderr, "%s\n", message.c_str());
	#endif
	return true;
}

} // namespace Rml
