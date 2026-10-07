#include <rmlui_renderinterface.h>
#include <materialsystem/imesh.h>
#include <materialsystem/imaterialsystem.h>
#include <materialsystem/itexture.h>
#include <materialsystem/imaterial.h>
#include <materialsystem/imaterialvar.h>
#include <vtf/vtf.h>
#include <vector>

struct SourceCompiledMesh {
    std::vector<Rml::Vertex> vertices;
    std::vector<int> indices;
};

RenderInterface_SrcEng renderinterface;
extern IMaterial* g_pDefaultUIMaterial;

static const Rml::Matrix4f* g_pCurrentTransform = nullptr;

Rml::CompiledGeometryHandle RenderInterface_SrcEng::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
{
    SourceCompiledMesh* pCustomMesh = new SourceCompiledMesh();
    pCustomMesh->vertices.assign(vertices.begin(), vertices.end());
    pCustomMesh->indices.assign(indices.begin(), indices.end());

    return reinterpret_cast<Rml::CompiledGeometryHandle>(pCustomMesh);
}

void RenderInterface_SrcEng::RenderGeometry(Rml::CompiledGeometryHandle geometry,
    Rml::Vector2f translation,
    Rml::TextureHandle texture)
{
    SourceCompiledMesh* pCustomMesh = reinterpret_cast<SourceCompiledMesh*>(geometry);
    if (!pCustomMesh || pCustomMesh->vertices.empty() || pCustomMesh->indices.empty())
        return;

    if (!g_pDefaultUIMaterial)
        return;

    CMatRenderContextPtr pRenderContext(g_pMaterialSystem->GetRenderContext());

    int vw = 0, vh = 0;
    g_pMaterialSystem->GetBackBufferDimensions(vw, vh);
    if (vw <= 0 || vh <= 0)
        return;

    pRenderContext->MatrixMode(MATERIAL_VIEW);
    pRenderContext->PushMatrix();
    pRenderContext->LoadIdentity();

    pRenderContext->MatrixMode(MATERIAL_PROJECTION);
    pRenderContext->PushMatrix();
    pRenderContext->LoadIdentity();

    pRenderContext->Ortho(0.0f, (float)vh, (float)vw, 0.0f, -1.0f, 1.0f);

    pRenderContext->MatrixMode(MATERIAL_MODEL);
    pRenderContext->PushMatrix();
    pRenderContext->LoadIdentity();

    pRenderContext->CullMode(MATERIAL_CULLMODE_CW);

    IMaterialVar* pBaseTextureVar = g_pDefaultUIMaterial->FindVar("$basetexture", nullptr);
    if (pBaseTextureVar)
    {
        ITexture* pTexture = nullptr;
        if (texture)
        {
            pTexture = reinterpret_cast<ITexture*>(texture);
        }
        else
        {
            pTexture = g_pMaterialSystem->FindTexture("vgui/white", TEXTURE_GROUP_VGUI);
            if (pTexture && pTexture->IsError())
                pTexture = nullptr;
        }

        if (pTexture)
            pBaseTextureVar->SetTextureValue(pTexture);
    }

    pRenderContext->Bind(g_pDefaultUIMaterial);

    IMesh* pDynamicMesh = pRenderContext->GetDynamicMesh(true);
    if (!pDynamicMesh)
    {
        pRenderContext->CullMode(MATERIAL_CULLMODE_CCW);
        pRenderContext->MatrixMode(MATERIAL_MODEL);
        pRenderContext->PopMatrix();
        pRenderContext->MatrixMode(MATERIAL_PROJECTION);
        pRenderContext->PopMatrix();
        pRenderContext->MatrixMode(MATERIAL_VIEW);
        pRenderContext->PopMatrix();
        return;
    }

    CMeshBuilder meshBuilder;

    const int nVerts = (int)pCustomMesh->vertices.size();
    const int nIndices = (int)pCustomMesh->indices.size();

    meshBuilder.Begin(pDynamicMesh, MATERIAL_TRIANGLES, nVerts, nIndices);

    for (const auto& rmlVertex : pCustomMesh->vertices)
    {
        Rml::Vector2f finalPos = rmlVertex.position + translation;

        if (g_pCurrentTransform)
        {
            Rml::Vector4f tp = (*g_pCurrentTransform) *
                Rml::Vector4f(finalPos.x, finalPos.y, 0.0f, 1.0f);
            meshBuilder.Position3f(tp.x, tp.y, tp.z);
        }
        else
        {
            meshBuilder.Position3f(finalPos.x, finalPos.y, 0.0f);
        }

        meshBuilder.TexCoord2f(0, rmlVertex.tex_coord.x, rmlVertex.tex_coord.y);
        meshBuilder.Color4ub(
            rmlVertex.colour.red,
            rmlVertex.colour.green,
            rmlVertex.colour.blue,
            rmlVertex.colour.alpha);

        meshBuilder.AdvanceVertex();
    }

    for (int index : pCustomMesh->indices)
    {
        meshBuilder.Index(index);
        meshBuilder.AdvanceIndex();
    }

    meshBuilder.End();
    pDynamicMesh->Draw();

    pRenderContext->CullMode(MATERIAL_CULLMODE_CCW);

    pRenderContext->MatrixMode(MATERIAL_MODEL);
    pRenderContext->PopMatrix();

    pRenderContext->MatrixMode(MATERIAL_PROJECTION);
    pRenderContext->PopMatrix();

    pRenderContext->MatrixMode(MATERIAL_VIEW);
    pRenderContext->PopMatrix();
}


