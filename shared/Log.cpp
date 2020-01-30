#include "Log.h"
#include <iostream>				   
#include <chrono>  
#include <string_view>
#include <filesystem>
#include <regex>


std::unique_ptr<Log> Log::instance_;
std::array<Log::Priority, 5> Log::priorities;


	/*vrací string podle formátu s placeholdery nahrazenými skuteènými èísly;
	placeholdery:
	%ms pro milisekundy
	%us pro mikrosekundy
	%ns pro nanosekundy*/
std::string getTimeString(std::string_view format) {
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::time_t cas = std::chrono::system_clock::to_time_t(now);
	std::tm time = *std::localtime(&cas);
	constexpr size_t BUFFER_SIZE = 0x200;
	char formatBuffer[BUFFER_SIZE] = { 0 };
	std::strncpy(formatBuffer, format.data(), format.size());

	static std::regex msRegex("%ms", std::regex::optimize), 
		nsRegex("%ns", std::regex::optimize), 
		usRegex("%us", std::regex::optimize);

	//pokud si vyžádá caller i milisekundy
	std::string nanoStr = std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count());

	std::string nano(nanoStr.end() - 3, nanoStr.end());
	std::string mikro(nanoStr.end() - 6, nanoStr.end() - 3);
	std::string mili(nanoStr.end() - 9, nanoStr.end() - 6);

	
	std::pair<std::regex&, std::string&> units[3] = {
		{msRegex, mili},
		{usRegex, mikro},
		{nsRegex, nano}
	};
	for (auto &[regex, value] : units)
		std::regex_replace(std::begin(formatBuffer), std::begin(formatBuffer), std::end(formatBuffer), regex, value);

	char buffer[BUFFER_SIZE];
	std::strftime(buffer, BUFFER_SIZE, formatBuffer, &time);
	return buffer;
}

std::filesystem::path createLogFile(std::string_view header, std::string_view suffix) {
	size_t highestIndex = 0;
	for (std::filesystem::directory_entry entry : std::filesystem::directory_iterator(".")) {
		std::string path = entry.path().string();
		if (std::size_t pozice = path.find(header); pozice != std::string::npos) {
			pozice += header.size() + 4; //4 for lenght of string "-Log"
			if (pozice >= path.size() || !isdigit(static_cast<unsigned char>(path[pozice])))
				continue;
			if (size_t index = std::atoi(path.c_str() + pozice); index > highestIndex)
				highestIndex = index;
		}
	}
	std::string cesta(header);
	cesta.append("-Log")
		.append(std::to_string(highestIndex + 1));
	std::filesystem::path path(cesta);
	if (std::filesystem::exists(path))
		std::cerr << __FUNCTION__ << ": Generated path " << path << " already exists.";
	return path;
}


std::ostream& operator<<(std::ostream& stream, Log::Priority p) {
	using Priority = Log::Priority;
	switch (p) {
		case Priority::err:
			return stream << "[E]";
		case Priority::log:
			return stream << "  [L]";
		case Priority::warning:
			return stream << "[W]";
		case Priority::prompt:
			return stream << "  [P]";
		case Priority::input:
			return stream << "  [I]";
		default:
			Log::error() << __FUNCTION__ << ": Not implemented token " << int(p);
			return stream;
	}
}

Log::Log(std::string_view header, std::string_view suffix) {
	if (isInitialized())
		error() << __FUNCTION__ << ": Log již zøejmì byl konstruován";
	fileStream_.open(createLogFile(header, suffix));
	fileStream_ << getTimeString("File stream opened at %H:%M:%S.%ms.%us.%ns, on %A %e.%m.%Y\n");
	for (Priority priority : priorities) {
		fronty_[priority]; //fronty_.emplace(priority, std::queue<std::pair<std::string, std::string>>());
		mutexPtrs_[priority]; //mutexPtrs_.emplace(priority, std::make_unique<std::mutex>());
	}
	vlakno_ = std::thread(&Log::vypisovaciLoop, this);
}

Log::~Log() {
	running_ = false;
	flush();
	vlakno_.join();
}

