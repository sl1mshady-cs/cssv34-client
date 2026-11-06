#ifndef RMLUI_SYSTEMINTERFACE_H
#define RMLUI_SYSTEMINTERFACE_H

#if defined(_WIN32)
#pragma once
#endif

#include <RmlUi/Core.h>

class SysInterface_SrcEng : public Rml::SystemInterface
{
public:
    virtual double GetElapsedTime() override;
    virtual bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;
    virtual int TranslateString(Rml::String& translated, const Rml::String& input) override;
};

extern SysInterface_SrcEng sysinterface;

#endif