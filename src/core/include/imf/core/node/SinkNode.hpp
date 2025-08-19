#pragma once

#include <imf/core/GraphNode.hpp>

#include <optional>

namespace imf::core
{

class SinkNode final : public GraphNode
{
public:
	constexpr static std::string_view operation_name = "Sink";

	SinkNode(const DataFlow& flow);
	SinkNode(const SinkNode&) = delete;
	SinkNode(SinkNode&&) noexcept = delete;
	SinkNode& operator=(const SinkNode&) = delete;
	SinkNode& operator=(SinkNode&&) noexcept = delete;

	// getters
	std::string_view operationName() const noexcept override;
	iterator_range<const std::string_view*> inputNames() const noexcept override;
	iterator_range<const TypeID*> inputTypes() const noexcept override;
	iterator_range<const std::shared_ptr<const DataFlow>*> inputs() const noexcept override;

	iterator_range<const std::string_view*> outputNames() const noexcept override;
	iterator_range<const TypeID*> outputTypes() const noexcept override;
	iterator_range<const DataFlow*> outputs() const noexcept override;

	// setters
	void setInput(const std::string_view& name, const DataFlow& flow) override;

	static std::shared_ptr<SinkNode> make(const DataFlow& flow);

private:
	std::shared_ptr<const DataFlow> m_input;
};

}
