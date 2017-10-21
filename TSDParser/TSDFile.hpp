#ifndef NOPE_TDSFILE_HPP_
# define NOPE_TSDFILE_HPP_

# include <string>
# include <vector>
# include "TSDModule.hpp"

namespace nope
{
	class TSDFile
	{
	public:
		TSDFile() = delete;
		TSDFile(std::string const &name);
		TSDFile(TSDFile const &that);
		TSDFile(TSDFile &&that);

		~TSDFile() noexcept;

		TSDFile &operator=(TSDFile const &that);
		TSDFile &operator=(TSDFile &&that);

		std::string json() const;

		void parse();

	private:
		std::string m_name;
		std::string m_content;
		std::vector<TSDModule> m_module;
	};
}

#endif // !NOPE_TDSFILE_HPP_