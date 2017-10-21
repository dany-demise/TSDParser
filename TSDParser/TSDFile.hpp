#ifndef NOPE_TDSFILE_HPP_
# define NOPE_TSDFILE_HPP_

# include <string>
# include <vector>
# include "TSDModule.hpp"
# include "Tracker.hpp"

namespace nope
{
	class TSDFile
	{
	public:
		TSDFile() = delete;
		TSDFile(std::string const &name);
		TSDFile(TSDFile const &that) = delete;
		TSDFile(TSDFile &&that) = delete;

		~TSDFile() noexcept;

		TSDFile &operator=(TSDFile const &that) = delete;
		TSDFile &operator=(TSDFile &&that) = delete;

		std::string json() const;

		void parse();

	private:
		Tracker m_tracker;
		std::vector<TSDModule> m_module;
	};
}

#endif // !NOPE_TDSFILE_HPP_