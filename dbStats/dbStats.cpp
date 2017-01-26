#include <repo/repo_controller.h>
#include <repo/core/handler/repo_database_handler_mongo.h>
#include <ctime>
#include <string>

static int64_t getTimeStamp(
	const int &year,
	const int &month)
{
	struct tm date;
	date.tm_year = year - 1900;
	date.tm_mon = month - 1;
	date.tm_mday = 1;
	date.tm_hour = date.tm_min = date.tm_sec = 0;
	date.tm_isdst = 0;

	auto timestamp = mktime(&date);
	mongo::Date_t mDate = mongo::Date_t(timestamp * 1000);
	return mDate.asInt64();
}

static void getYearMonthFromTimeStamp(
	const int64_t &timestamp,
	int &year,
	int &month)
{
	
	auto ts = timestamp / 1000;
	auto dateTime = gmtime(&ts);
	year = dateTime->tm_year + 1900;
	month = dateTime->tm_mon + 1;

}

static int64_t getTimestampOfFirstRevision(
	repo::core::handler::MongoDatabaseHandler *handler,
	const std::string                          &dbName,
	const std::string                          &project,
	std::map < int, std::map<int, int> >       &revisionsPerMonth)
{
	repo::core::model::RepoBSON criteria = BSON(REPO_NODE_REVISION_LABEL_INCOMPLETE << BSON("$exists" << false));
	auto bsons = handler->findAllByCriteria(dbName, project + ".history", criteria, REPO_NODE_REVISION_LABEL_TIMESTAMP);
	int64_t firstRevTS = -1;

	for (const auto &bson : bsons)
	{
		repo::core::model::RevisionNode revNode(bson);
		auto revTS = revNode.getTimestampInt64();


		if (revTS != -1)
		{
			if (firstRevTS == -1)
				firstRevTS = revTS;

			int year, month;
			getYearMonthFromTimeStamp(revTS, year, month);
			if (revisionsPerMonth.find(year) == revisionsPerMonth.end())
				revisionsPerMonth[year] = std::map<int, int>();
			if (revisionsPerMonth[year].find(month) == revisionsPerMonth[year].end())
				revisionsPerMonth[year][month] = 0;

			++revisionsPerMonth[year][month];
		}
		
		


	}

	return firstRevTS;
}

static void getProjectsStatistics(
	repo::RepoController                      *controller,
	const repo::RepoController::RepoToken     *token,
	repo::core::handler::MongoDatabaseHandler *handler
	)
{
	auto databases = controller->getDatabases(token);
	auto projects = controller->getDatabasesWithProjects(token, databases);
	std::map < int, std::map<int, int> > newProjectsPerMonth;
	std::map < int, std::map<int, int> > newRevisionsPerMonth;
	int totalNProjects = 0;
	for (const auto &dbEntry : projects)
	{
		
		auto dbName = dbEntry.first;
		for (const auto project : dbEntry.second)
		{
			totalNProjects++;
			auto time  = getTimestampOfFirstRevision(handler, dbName, project, newRevisionsPerMonth);
			if (time != -1)
			{
				int year = 0, month = 0;
				getYearMonthFromTimeStamp(time, year, month);
				if (newProjectsPerMonth.find(year) == newProjectsPerMonth.end())
					newProjectsPerMonth[year] = std::map<int, int>();
				if (newProjectsPerMonth[year].find(month) == newProjectsPerMonth[year].end())
					newProjectsPerMonth[year][month] = 0;

				++newProjectsPerMonth[year][month];
			}						
		}		
	}


	repoInfo << "======== NEW PROJECTS PER MONTH =========";
	for (const auto yearEntry : newProjectsPerMonth)
	{
		auto year = yearEntry.first;
		for (const auto monthEntry : yearEntry.second)
		{
			repoInfo << "Year: " << year << "\tMonth: " << monthEntry.first << " \tProjects: " << monthEntry.second;
		}
	}
	repoInfo << "Total #Projects: " << totalNProjects;

	int nRevisions = 0;
	repoInfo << "======== NEW REVISIONS PER MONTH =========";
	for (const auto yearEntry : newRevisionsPerMonth)
	{
		auto year = yearEntry.first;
		for (const auto monthEntry : yearEntry.second)
		{
			repoInfo << "Year: " << year << "\tMonth: " << monthEntry.first << " \tProjects: " << monthEntry.second;
			nRevisions += monthEntry.second;
		}
	}
	repoInfo << "Total #Revisions: " << nRevisions;
}

static uint64_t getNewUsersWithinDuration(
	repo::core::handler::MongoDatabaseHandler *handler,
	const bool	                               paidUsers,
	const int64_t                             &from,
	const int64_t                             &to )
{
	
	repo::core::model::RepoBSONBuilder timeRangeBuilder;
	timeRangeBuilder.appendTime("$lt", to);
	timeRangeBuilder.appendTime("$gt", from);

	repo::core::model::RepoBSON planInfo;
	if(paidUsers)
		planInfo = BSON("plan" << "THE-100-QUID-PLAN" << "inCurrentAgreement" << true << "createdAt" << timeRangeBuilder.obj());
	else
		planInfo = BSON("plan" << "BASIC" << "createdAt" << timeRangeBuilder.obj());

	auto subscriptionCriteria = BSON("$elemMatch" << planInfo);
	repo::core::model::RepoBSON criteria = BSON("customData.billing.subscriptions" << subscriptionCriteria);
	return handler->findAllByCriteria(REPO_ADMIN, REPO_SYSTEM_USERS, criteria).size();
}

static void getNewUsersPerMonth(
	repo::core::handler::MongoDatabaseHandler *handler,
	const bool	                               paidUsers)
{
	int month = 8;
	int year = 2016;

	if (paidUsers)
		month = 10;

	time_t rawTime;
	time(&rawTime);
	auto currTime = gmtime(&rawTime);
	int maxYear = currTime->tm_year + 1900;
	int maxMonth = currTime->tm_mon + 1;

	int nUsers = 0;
	while (maxYear > year || (maxYear == year && maxMonth >= month))
	{
		int nextMonth = month == 12 ? 1 : month + 1;
		int nextYear = nextMonth == 1 ? year + 1 : year;

		auto from = getTimeStamp(year, month);
		auto to = getTimeStamp(nextYear, nextMonth);
		auto users = getNewUsersWithinDuration(handler, paidUsers, from, to);
		nUsers += users;
		repoInfo << "Year: " << year << "\tMonth: " << month << " \tUsers: " <<  users;
	
		year = nextYear;
		month = nextMonth;
	}
	repoInfo << "Total Users: " << nUsers;
}

int main(int argc, char* argv[])
{
	repo::RepoController *controller = new repo::RepoController();
	controller->setLoggingLevel(repo::lib::RepoLog::RepoLogLevel::INFO);
	std::string errMsg;

	if (argc < 5)	
	{
		std::cout << "Usage: " << argv[0] << " <dbAddress> <port> <username> <password" << std::endl;
		exit(0);
	}
	std::string dbAdd = argv[1];
	int port = std::stoi(argv[2]);
	std::string username = argv[3];
	std::string password = argv[4];
	repo::RepoController::RepoToken* token = controller->authenticateToAdminDatabaseMongo(errMsg, dbAdd, port, username, password);

	auto handler = repo::core::handler::MongoDatabaseHandler::getHandler("");

	repoInfo << "======== NEW USERS PER MONTH ==========";
	getNewUsersPerMonth(handler, false);
	repoInfo << "======== NEW PAID USERS PER MONTH ==========";
	getNewUsersPerMonth(handler, true);

	getProjectsStatistics(controller, token, handler);
}
