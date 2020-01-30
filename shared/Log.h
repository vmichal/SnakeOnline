#pragma once
#ifndef LOG_H
#define LOG_H

#include <string>
#include <string_view>
#include <array>
#include <queue>
#include <sstream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unordered_map>
#include <fstream>
#include <memory>

#define TEMPLATED

#ifdef TEMPLATED
std::string getTimeString(std::string_view format);
#endif

class Log {
public:
	struct formatEnd_t {};
	/*Okamžitì ukonèí formátování a odešle zprávu k vypsání*/
	static inline constexpr formatEnd_t formatEnd = {};
	enum class Priority { err, warning, log, prompt, input };


	template<class _Ty,
		class... _Types,
		std::enable_if_t<!std::is_array_v<_Ty>, int>>
		friend std::unique_ptr<_Ty> std::make_unique(_Types&&... _Args);

private:
	template<Priority P> class Formatter;

	static std::unique_ptr<Log> instance_;
	static std::array<Priority, 5> priorities;

	bool running_ = true;
	std::unordered_map<Priority, std::queue<std::pair<std::string, std::string>>> fronty_;
	std::unordered_map<Priority, std::unique_ptr<std::mutex>> mutexPtrs_;
	std::condition_variable condVar_;
	std::ofstream fileStream_;
	std::thread vlakno_;

#ifdef TEMPLATED

	template<Priority p, typename T, typename = std::enable_if_t<std::is_constructible_v<std::string, T>>>
	void insertString(T&& str) {
		std::pair<std::string, std::string> pair = std::make_pair(getTimeString("%H:%M:%S.%ms"), std::forward<T>(str));
		{
			std::lock_guard guard(*mutexPtrs_[p]);
			fronty_[p].push(std::move(pair));
		}
		flush();
	}

#else 

	void insertString(std::string&& str, Priority priority);
	void insertString(const std::string& str, Priority priority);

#endif

	template<Priority P>
	Formatter<P> createFormatter() {
		return Formatter<P>(*this);
	}

	void vypisovaciLoop();

	Log(std::string_view header, std::string_view suffix);
public:
	static void initialize(std::string_view header, std::string_view suffix);
	static void flush();
	static bool isInitialized();

	~Log();
	Log(Log&&) = delete;
	Log(const Log&) = delete;
	Log& operator=(const Log&) = delete;
	Log& operator=(Log&&) = delete;

	static Formatter<Priority::log> log();		
	static Formatter<Priority::err> error();
	static Formatter<Priority::warning> warning();
	static Formatter<Priority::prompt> prompt();   
	static Formatter<Priority::input> input();

#ifdef TEMPLATED
	template<typename T>
	static void log(T&& str) {
		instance_->insertString<Priority::log>(std::forward<T>(str));
	}
#else
	static void log(const std::string& str);
	static void log(std::string&& str);
#endif


#ifdef TEMPLATED
	template<typename T>
	static void error(T&& str) {
		instance_->insertString<Priority::err>(std::forward<T>(str));
	}
#else
	static void error(const std::string& str);
	static void error(std::string&& str);
#endif


#ifdef TEMPLATED
	template<typename T>
	static void warning(T&& str) {
		instance_->insertString<Priority::warning>(std::forward<T>(str));
	}
#else
	static void warning(const std::string& str);
	static void warning(std::string&& str);
#endif


#ifdef TEMPLATED
	template<typename T>
	static void prompt(T&& str) {
		instance_->insertString<Priority::prompt>(std::forward<T>(str));
	}
#else
	static void prompt(const std::string& str);
	static void prompt(std::string&& str);
#endif

private:
	template<Priority P>
	struct FormatterBase {
	protected:
		Log & myLog_;
		std::unique_ptr<std::mutex> mutexPtr_; //unique_ptr, aby byl FormatterBase move-constructible, mutex by byl immutable
		std::ostringstream stringStream_;

	public:
		FormatterBase(Log& log)
			: myLog_(log), mutexPtr_(std::make_unique<std::mutex>()) {}

		virtual ~FormatterBase() noexcept {
			*this << formatEnd;
		}

		FormatterBase(FormatterBase&& moveCtr) = default;
		FormatterBase(const FormatterBase& copyCtor)
			: myLog_(copyCtor.myLog_), mutexPtr_(std::make_unique()), stringStream_(copyCtor.str()) {}


		void operator<<(formatEnd_t) {
			std::lock_guard<std::mutex> guard(*mutexPtr_);
			if (std::string str = stringStream_.str(); str.size())
				myLog_.insertString<P>(std::move(str));
			stringStream_.str("");
		}
	};

	template<Priority P>
	struct Formatter : public FormatterBase<P> {
		using MyBase = FormatterBase<P>;
		using MyBase::operator<<;

		Formatter(Log& log) : MyBase(log) {}
		~Formatter() override = default;

		Formatter(const Formatter<P>&) = delete;
		Formatter(Formatter<P>&&) = default;
		Formatter& operator=(const Formatter<P>&) = delete;
		Formatter& operator=(Formatter<P>&&) = default;

		template<typename T>
		Formatter& operator<<(T&& t) {
			std::lock_guard<std::mutex> guard(*this->mutexPtr_);
			this->stringStream_ << t;
			return *this;
		}

		Formatter& operator<<(std::ostream& (*ptr)(std::ostream&)) {
			std::lock_guard<std::mutex> guard(*this->mutexPtr_);
			ptr(this->stringStream_);
			return *this;
		}
	};

	template<>
	struct Formatter<Priority::input> : public FormatterBase<Priority::input> {
		using MyBase = FormatterBase<Priority::input>;
		using MyBase::operator<<;

		Formatter(Log& log) : MyBase(log) {}
		~Formatter() override = default;

		Formatter(const Formatter<Priority::input>&) = delete;
		Formatter(Formatter<Priority::input>&&) = default;
		Formatter& operator=(const Formatter<Priority::input>&) = delete;
		Formatter& operator=(Formatter<Priority::input>&&) = default;

		template<typename T>
		Formatter& operator>>(T& t);

		std::string getline();
	};
};

std::ostream& operator<<(std::ostream&, Log::Priority);


#endif