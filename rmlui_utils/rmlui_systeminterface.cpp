#include <rmlui_systeminterface.h>
#include <RmlUi/Core/Platform.h>
#include <tier0/dbg.h>

SysInterface_SrcEng sysinterface;

double SysInterface_SrcEng::GetElapsedTime()
{
    return Rml::SystemInterface::GetElapsedTime();
}

bool SysInterface_SrcEng::LogMessage(Rml::Log::Type type, const Rml::String& message)
{
    switch (type)
    {
    case Rml::Log::LT_ERROR:
        Warning("%s\n", message.c_str());
        break;
    case Rml::Log::LT_ASSERT:
        Warning("%s\n", message.c_str());
        break;
    case Rml::Log::LT_WARNING:
        Warning("%s\n", message.c_str());
        break;
    case Rml::Log::LT_INFO:
        Msg("%s\n", message.c_str());
        break;
    case Rml::Log::LT_DEBUG:
        DevMsg("%s\n", message.c_str());
        break;
    default:
        Msg("%s\n", message.c_str());
        break;
    }
    return true;
}

int SysInterface_SrcEng::TranslateString(Rml::String& translated, const Rml::String& input)
{
    if (input == "#RmlUI_Version")
    {
        translated = "Version: 1.0.0";
    }
    else
    {
        translated = input;
    }
    return 1;
}