void RenderInterface_SrcEng::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
{
    SourceCompiledMesh* pCustomMesh = reinterpret_cast<SourceCompiledMesh*>(geometry);
    if (pCustomMesh)
    {
        delete pCustomMesh;
    }
}

Rml::TextureHandle RenderInterface_SrcEng::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
{
    ITexture* pTexture = g_pMaterialSystem->FindTexture(source.c_str(), TEXTURE_GROUP_VGUI);
    if (!pTexture || pTexture->IsError())
    {
        return 0;
    }

    texture_dimensions.x = pTexture->GetActualWidth();
    texture_dimensions.y = pTexture->GetActualHeight();

    pTexture->IncrementReferenceCount();
    return reinterpret_cast<Rml::TextureHandle>(pTexture);
}

class CRmlTextureRegenerator : public ITextureRegenerator
{
private:
    Rml::Span<const unsigned char> m_Source;
public:
    CRmlTextureRegenerator(Rml::Span<const unsigned char> source) : m_Source(source) {}

    virtual void RegenerateTextureBits(ITexture* pTexture, IVTFTexture* pVTFTexture, Rect_t* pRect) override
    {
        unsigned char* pDestBits = pVTFTexture->ImageData(0, 0, 0);
        if (pDestBits && m_Source.data())
        {
            int width = pTexture->GetActualWidth();
            int height = pTexture->GetActualHeight();
            std::memcpy(pDestBits, m_Source.data(), width * height * 4);
        }
    }

    virtual void Release() override {}
};

Rml::TextureHandle RenderInterface_SrcEng::GenerateTexture(Rml::Span<const unsigned char> source, Rml::Vector2i source_dimensions)
{
    static int pUniqueTexId = 0;
    char szTexName[64];
    snprintf(szTexName, sizeof(szTexName), "rmlui_proc_tex_%d", pUniqueTexId++);

    ITexture* pTexture = g_pMaterialSystem->CreateProceduralTexture(
        szTexName,
        TEXTURE_GROUP_VGUI,
        source_dimensions.x,
        source_dimensions.y,
        IMAGE_FORMAT_RGBA8888,
        TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD
    );

    if (!pTexture)
        return 0;

    CRmlTextureRegenerator regenerator(source);
    pTexture->SetTextureRegenerator(&regenerator);
    pTexture->Download();
    pTexture->SetTextureRegenerator(nullptr);

    pTexture->IncrementReferenceCount();
    return reinterpret_cast<Rml::TextureHandle>(pTexture);
}

void RenderInterface_SrcEng::ReleaseTexture(Rml::TextureHandle texture)
{
    ITexture* pTexture = reinterpret_cast<ITexture*>(texture);
    if (pTexture)
    {
        pTexture->DecrementReferenceCount();
    }
}

void RenderInterface_SrcEng::EnableScissorRegion(bool enable)
{
    CMatRenderContextPtr pRenderContext(g_pMaterialSystem->GetRenderContext());
    
    if (!enable)
    {
        pRenderContext->SetScissorRect(0, 0, 0, 0, false);
    }
}

void RenderInterface_SrcEng::SetScissorRegion(Rml::Rectanglei region)
{
    CMatRenderContextPtr pRenderContext(g_pMaterialSystem->GetRenderContext());
    
    pRenderContext->SetScissorRect(
        region.Left(),
        region.Top(),
        region.Right(),
        region.Bottom(),
        true
    );
}

void RenderInterface_SrcEng::SetTransform(const Rml::Matrix4f* transform)
{
    g_pCurrentTransform = transform;
}

void RenderInterface_SrcEng::EnableClipMask(bool enable) {}

void RenderInterface_SrcEng::RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation) {}

Rml::LayerHandle RenderInterface_SrcEng::PushLayer() { return 0; }

void RenderInterface_SrcEng::CompositeLayers(Rml::LayerHandle source, Rml::LayerHandle destination, Rml::BlendMode blend_mode, Rml::Span<const Rml::CompiledFilterHandle> filters) {}

void RenderInterface_SrcEng::PopLayer() {}

Rml::TextureHandle RenderInterface_SrcEng::SaveLayerAsTexture() { return 0; }

Rml::CompiledFilterHandle RenderInterface_SrcEng::SaveLayerAsMaskImage() { return 0; }

Rml::CompiledFilterHandle RenderInterface_SrcEng::CompileFilter(const Rml::String& name, const Rml::Dictionary& parameters) { return 0; }

void RenderInterface_SrcEng::ReleaseFilter(Rml::CompiledFilterHandle filter) {}

Rml::CompiledShaderHandle RenderInterface_SrcEng::CompileShader(const Rml::String& name, const Rml::Dictionary& parameters) { return 0; }

void RenderInterface_SrcEng::RenderShader(Rml::CompiledShaderHandle shader, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) {}

void RenderInterface_SrcEng::ReleaseShader(Rml::CompiledShaderHandle shader) {}
