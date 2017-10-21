#include <fstream>
#include <sstream>
#include "TSDFile.hpp"

namespace nope
{
	TSDFile::TSDFile(std::string const &name) :
		m_name(name),
		m_content((std::istreambuf_iterator<char>(std::ifstream(name))), std::istreambuf_iterator<char>()),
		m_module()
	{
	}

	TSDFile::TSDFile(TSDFile const &that) : m_name(that.m_name)
	{
	}

	TSDFile::TSDFile(TSDFile &&that) : m_name(std::move(that.m_name))
	{
	}

	TSDFile::~TSDFile() noexcept
	{
	}

	TSDFile &TSDFile::operator=(TSDFile const &that)
	{
		if (this == &that)
			return *this;
		m_name = that.m_name;
		return *this;
	}

	TSDFile &TSDFile::operator=(TSDFile &&that)
	{
		if (this == &that)
			return *this;
		m_name = std::move(that.m_name);
		return *this;
	}
}