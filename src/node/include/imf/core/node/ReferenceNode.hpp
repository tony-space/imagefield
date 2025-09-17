#pragma once

#include <imf/core/GraphNode.hpp>
#include <imf/core/DataFlow.hpp>

namespace imf::core
{

class ReferenceNode : public GraphNode
{
public:
	// prevents base class function hiding
	using GraphNode::setInput;

	constexpr static std::string_view operation_name = "Reference";

	constexpr static std::string_view input_names[] =
	{
		"arg",
	};
	constexpr static std::string_view output_names[] =
	{
		"output",
	};

	ReferenceNode(const DataFlow& flow);
	ReferenceNode(const TypeID& type);
	ReferenceNode(const ReferenceNode&) = delete;
	ReferenceNode(ReferenceNode&&) noexcept = delete;
	ReferenceNode& operator=(const ReferenceNode&) = delete;
	ReferenceNode& operator=(ReferenceNode&&) noexcept = delete;

	std::string_view operationName() const noexcept override;

	iterator_range<const std::string_view*> inputNames() const noexcept override;
	iterator_range<const TypeID*> inputTypes() const noexcept override;
	iterator_range<const std::shared_ptr<const DataFlow>*> inputs() const noexcept override;
	
	iterator_range<const std::string_view*> outputNames() const noexcept override;
	iterator_range<const TypeID*> outputTypes() const noexcept override;
	iterator_range<const DataFlow*> outputs() const noexcept override;

	void setInput(const std::string_view& name, const DataFlow& flow) override;
	
	static std::shared_ptr<ReferenceNode> make(const DataFlow& flow);
	static std::shared_ptr<ReferenceNode> make(const TypeID& id);

private:
	std::shared_ptr<const DataFlow> m_input;
	DataFlow m_output;
};

}
