#ifndef NOPE_TRACKER_HPP_
# define NOPE_TRACKER_HPP_

# include <string>
# include <utility>

namespace nope
{
	class Tracker
	{
	public:
		Tracker() = delete;
		Tracker(std::string const &filename);
		Tracker(Tracker const &that) = delete;
		Tracker(Tracker &&that) = delete;

		~Tracker() noexcept;

		Tracker &operator=(Tracker const &that) = delete;
		Tracker &operator=(Tracker &&that) = delete;

		std::string const &content() const;
		
		Tracker &get(char &c);
		Tracker const &peek(char &c) const;
		Tracker &peek(char &c);
		std::string_view peek(int n) const;
		void unget();

		std::pair<std::uint32_t, std::uint32_t> position() const;

		operator bool() const;

		void error(std::string const &message) const;

	private:
		std::string m_filename;
		std::string m_content;

		std::int32_t m_cursor;
	};
}

#endif // !NOPE_TRACKER_HPP_