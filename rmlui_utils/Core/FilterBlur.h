#pragma once

#include "RmlUi/Core/Filter.h"
#include "RmlUi/Core/ID.h"
#include "RmlUi/Core/NumericValue.h"

namespace Rml {

class FilterBlur : public Filter {
public:
	bool Initialise(NumericValue sigma);

	CompiledFilter CompileFilter(Element* element) const override;

	void ExtendInkOverflow(Element* element, Rectanglef& scissor_region) const override;

private:
	NumericValue sigma_value;
};

class FilterBlurInstancer : public FilterInstancer {
public:
	FilterBlurInstancer();

	SharedPtr<Filter> InstanceFilter(const String& name, const PropertyDictionary& properties) override;

private:
	struct PropertyIds {
		PropertyId sigma;
	};
	PropertyIds ids;
};

} // namespace Rml
