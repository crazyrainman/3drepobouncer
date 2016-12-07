/**
*  Copyright (C) 2016 3D Repo Ltd
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU Affero General Public License as
*  published by the Free Software Foundation, either version 3 of the
*  License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Affero General Public License for more details.
*
*  You should have received a copy of the GNU Affero General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <repo/core/model/repo_node_utils.h>
#include <repo/repo_controller.h>
#include "repo_test_database_info.h"
#include <fstream>
#include <boost/iostreams/device/mapped_file.hpp>

static bool projectExists(
	const std::string &db,
	const std::string &project)
{
	bool res = false;
	repo::RepoController *controller = new repo::RepoController();
	std::string errMsg;
	repo::RepoController::RepoToken *token =
		controller->authenticateToAdminDatabaseMongo(errMsg, REPO_GTEST_DBADDRESS, REPO_GTEST_DBPORT,
		REPO_GTEST_DBUSER, REPO_GTEST_DBPW);
	if (token)
	{
		std::list<std::string> dbList;
		dbList.push_back(db);
		auto dbMap = controller->getDatabasesWithProjects(token, dbList);
		auto dbMapIt = dbMap.find(db);
		if (dbMapIt != dbMap.end())
		{
			std::list<std::string> projects = dbMapIt->second;
			res = std::find(projects.begin(), projects.end(), project) != projects.end();
		}
	}
	controller->disconnectFromDatabase(token);
	delete controller;
	return res;
}

static bool projectSettingsCheck(
	const std::string  &dbName, const std::string  &projectName, const std::string  &owner, const std::string  &tag, const std::string  &desc)
{
	bool res = false;
	repo::RepoController *controller = new repo::RepoController();
	std::string errMsg;
	repo::RepoController::RepoToken *token =
		controller->authenticateToAdminDatabaseMongo(errMsg, REPO_GTEST_DBADDRESS, REPO_GTEST_DBPORT,
		REPO_GTEST_DBUSER, REPO_GTEST_DBPW);
	if (token)
	{
		auto scene = controller->fetchScene(token, dbName, projectName, REPO_HISTORY_MASTER_BRANCH, true, true);
		if (scene)
		{
			res = scene->getOwner() == owner && scene->getTag() == tag && scene->getMessage() == desc;
			delete scene;
		}
	}
	controller->disconnectFromDatabase(token);
	delete controller;
	return res;
}

static bool fileExists(
	const std::string &file)
{
	std::ifstream ofs(file);
	const bool valid = ofs.good();
	ofs.close();
	return valid;
}

static bool filesCompare(
	const std::string &fileA,
	const std::string &fileB )
{
	bool match = false;
	std::ifstream fA(fileA), fB(fileB);
	if (fA.good() && fB.good())
	{
		std::string lineA, lineB;
		bool endofA, endofB;
		int i = 0; 
		while ((endofA = (bool)std::getline(fA, lineA)) && (endofB = (bool)std::getline(fB, lineB)))
		{
			if (lineA.size() != lineB.size())
			{

			}
			match = lineA == lineB;
			++i;
			if (!match)
			{
				std::cout << "Failed match. " << std::endl;
				std::cout << "line A: " << lineA << std::endl;
				std::cout << "line B: " << lineB << std::endl;
				std::cout << "lines match ? " << (lineA == lineB) << std::endl;
				std::cout << "size: " << lineA.size() << " , " << lineB.size() << std::endl;
				for (int i = 0; i < lineA.size(); ++i)
				{
					std::cout << lineA[i] << " , " << lineB[i] << std::endl;
				}
				break;
			}

		}
		
		if (!endofA)
		{
			//if endofA is false then end of B won't be found as getline wouldn't have ran for fB
			endofB = (bool)std::getline(fB, lineB);
		}
			
		match &= (!endofA && !endofB);
		std::cout << "End of A? " << endofA << " end of B? " << endofB << " #lines scanned: " << i << std::endl;
	}

	return match;

	//FILE *afile = fopen(fileA.c_str(), "r");
	//FILE *bfile = fopen(fileB.c_str(), "r");
	//if (afile && bfile)
	//{
	//	std::vector<char> bufferA, bufferB;
	//	bufferA.resize(1024000);
	//	bufferB.resize(1024000);
	//	size_t sizeA, sizeB;
	//	int count = 0;
	//	do{
	//		sizeA = fread(bufferA.data(), bufferA.size(), 1, afile);
	//		sizeB = fread(bufferB.data(), bufferB.size(), 1, bfile);
	//		if (sizeA == sizeB)
	//		{
	//			for (int i = 0; i < bufferA.size(); ++i)
	//			{
	//				match = bufferA[i] == bufferB[i];
	//				if (!match)
	//				{
	//					std::cout << "Count: " << i + count*bufferA.size() << " mistatched! a: " << bufferA[i] << " b: " << bufferB[i] << std::endl;
	//					break;
	//				}
	//			}
	//		}
	//		else
	//		{
	//			std::cout << " count mismatched: sizeA : " << sizeA << ", " << sizeB << std::endl;
	//			match = false;
	//			break;
	//		}
	//		count++;
	//	} while (sizeA > 0);

	//	fclose(afile);
	//	fclose(bfile);
	//}

	//boost::iostreams::mapped_file_source f1(fileA);
	//boost::iostreams::mapped_file_source f2(fileB);

	//return f1.size() == f2.size()
	//	&& std::equal(f1.data(), f1.data() + f1.size(), f2.data());
	
}

static bool compareVectors(const repo_vector2d_t &v1, const repo_vector2d_t &v2)
{
	return v1.x == v2.x && v1.y == v2.y;
}

static bool compareVectors(const repo_vector_t &v1, const repo_vector_t &v2)
{
	return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

static bool compareVectors(const repo_color4d_t &v1, const repo_color4d_t &v2)
{
	return v1.r == v2.r && v1.g == v2.g && v1.b == v2.b && v1.a == v2.a;
}

template <typename T>
static bool compareVectors(const std::vector<T> &v1, const  std::vector<T> &v2)
{
	if (v1.size() != v2.size())
	{
		return false;
	}

	for (size_t i = 0; i < v1.size(); ++i)
	{
		if (!compareVectors(v1[i], v2[i]))
		{
			return false;
		}
	}

	return true;
}

template <typename T>
static bool compareStdVectors(const std::vector<T> &v1, const std::vector<T> &v2)
{
	bool identical;
	if (identical = v1.size() == v2.size())
	{
		for (int i = 0; i < v1.size(); ++i)
		{
			identical &= v1[i] == v2[i];
		}
	}
	return identical;
}

static bool compareMaterialStructs(const repo_material_t &m1, const repo_material_t &m2)
{
	return compareStdVectors(m1.ambient, m2.ambient)
		&& compareStdVectors(m1.diffuse, m2.diffuse)
		&& compareStdVectors(m1.specular, m2.specular)
		&& compareStdVectors(m1.emissive, m2.emissive)
		&& m1.opacity == m2.opacity
		&& m1.shininess == m2.shininess
		&& m1.shininessStrength == m2.shininessStrength
		&& m1.isWireframe == m2.isWireframe
		&& m1.isTwoSided == m2.isTwoSided;
}

static std::string getRandomString(const uint32_t &iLen)
{
	std::string sStr;
	sStr.reserve(iLen);
	char syms[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	unsigned int Ind = 0;
	srand(time(NULL) + rand());
	for (int i = 0; i < iLen; ++i);
	{
		sStr.push_back(syms[rand() % 62]);
	}

	return sStr;
}