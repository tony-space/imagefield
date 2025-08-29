#pragma once

#include <imf/runtime/cpu/CpuRuntime.hpp>

#include <imf/core/operand.hpp>
#include <imf/core/IBackendOperation.hpp>

#include <functional>
#include <memory>
#include <string_view>

namespace imf::runtime::cpu
{

class CpuRuntime;

[[nodiscard]] std::unique_ptr<core::IBackendOperation> make_operation(
	const std::string_view& name,
	CpuRuntime&,
	const core::destination_operands_range&,
	const core::source_operands_range&);


using operation_instantiator_t = std::function<std::unique_ptr<imf::core::IBackendOperation>(CpuRuntime&, const imf::core::destination_operands_range&, const imf::core::source_operands_range&)>;

void register_operation(std::string_view name, operation_instantiator_t instantiator);
std::map<std::string_view, operation_instantiator_t> register_standard_operations();

}

#define DeclareCpuOperation(TYPE)\
std::unique_ptr<imf::core::IBackendOperation> make_##TYPE(\
	imf::runtime::cpu::CpuRuntime& runtime,\
	const imf::core::destination_operands_range& dst,\
	const imf::core::source_operands_range& src);\
std::unique_ptr<imf::core::IBackendOperation> make_##TYPE(\
	imf::runtime::cpu::CpuRuntime& runtime,\
	const imf::core::destination_operands_range& dst,\
	const imf::core::source_operands_range& src)\
{\
	return std::make_unique<imf::runtime::cpu::TYPE>(runtime, dst, src); \
}\
const std::string_view& operation_name_##TYPE() noexcept;\
const std::string_view& operation_name_##TYPE() noexcept\
{\
	return imf::runtime::cpu::TYPE::operation_name;\
}

