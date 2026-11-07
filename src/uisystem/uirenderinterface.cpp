#include <uirenderinterface.h>
#include <keyvalues.h>
#include <uiimage.h>

struct SourceCompiledMesh {
    std::vector<Rml::Vertex> vertices;
    std::vector<int> indices;
};

RenderInterface_SrcEng renderinterface;

static Rml::Matrix4f g_currentTransform;
static bool          g_bTransformEnabled = false;

void RenderInterface_SrcEng::Initialize()
{
    m_pUIMaterial = g_pMaterialSystem->FindMaterial("vgui/white", TEXTURE_GROUP_VGUI);
    if (m_pUIMaterial) m_pUIMaterial->IncrementReferenceCount();
}

void RenderInterface_SrcEng::Shutdown()
{
    if (m_pUIMaterial)
    {
        m_pUIMaterial->DecrementReferenceCount();
        m_pUIMaterial = nullptr;
    }
}

void RenderInterface_SrcEng::BeginFrame(int w, int h)
{
    CMatRenderContextPtr pRenderContext(g_pMaterialSystem->GetRenderContext());

    pRenderContext->MatrixMode(MATERIAL_VIEW);
    pRenderContext->PushMatrix();
    pRenderContext->LoadIdentity();

    pRenderContext->Ortho(0.0f, (float)h, (float)w, 0.0f, -1.0f, 1.0f);
    pRenderContext->CullMode(MATERIAL_CULLMODE_CW);
}

void RenderInterface_SrcEng::EndFrame()
{
    CMatRenderContextPtr pRenderContext(g_pMaterialSystem->GetRenderContext());
    pRenderContext->CullMode(MATERIAL_CULLMODE_CCW);

    pRenderContext->MatrixMode(MATERIAL_VIEW);
    pRenderContext->PopMatrix();
}

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

    CMatRenderContextPtr pRenderContext(g_pMaterialSystem->GetRenderContext());

    int vw = 0, vh = 0;
    g_pMaterialSystem->GetBackBufferDimensions(vw, vh);
    if (vw <= 0 || vh <= 0)
        return;

    BeginFrame(vw, vh);

    IMaterialVar* pBaseTextureVar = m_pUIMaterial->FindVar("$basetexture", nullptr);
    if (pBaseTextureVar)
    {
        ITexture* pTexture = nullptr;
        if (texture)
        {
            CUIImage* img = reinterpret_cast<CUIImage*>(texture);
            pTexture = img->GetSrcTexture();
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

    pRenderContext->Bind(m_pUIMaterial);

    IMesh* pDynamicMesh = pRenderContext->GetDynamicMesh(true);
    if (!pDynamicMesh)
    {
        EndFrame();
        return;
    }

    CMeshBuilder meshBuilder;

    const int nVerts = (int)pCustomMesh->vertices.size();
    const int nIndices = (int)pCustomMesh->indices.size();

    meshBuilder.Begin(pDynamicMesh, MATERIAL_TRIANGLES, nVerts, nIndices);

    for (const auto& rmlVertex : pCustomMesh->vertices)
    {
        Rml::Vector2f finalPos = rmlVertex.position + translation;

        if (g_bTransformEnabled)
        {
            const float* m = g_currentTransform.data();
            const float x = finalPos.x;
            const float y = finalPos.y;

            finalPos.x = m[0] * x + m[4] * y + m[12];
            finalPos.y = m[1] * x + m[5] * y + m[13];
        }

        meshBuilder.Position3f(finalPos.x, finalPos.y, 0.0f);
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

    EndFrame();
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
    CUIImage* img = new CUIImage(source.c_str());
    if (img->GetSrcTexture())
    {
        texture_dimensions.x = img->GetSrcTexture()->GetActualWidth();
        texture_dimensions.y = img->GetSrcTexture()->GetActualHeight();
    }
    return reinterpret_cast<Rml::TextureHandle>(img);
}

Rml::TextureHandle RenderInterface_SrcEng::GenerateTexture(Rml::Span<const unsigned char> source, Rml::Vector2i source_dimensions)
{
    CUIImage* img = new CUIImage(source, source_dimensions);
    return reinterpret_cast<Rml::TextureHandle>(img);
}

void RenderInterface_SrcEng::ReleaseTexture(Rml::TextureHandle texture)
{
    CUIImage* img = (CUIImage*)texture;
    delete img;
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
    if (transform)
    {
        g_currentTransform = *transform;
        g_bTransformEnabled = true;
    }
    else
    {
        g_bTransformEnabled = false;
    }
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