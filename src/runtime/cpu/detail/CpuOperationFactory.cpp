#include "CpuOperationFactory.hpp"

#include <imf/core/log.hpp>

#include <map>
#include <stdexcept>

#define RegisterType(TYPE)\
std::unique_ptr<imf::core::IBackendOperation> make_##TYPE(imf::runtime::cpu::CpuRuntime&, const imf::core::destination_operands_range&, const imf::core::source_operands_range&);\
const std::string_view& operation_name_##TYPE(void) noexcept;\
map[operation_name_##TYPE()] = &(make_##TYPE)

static std::map<std::string_view, imf::runtime::cpu::operation_instantiator_t> register_standard_cpu_operations_impl()
{
	std::map<std::string_view, imf::runtime::cpu::operation_instantiator_t> map;

	RegisterType(CpuBicubicUpscale);
	RegisterType(CpuColorSpaceConvert);
	RegisterType(CpuFunctor);
	RegisterType(CpuMove);
	RegisterType(CpuTransform);

	return map;
}

namespace imf::runtime::cpu
{

static std::map<std::string_view, operation_instantiator_t>& operation_instantiators()
{
	static std::map<std::string_view, operation_instantiator_t> map(register_standard_operations());

	return map;
}

std::unique_ptr<core::IBackendOperation> make_operation(
	const std::string_view& name,
	CpuRuntime& runtime,
	const core::destination_operands_range& dest,
	const core::source_operands_range& src)
{
	auto& map = operation_instantiators();
	auto it = map.find(name);
	assert(it != map.end());
	if (it == map.end())
	{
		core::log::err("runtime") << "cpu operation '" << name << "' not registered";
		throw std::invalid_argument("cpu operation not registered");
	}
	return it->second(runtime, dest, src);
}

void register_operation(std::string_view name, operation_instantiator_t instantiator)
{
	auto _pair = operation_instantiators().emplace(name, std::move(instantiator));
	if (!_pair.second)
	{
		core::log::err("cpu") << "cpu operation '" << name << "' already registered";

		throw std::invalid_argument("cpu operation already registered");
	}
}

std::map<std::string_view, operation_instantiator_t> register_standard_operations()
{
	return register_standard_cpu_operations_impl();
}

}
