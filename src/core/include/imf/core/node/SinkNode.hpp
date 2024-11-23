#pragma once

#include <imf/core/GraphNode.hpp>

#include <optional>

namespace imf::core
{

class SinkNode final : public GraphNode
{
public:
	constexpr static std::string_view operation_type = "Sink";

	SinkNode(std::shared_ptr<const DataFlow> flow);
	SinkNode(const SinkNode&) = delete;
	SinkNode(SinkNode&&) noexcept = delete;
	SinkNode& operator=(const SinkNode&) = delete;
	SinkNode& operator=(SinkNode&&) noexcept = delete;

	std::string_view operationName() const noexcept override;
	iterator_range<const std::string_view*> inputNames() const noexcept override;
	iterator_range<const TypeID*> inputTypes() const noexcept override;
	iterator_range<const std::string_view*> outputNames() const noexcept override;
	iterator_range<const TypeID*> outputTypes() const noexcept override;
	iterator_range<const DataFlow*> outputs() const noexcept override;
	hast_t hash() const noexcept override;

	static std::shared_ptr<SinkNode> make(std::shared_ptr<const DataFlow> flow);

private:
	std::shared_ptr<const DataFlow> m_input;
	mutable std::optional<hast_t> m_hash;
};

}
