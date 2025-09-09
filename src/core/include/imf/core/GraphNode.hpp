#pragma once

#include <imf/core/iterator_range.hpp>
#include <imf/core/TypeQualifier.hpp>
#include <imf/core/unique_id_t.hpp>

#include <memory>

namespace imf::core
{

class DataFlow;
class TypeID;

class GraphNode : public std::enable_shared_from_this<GraphNode>
{
public:
	virtual ~GraphNode() = default;

	// getters
	unique_id_t instanceId() const noexcept { return m_instanceId; }

	virtual std::string_view operationName() const noexcept = 0;

	virtual iterator_range<const std::string_view*> inputNames() const noexcept = 0;
	virtual iterator_range<const TypeID*> inputTypes() const noexcept = 0;
	virtual iterator_range<const std::shared_ptr<const DataFlow>*> inputs() const noexcept = 0;
	
	virtual iterator_range<const std::string_view*> outputNames() const noexcept = 0;
	virtual iterator_range<const TypeID*> outputTypes() const noexcept = 0;
	virtual iterator_range<const DataFlow*> outputs() const noexcept = 0;

	//setters
	virtual void setInput(const std::string_view& name, const DataFlow& flow) = 0;
	
	template<typename T> void setInput(std::string_view name, TypeQualifier qualifier, T&& value);
	template<typename T> T* as();
	template<typename T> const T* as() const;
	template<typename T> T& is();
	template<typename T> const T& is() const;

private:
	unique_id_t m_instanceId{ make_unique_id() };
};

template<typename T>
T* GraphNode::as()
{
	if (T::operation_name == operationName())
	{
		return static_cast<T*>(this);
	}
	return nullptr;
}

template<typename T>
const T* GraphNode::as() const
{
	if (T::operation_name == operationName())
	{
		return static_cast<const T*>(this);
	}
	return nullptr;
}

template<typename T>
T& GraphNode::is()
{
	assert(T::operation_name == operationName());
	if (T::operation_name == operationName())
	{
		return static_cast<T&>(*this);
	}

	throw std::bad_cast();
}

template<typename T>
const T& GraphNode::is() const
{
	assert(T::operation_name == operationName());
	if (T::operation_name == operationName())
	{
		return static_cast<const T&>(*this);
	}
	throw std::bad_cast();
}

}
