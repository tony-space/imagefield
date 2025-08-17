#pragma once

#include <imf/core/iterator_range.hpp>

#include <memory>

namespace imf::core
{

class DataFlow;
class TypeID;

class GraphNode : public std::enable_shared_from_this<GraphNode>
{
public:
	using hast_t = size_t;

	virtual ~GraphNode() = default;

	// getters
	virtual std::string_view operationName() const noexcept = 0;

	virtual iterator_range<const std::string_view*> inputNames() const noexcept = 0;
	virtual iterator_range<const TypeID*> inputTypes() const noexcept = 0;
	virtual iterator_range<const std::shared_ptr<const DataFlow>*> inputs() const noexcept = 0;
	
	virtual iterator_range<const std::string_view*> outputNames() const noexcept = 0;
	virtual iterator_range<const TypeID*> outputTypes() const noexcept = 0;
	virtual iterator_range<const DataFlow*> outputs() const noexcept = 0;

	virtual hast_t hash() const noexcept = 0;

	//setters
	virtual void setInput(const std::string_view& name, const DataFlow& flow) = 0;

private:
};

}
