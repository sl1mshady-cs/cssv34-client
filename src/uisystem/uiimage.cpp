#include "uiimage.h"
#include <cstdio>
#include <cstring>

static void MakeUniqueTextureName(const char* prefix, char* out, size_t outSize)
{
    static int s_UniqueTexId = 0;
    snprintf(out, outSize, "%s_%d", prefix, s_UniqueTexId++);
}

CUIImage::CUIImage(const char* file)
{
    if (!file || !file[0])
        return;

    const char* ext = strrchr(file, '.');
    if (ext && V_stricmp(ext, ".vtf") == 0)
        LoadVTF(file);
    else
        LoadImage(file);
}

CUIImage::CUIImage(Rml::Span<const unsigned char> source, Rml::Vector2i source_dimensions)
{
    if (source_dimensions.x <= 0 || source_dimensions.y <= 0)
    {
        Warning("[UI Texture] Invalid dimensions %dx%d\n",
            source_dimensions.x, source_dimensions.y);
        return;
    }

    if (!source.data())
    {
        Warning("[UI Texture] Null span data\n");
        return;
    }

    const size_t requiredSize =
        static_cast<size_t>(source_dimensions.x) *
        static_cast<size_t>(source_dimensions.y) * 4;

    if (source.size() < requiredSize)
    {
        Warning("[UI Texture] Span too small: got %zu, need %zu\n",
            source.size(), requiredSize);
        return;
    }

    m_pStbTexturePixels = static_cast<stbi_uc*>(malloc(requiredSize));
    if (!m_pStbTexturePixels)
    {
        Warning("[UI Texture] Out of memory allocating %zu bytes\n", requiredSize);
        return;
    }
    memcpy(m_pStbTexturePixels, source.data(), requiredSize);

    m_Width = source_dimensions.x;
    m_Height = source_dimensions.y;

    char szTexName[64];
    MakeUniqueTextureName("rmlui_proc_tex", szTexName, sizeof(szTexName));

    m_pTexture = g_pMaterialSystem->CreateProceduralTexture(
        szTexName,
        TEXTURE_GROUP_VGUI,
        m_Width,
        m_Height,
        IMAGE_FORMAT_RGBA8888,
        TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_NOMIP |
        TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL | TEXTUREFLAGS_SINGLECOPY
    );

    if (!m_pTexture)
    {
        Warning("[UI Texture] Failed to create procedural texture '%s'\n", szTexName);
        free(m_pStbTexturePixels);
        m_pStbTexturePixels = nullptr;
        return;
    }

    m_pTexture->SetTextureRegenerator(new CStbTextureRegenerator(this));
    m_pTexture->Download();
}

CUIImage::~CUIImage()
{
    if (m_pTexture)
    {
        m_pTexture->Release();
        m_pTexture = nullptr;
    }

    if (m_pVTFTexture)
    {
        DestroyVTFTexture(m_pVTFTexture);
        m_pVTFTexture = nullptr;
    }

    if (m_pStbTexturePixels)
    {
        stbi_image_free(m_pStbTexturePixels);
        m_pStbTexturePixels = nullptr;
    }
}

void CUIImage::LoadVTF(const char* file)
{
    Rml::FileHandle hFile = fileinterface.Open(file);
    if (!hFile)
    {
        Warning("[UI Texture] Failed to open '%s'\n", file);
        return;
    }

    size_t fSize = fileinterface.Length(hFile);
    if (fSize == 0)
    {
        fileinterface.Close(hFile);
        return;
    }

    unsigned char* data = new unsigned char[fSize];
    fileinterface.Read(data, fSize, hFile);
    fileinterface.Close(hFile);

    CUtlBuffer buf(data, fSize);
    IVTFTexture* pVTF = CreateVTFTexture();
    const bool bParsed = pVTF->Unserialize(buf);
    delete[] data;

    if (!bParsed)
    {
        DestroyVTFTexture(pVTF);
        Warning("[UI Texture] Failed to parse VTF '%s'\n", file);
        return;
    }

    m_pVTFTexture = pVTF;
    m_Width = pVTF->Width();
    m_Height = pVTF->Height();

    char szTexName[64];
    MakeUniqueTextureName("ui_vtf_tex", szTexName, sizeof(szTexName));

    m_pTexture = g_pMaterialSystem->CreateProceduralTexture(
        szTexName,
        TEXTURE_GROUP_VGUI,
        m_Width,
        m_Height,
        pVTF->Format(),
        TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_NOMIP |
        TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL | TEXTUREFLAGS_SINGLECOPY
    );

    if (!m_pTexture)
    {
        DestroyVTFTexture(m_pVTFTexture);
        m_pVTFTexture = nullptr;
        Warning("[UI Texture] Failed to create texture for '%s'\n", file);
        return;
    }

    m_pTexture->SetTextureRegenerator(new CVTFTextureRegenerator(this));
    m_pTexture->Download();
}

