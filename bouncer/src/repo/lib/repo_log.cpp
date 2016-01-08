#include "repo_log.h"

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp>
#if BOOST_VERSION > 105700
#include <boost/core/null_deleter.hpp>
#else
#include <boost/utility/empty_deleter.hpp>
#endif
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>



using namespace repo::lib;

using text_sink = boost::log::sinks::synchronous_sink< boost::log::sinks::text_ostream_backend >;


RepoLog::RepoLog()
{
}
RepoLog::~RepoLog()
{
}

void RepoLog::log(
	const RepoLogLevel &severity, 
	const std::string  &msg)
{

	switch (severity)
	{
	case RepoLogLevel::TRACE :
		repoTrace << msg;
		break;
	case RepoLogLevel::DEBUG :
		repoDebug << msg;
		break;
	case RepoLogLevel::INFO :
		repoInfo << msg;
		break;
	case RepoLogLevel::WARNING :
		repoWarning << msg;
		break;
	case RepoLogLevel::ERR :
		repoError << msg;
		break;
	case RepoLogLevel::FATAL :
		repoFatal << msg;
	}

	
}

void RepoLog::logToFile(const std::string &filePath)
{

	boost::log::add_file_log
		(
		boost::log::keywords::file_name = filePath + "_%N",
		boost::log::keywords::rotation_size = 10 * 1024 * 1024,
		boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
		boost::log::keywords::format = "[%TimeStamp%]: %Message%"
		);

}

void RepoLog::setLoggingLevel(const RepoLogLevel &level)
{
	boost::log::trivial::severity_level loggingLevel;
	switch (level)
	{
	case repo::lib::RepoLog::RepoLogLevel::TRACE:
		loggingLevel = boost::log::trivial::trace;
		break;
	case repo::lib::RepoLog::RepoLogLevel::DEBUG:
		loggingLevel = boost::log::trivial::debug;
		break;
	case repo::lib::RepoLog::RepoLogLevel::INFO:
		loggingLevel = boost::log::trivial::info;
		break;
	case repo::lib::RepoLog::RepoLogLevel::WARNING:
		loggingLevel = boost::log::trivial::warning;
		break;
	case repo::lib::RepoLog::RepoLogLevel::ERR:
		loggingLevel = boost::log::trivial::error;
		break;
	case repo::lib::RepoLog::RepoLogLevel::FATAL:
		loggingLevel = boost::log::trivial::fatal;
		break;
	default:
		repoError << "Unknown log level: " << (int)level;
		return;

	}

	boost::log::core::get()->set_filter(
		boost::log::trivial::severity >= loggingLevel);
}

void RepoLog::subscribeBroadcaster(RepoBroadcaster *broadcaster){

	boost::iostreams::stream<RepoBroadcaster> *streamptr =
		new boost::iostreams::stream<RepoBroadcaster>(*broadcaster);

#if BOOST_VERSION > 105700
	boost::shared_ptr< std::ostream > stream(
		streamptr, boost::null_deleter());
#else
	boost::shared_ptr< std::ostream > stream(
		streamptr, boost::empty_deleter());
#endif


	boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();


	sink->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);
	//FIXME: better format!
	sink->set_formatter
		(
		boost::log::expressions::stream
		<< "%" << boost::log::trivial::severity << "%"
		<< boost::log::expressions::smessage);


	sink->locked_backend()->add_stream(stream);
	sink->locked_backend()->auto_flush(true);

	// Register the sink in the logging core
	boost::log::core::get()->add_sink(sink);

	repoTrace << "Subscribed broadcaster to log";
}


void RepoLog::subscribeListeners(
	const std::vector<RepoAbstractListener*> &listeners)
{
	repoTrace << "Adding new subscriber(s) to the log";

	/*
		FIXME: ideally, we should have a single broadcaster for the application
		       and you should only need to add the listeners when you subscribe
			   But the whole hackery of making a broadcaster to be a ostream
			   to tap into the boost logger made this really difficult
			   so for now, instantiate a new broadcaster everytime.
			   And I will review this on a braver day...
	*/
	RepoBroadcaster *broadcaster = new RepoBroadcaster();

	for (auto listener : listeners)
		broadcaster->subscribe(listener);

	subscribeBroadcaster(broadcaster);
}
