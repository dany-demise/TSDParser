#ifndef NOPE_TDSMODULE_HPP_
# define NOPE_TSDMODULE_HPP_

#include <string>
#include <vector>
#include "TSDClassDeclaration.hpp"
#include "Tracker.hpp"

namespace nope
{
	class TSDModule
	{
	public:
		TSDModule() = delete;
		TSDModule(Tracker &tracker);
		TSDModule(std::string const &name);
		TSDModule(TSDModule const &that) = delete;
		TSDModule(TSDModule &&that);

		~TSDModule() noexcept;

		TSDModule &operator=(TSDModule const &that) = delete;
		TSDModule &operator=(TSDModule &&that);

	private:
		std::string m_name;
		std::vector<TSDClassDeclaration> m_classe;
	};
}

#endif // !NOPE_TDSMODULE_HPP_