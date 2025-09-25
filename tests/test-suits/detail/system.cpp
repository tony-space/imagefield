#include <imf/tests/system.hpp>

#include <boost/test/test_tools.hpp>

namespace imf::tests
{

const std::filesystem::path& project_root()
{
	static std::filesystem::path cached;
	if (!cached.empty())
	{
		return cached;
	}

	return cached = build_dir().parent_path();
}


const std::filesystem::path& project_assets()
{
	static std::filesystem::path cached = project_root() / "assets";
	return cached;
}

const std::filesystem::path& build_dir()
{
	static std::filesystem::path cached;
	if (!cached.empty())
	{
		return cached;
	}

	for (auto workingDirectory = std::filesystem::current_path(); !workingDirectory.empty(); workingDirectory = workingDirectory.parent_path())
	{
		auto candidate = workingDirectory.parent_path() / "assets/png";
		if (std::filesystem::is_directory(candidate))
		{
			cached = std::move(workingDirectory);
			return cached;
		}
	}

	BOOST_TEST_REQUIRE(!cached.empty());
	throw std::runtime_error("cannot find project root location");
}

const std::filesystem::path& output_dir()
{
	static std::filesystem::path cached;
	if (!cached.empty())
	{
		return cached;
	}

	cached = build_dir() / "tests-outputs";
	std::filesystem::create_directory(cached);
	return cached;
}


}
