#include <imf/core/DataFlow.hpp>
#include <imf/core/node/FunctorNode.hpp>
#include <imf/core/node/PlaceholderNode.hpp>

#include <string>

namespace imf::core
{
FunctorNode::FunctorNode
(
	functor_t functor,
	small_vector_t<TypeID> inputTypes,
	small_vector_t<TypeID> outputTypes,
	small_vector_t<std::shared_ptr<const DataFlow>> inputFlows
)
{
	m_inputNames.resize(inputTypes.size());
	m_inputNamesView.resize(inputTypes.size());
	m_inputFlows.resize(inputTypes.size());

	m_inputNamesView[0] = m_inputNames[0] = "functor";

	for (std::size_t i = 1; i < inputTypes.size(); ++i)
	{
		m_inputNames[i] = "arg" + std::to_string(i - 1);
		m_inputNamesView[i] = m_inputNames[i];
	}
	m_inputTypes = std::move(inputTypes);

	m_outputNames.resize(outputTypes.size());
	m_outputNamesView.resize(outputTypes.size());
	m_outputFlows.reserve(outputTypes.size());

	for (std::size_t i = 0; i < outputTypes.size(); ++i)
	{
		m_outputNamesView[i] = m_outputNames[i] = "output" + std::to_string(i);
		m_outputFlows.emplace_back(*this, outputTypes[i]);
	}
	m_outputTypes = std::move(outputTypes);

	setInput("functor", TypeQualifier::Constant, std::move(functor));
	for (std::size_t i = 0; i < inputFlows.size(); ++i)
	{
		setInput(("arg" + std::to_string(i)), *inputFlows[i]);
	}
}
std::string_view FunctorNode::operationName() const noexcept
{
	return "Functor";
}
iterator_range<const std::string_view*> FunctorNode::inputNames() const noexcept
{
	return { m_inputNamesView.data(), m_inputNamesView.data() + m_inputNamesView.size() };
}
iterator_range<const TypeID*> FunctorNode::inputTypes() const noexcept
{
	return { m_inputTypes.data(), m_inputTypes.data() + m_inputTypes.size() };
}
iterator_range<const std::shared_ptr<const DataFlow>*> FunctorNode::inputs() const noexcept
{
	return { m_inputFlows.data(), m_inputFlows.data() + m_inputFlows.size() };
}
iterator_range<const std::string_view*> FunctorNode::outputNames() const noexcept
{
	return { m_outputNamesView.data(), m_outputNamesView.data() + m_outputNamesView.size() };
}
iterator_range<const TypeID*> FunctorNode::outputTypes() const noexcept
{
	return { m_outputTypes.data(), m_outputTypes.data() + m_outputTypes.size() };
}
iterator_range<const DataFlow*> FunctorNode::outputs() const noexcept
{
	return { m_outputFlows.data(), m_outputFlows.data() + m_outputFlows.size() };
}
void FunctorNode::setInput(const std::string_view& name, const DataFlow& flow)
{
	auto it = std::find(std::begin(m_inputNamesView), std::end(m_inputNamesView), name);
	
	if (it == std::end(m_inputNamesView))
	{
		throw std::out_of_range("no such parameter");
	}

	auto idx = std::distance(std::begin(m_inputNamesView), it);

	if (flow.dataType() != m_inputTypes[idx])
	{
		throw std::invalid_argument("invalid input parameter type");
	}

	m_inputFlows[idx] = flow.sharedPtr();
}
}