void Log::vypisovaciLoop() {
	auto predicate = [this]() -> bool {
		return !this->running_ || std::any_of(fronty_.begin(), fronty_.end(), [](decltype(*fronty_.begin()) pair) -> bool {return !pair.second.empty(); });
	};
	static std::mutex dummyMutex;
	std::unique_lock lock(dummyMutex);	 //prázdný lock jen pro cond_var; zamykání mutexù probíhá až v tìle cyklu
	while (running_) {
		condVar_.wait(lock, predicate);
		for (Priority priority : priorities) {
			std::unique_lock queue_lock(*mutexPtrs_[priority], std::try_to_lock);
			if (!queue_lock.owns_lock())			   //pokus o zamèení mutexu; pokud selže, pøeskoèí iteraci
				continue;							   //nepodaøí-li se získat mutex, neblokujeme a pokraèujeme na další prioritu
			auto& fronta = fronty_[priority];
			for (size_t size = fronta.size(); size; --size) {
				const auto&[time, msg] = fronta.front();
				switch (priority) {
					//switch how to write to std::cout
					case Priority::err:
					case Priority::warning:
					case Priority::log:
						std::cout << priority << time << '\t';
					case Priority::prompt:
						std::cout << msg << std::endl;
					case Priority::input:
						break;
					default:
						Log::error() << __FUNCTION__ << ": Not implemented token " << priority << '(' << unsigned(priority) << ')';
				}
				fileStream_ << priority << time << '\t' << msg << std::endl;
				fronta.pop();
			}
		}
	}
}

#ifndef TEMPLATED

void Log::insertString(const std::string& str, Log::Priority p) {
	insertString(std::string(str), p);
}

void Log::insertString(std::string&& str, Log::Priority p) {
	std::pair<std::string, std::string> pair = std::make_pair(getTimeString("%H:%M:%S.%ms"), std::move(str));
	{
		std::lock_guard<std::mutex> guard(*mutexPtrs_[p]);
		fronty_[p].push(std::move(pair));
	}
	flush();
}

#endif

bool Log::isInitialized() {
	return instance_.get();
}

void Log::flush() {
	instance_->condVar_.notify_one();
}

void Log::initialize(std::string_view header, std::string_view suffix) {
	if (isInitialized())
		Log::error() << __FUNCTION__ << ": Inicializace již jednou zavolána!";
	else
		instance_ = std::make_unique<Log>(header, suffix);
}

Log::Formatter<Log::Priority::err> Log::error() {
	return instance_->createFormatter<Priority::err>();
}

Log::Formatter<Log::Priority::warning> Log::warning() {
	return instance_->createFormatter<Priority::warning>();
}

Log::Formatter<Log::Priority::log> Log::log() {
	return instance_->createFormatter<Priority::log>();
}

Log::Formatter<Log::Priority::prompt> Log::prompt() {
	return instance_->createFormatter<Priority::prompt>();
}

Log::Formatter<Log::Priority::input> Log::input() {
	return instance_->createFormatter<Priority::input>();
}


#ifndef TEMPLATED

void Log::error(const std::string& str) {
	instance_->insertString(str, Priority::err);
}

void Log::error(std::string&& str) {
	instance_->insertString(std::move(str), Priority::err);
}

void Log::warning(const std::string& str) {
	instance_->insertString(str, Priority::warning);
}

void Log::warning(std::string&& str) {
	instance_->insertString(std::move(str), Priority::warning);
}

void Log::log(const std::string& str) {
	instance_->insertString(str, Priority::log);
}
void Log::log(std::string&& str) {
	instance_->insertString(std::move(str), Priority::log);
}

void Log::prompt(const std::string& str) {
	instance_->insertString(str, Priority::prompt);
}

void Log::prompt(std::string&& str) {
	instance_->insertString(std::move(str), Priority::prompt);
}
#endif


template<typename T>
Log::Formatter<Log::Priority::input>& Log::Formatter<Log::Priority::input>::operator>>(T& t) {
	std::lock_guard<std::mutex> guard(*mutexPtr_);
	std::cin >> t;
	stringStream_ << t;
	return *this;
}

std::string Log::Formatter<Log::Priority::input>::getline() {
	std::string res;
	std::getline(std::cin, res);
	return res;
}