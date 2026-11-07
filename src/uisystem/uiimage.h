#pragma once
#include "uisystem_internal.h"
#include "uifileinterface.h"
#include "pixelwriter.h"

#include <materialsystem/imaterialsystem.h>
#include <materialsystem/itexture.h>
#include <vtf/vtf.h>

#include <stb/stb_image.h>

class IVTFTexture;

class CUIImage
{
public:
    explicit CUIImage(const char* file);
    explicit CUIImage(Rml::Span<const unsigned char> source, Rml::Vector2i source_dimensions);
    ~CUIImage();

    ITexture* GetSrcTexture() const { return m_pTexture; }
    int       GetWidth()      const { return m_Width; }
    int       GetHeight()     const { return m_Height; }

private:
    class CStbTextureRegenerator : public ITextureRegenerator
    {
    public:
        CStbTextureRegenerator(CUIImage* pOwner) : m_pOwner(pOwner) {}
        virtual void RegenerateTextureBits(ITexture* pTexture, IVTFTexture* pVTFTexture, Rect_t* pRect) override;
        virtual void Release() override { delete this; }
    private:
        CUIImage* m_pOwner;
    };

    class CVTFTextureRegenerator : public ITextureRegenerator
    {
    public:
        CVTFTextureRegenerator(CUIImage* pOwner) : m_pOwner(pOwner) {}
        virtual void RegenerateTextureBits(ITexture* pTexture, IVTFTexture* pVTFTexture, Rect_t* pRect) override;
        virtual void Release() override { delete this; }
    private:
        CUIImage* m_pOwner;
    };

    void LoadVTF(const char* file);
    void LoadImage(const char* file);

    ITexture* m_pTexture = nullptr;
    stbi_uc* m_pStbTexturePixels = nullptr;
    IVTFTexture* m_pVTFTexture = nullptr;
    int          m_Width = 0;
    int          m_Height = 0;

    CUIImage(const CUIImage&) = delete;
    CUIImage& operator=(const CUIImage&) = delete;
};