void CUIImage::LoadImage(const char* file)
{
    Rml::FileHandle hFile = fileinterface.Open(file);
    if (!hFile)
    {
        Warning("[UI Texture] Failed to open '%s'\n", file);
        return;
    }

    size_t fileLen = fileinterface.Length(hFile);
    if (fileLen == 0)
    {
        fileinterface.Close(hFile);
        return;
    }

    unsigned char* fData = new unsigned char[fileLen];
    fileinterface.Read(fData, fileLen, hFile);
    fileinterface.Close(hFile);

    int width = 0, height = 0, channels = 0;
    m_pStbTexturePixels = stbi_load_from_memory(
        fData, static_cast<int>(fileLen), &width, &height, &channels, STBI_rgb_alpha);
    delete[] fData;

    if (!m_pStbTexturePixels)
    {
        Warning("[UI Texture] stb: %s\n", stbi_failure_reason());
        return;
    }

    m_Width = width;
    m_Height = height;

    char szTexName[64];
    MakeUniqueTextureName("ui_stb_tex", szTexName, sizeof(szTexName));

    m_pTexture = g_pMaterialSystem->CreateProceduralTexture(
        szTexName,
        TEXTURE_GROUP_VGUI,
        m_Width,
        m_Height,
        IMAGE_FORMAT_RGBA8888,
        TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_NOMIP |
        TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL | TEXTUREFLAGS_SINGLECOPY
    );

    if (!m_pTexture)
    {
        Warning("[UI Texture] Failed to create procedural texture for '%s'\n", file);
        stbi_image_free(m_pStbTexturePixels);
        m_pStbTexturePixels = nullptr;
        return;
    }

    m_pTexture->SetTextureRegenerator(new CStbTextureRegenerator(this));
    m_pTexture->Download();
}

void CUIImage::CStbTextureRegenerator::RegenerateTextureBits(
    ITexture* pTexture, IVTFTexture* pVTFTexture, Rect_t* pRect)
{
    if (!m_pOwner || !m_pOwner->m_pStbTexturePixels)
        return;

    const int nWidth = pVTFTexture->Width();
    const int nHeight = pVTFTexture->Height();

    if (nWidth != m_pOwner->m_Width || nHeight != m_pOwner->m_Height)
    {
        Warning("[UI Texture] Size mismatch: VTF %dx%d, image %dx%d\n",
            nWidth, nHeight, m_pOwner->m_Width, m_pOwner->m_Height);
        return;
    }

    CPixelWriter pixelWriter;
    pixelWriter.SetPixelMemory(
        pVTFTexture->Format(),
        pVTFTexture->ImageData(),
        pVTFTexture->RowSizeInBytes(0)
    );

    for (int y = pRect->y; y < pRect->height; ++y)
    {
        pixelWriter.Seek(pRect->x, y);
        for (int x = pRect->x; x < pRect->width; ++x)
        {
            const int idx = (y * m_pOwner->m_Width + x) * 4;
            const stbi_uc* pixel = &m_pOwner->m_pStbTexturePixels[idx];
            pixelWriter.WritePixel(pixel[0], pixel[1], pixel[2], pixel[3]);
        }
    }
}

void CUIImage::CVTFTextureRegenerator::RegenerateTextureBits(
    ITexture* pTexture, IVTFTexture* pVTFTexture, Rect_t* pRect)
{
    if (!m_pOwner || !m_pOwner->m_pVTFTexture)
        return;

    IVTFTexture* src = m_pOwner->m_pVTFTexture;

    if (src->Width() != pVTFTexture->Width() ||
        src->Height() != pVTFTexture->Height() ||
        src->Format() != pVTFTexture->Format())
    {
        Warning("[UI Texture] VTF regenerator: format/size mismatch\n");
        return;
    }

    const int srcStride = src->RowSizeInBytes(0);
    const int dstStride = pVTFTexture->RowSizeInBytes(0);
    if (srcStride != dstStride)
    {
        Warning("[UI Texture] VTF regenerator: stride mismatch\n");
        return;
    }

    const unsigned char* srcData = src->ImageData();
    unsigned char* dstData = pVTFTexture->ImageData();
    if (!srcData || !dstData)
        return;

    for (int y = pRect->y; y < pRect->height; ++y)
    {
        memcpy(dstData + y * dstStride,
            srcData + y * srcStride,
            srcStride);
    }
}