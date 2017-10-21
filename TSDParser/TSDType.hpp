#ifndef NOPE_TSDTYPE_HPP_
# define NOPE_TSDTYPE_HPP_

# include "TSDClass.hpp"

namespace nope
{
	class TSDType
	{
	public:
		static std::vector<TSDClass> classes;

		static void generateBasicTypes();

	private:
		std::vector<std::string> m_callerPath;
		std::vector<std::string> m_path;
		std::string m_name;
	};
}

#endif // !NOPE_TSDTYPE_HPP_