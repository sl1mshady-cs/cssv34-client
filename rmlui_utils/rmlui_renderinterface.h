#ifndef RMLUI_RENDERINTERFACE_H
#define RMLUI_RENDERINTERFACE_H

#if defined(_WIN32)
#pragma once
#endif

#include <RmlUi/Core.h>
#include <RmlUi/Core/Span.h>

class RenderInterface_SrcEng : public Rml::RenderInterface
{
public:
	virtual Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices);
	
	virtual void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture);

	virtual void ReleaseGeometry(Rml::CompiledGeometryHandle geometry);

	virtual Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source);

	virtual Rml::TextureHandle GenerateTexture(
		Rml::Span<const unsigned char> source,
		Rml::Vector2i source_dimensions
	);

	virtual void ReleaseTexture(Rml::TextureHandle texture);

	virtual void EnableScissorRegion(bool enable);

	virtual void SetScissorRegion(Rml::Rectanglei region);

	virtual void EnableClipMask(bool enable);

	virtual void RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation);

	virtual void SetTransform(const Rml::Matrix4f* transform);

	virtual Rml::LayerHandle PushLayer();

	virtual void CompositeLayers(Rml::LayerHandle source, Rml::LayerHandle destination, Rml::BlendMode blend_mode, Rml::Span<const Rml::CompiledFilterHandle> filters);

	virtual void PopLayer();

	virtual Rml::TextureHandle SaveLayerAsTexture();

	virtual Rml::CompiledFilterHandle SaveLayerAsMaskImage();

	virtual Rml::CompiledFilterHandle CompileFilter(const Rml::String& name, const Rml::Dictionary& parameters);

	virtual void ReleaseFilter(Rml::CompiledFilterHandle filter);

	virtual Rml::CompiledShaderHandle CompileShader(const Rml::String& name, const Rml::Dictionary& parameters);

	virtual void RenderShader(Rml::CompiledShaderHandle shader, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture);

	virtual void ReleaseShader(Rml::CompiledShaderHandle shader);
};

extern RenderInterface_SrcEng renderinterface;

#endif