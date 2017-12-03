/*
 * Information_Management.h
 *
 *  Created on: Nov 8, 2017
 *      Author: Ben Simon
 *      This File Defines functions for managing the information used in the rest of the program.
 *      Primary functions consist of linking with an SQLite DB, and passing information to/from program to/from DB
 *      DB Maintenance and Setup Functions are Also Defined Here.
 *
 *      NOTE: This file defines primative DB routines for usage in other modules.
 *      The functions defined here, with one or two exceptions, should never be directly invoked by the main program.
 *
 *      All search functions return
 */

#ifndef INFO_MANAGEMENT_INFORMATION_MANAGEMENT_H_
#define INFO_MANAGEMENT_INFORMATION_MANAGEMENT_H_

/* BEGIN LINKAGE OF DEPENDENT LIBRARIES */
//#include "Libraries/SQLite/sqlite3.h"
//#include "Libraries/SQLite/sqlite3.c"
#include "Libraries/SQLiteCpp/Backup.cpp"
#include "Libraries/SQLiteCpp/Column.cpp"
#include "Libraries/SQLiteCpp/Database.cpp"
#include "Libraries/SQLiteCpp/Exception.cpp"
#include "Libraries/SQLiteCpp/Statement.cpp"
#include "Libraries/SQLiteCpp/Transaction.cpp"
#include <iostream>
#include <fstream>
#include <string>
#include "../Appointments/Appointment.h"
/* END LINKAGE OF DEPENDENT LIBRARIES */

bool verifyDBExists(std::string DBFilePath)
{
	std::cout << "Checking that the Database at" << DBFilePath << " exists...";
	std::ifstream ifile(DBFilePath);
	return (bool)ifile;
}

void createNewDB(std::string DBFilePath)
{
	SQLite::Database db(DBFilePath,SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
	std::string workerSQL = 	"CREATE TABLE WORKERS("  \
								"ID INT PRIMARY KEY UNIQUE NOT NULL," \
								"NAME TEXT NOT NULL," \
								"DAYS_AVAILABLE CHAR(13) NOT NULL," \
								"START_TIME INT NOT NULL," \
								"STOP_TIME INT NOT NULL);";
	db.exec(workerSQL);
	std::string visitorSQL = 	"CREATE TABLE VISITORS(" \
								"ID INT PRIMARY KEY UNIQUE NOT NULL," \
								"NAME TEXT NOT NULL);";
	db.exec(visitorSQL);
	std::string appointmentSQL = 	"CREATE TABLE APPOINTMENTS("  \
									"ID INT PRIMARY KEY UNIQUE NOT NULL," \
									"APPOINTMENT_DATE TEXT NOT NULL," \
									"APPOINTMENT_TIME INT NOT NULL," \
									"WORKER_ID INTEGER NOT NULL," \
									"VISITOR_ID INTEGER," \
									"IS_DAY_OFF BOOLEAN NOT NULL," \
									"VISITOR_LINK FOREIGN KEY (VISITOR_ID) REFERENCES VISITOR(ID)," \
									"WORKER_LINK FOREIGN KEY (WORKER_ID) REFERENCES WORKER(ID));";
	db.exec(appointmentSQL);
}


void addNewWorker(std::string DBFilePath,int id, std::string name, std::string days_available, int startHour, int startMinute, int stopHour, int stopMinute)
{
	SQLite::Database db(DBFilePath,SQLite::OPEN_READWRITE);
	std::string workerCreation = "INSERT INTO WORKERS(ID,NAME,DAYS_AVAILABLE,START_TIME,STOP_TIME) " \
							"VALUES("+std::to_string(id)+","+"\""+name+"\""+","+"'"+days_available+"'"+",\""+std::to_string(startHour)+":"+std::to_string(startMinute)+"\""+",\""+std::to_string(stopHour)+":"+std::to_string(stopMinute)+"\");";
	db.exec(workerCreation);
}

void addNewVisitor(std::string DBFilePath, int id, std::string name)
{
	SQLite::Database db(DBFilePath,SQLite::OPEN_READWRITE);
	std::string visitorCreation = 	"INSERT INTO WORKERS(ID,NAME,DAYS_AVAILABLE,START_TIME,STOP_TIME) " \
									"VALUES("+std::to_string(id)+","+"\""+name+"\""");";
	db.exec(visitorCreation);
}

//TODO: Fix Appointment Creator to account for visitor link
void addNewAppointment(std::string DBFilePath,int id, std::string appointmentDate,int appointmentTime, int workerID, int visitorID, bool isDayOff)
{
	SQLite::Database db(DBFilePath,SQLite::OPEN_READWRITE);
	std::string appointmentCreation = 	"INSERT INTO APPOINTMENTS(ID,APPOINTMENT_DATE,APPOINTMENT_TIME,WORKER_ID,VISITOR_ID,IS_DAY_OFF) " \
										"VALUES("+std::to_string(id)+","+"\""+appointmentDate+"\","+std::to_string(appointmentTime)+","+std::to_string(workerID)+","+std::to_string(visitorID)+","+std::to_string(isDayOff)+");";
	db.exec(appointmentCreation);
}

void removeWorker(std::string DBFilePath, int workerID)
{
	SQLite::Database db(DBFilePath,SQLite::OPEN_READWRITE);
	std::string workerRemoval = "DELETE FROM WORKERS WHERE ID="+std::to_string(workerID)+";";
	db.exec(workerRemoval);
}

void removeVisitor(std::string DBFilePath, int visitorID)
{
	SQLite::Database db(DBFilePath,SQLite::OPEN_READWRITE);
	std::string visitorRemoval = "DELETE FROM VISITORS WHERE ID="+std::to_string(visitorID)+";";
	db.exec(visitorRemoval);
}

void removeAppointment(std::string DBFilePath, int appointmentID)
{
	SQLite::Database db(DBFilePath,SQLite::OPEN_READWRITE);
	std::string appointmentRemoval = "DELETE FROM APPOINTMENTS WHERE ID="+std::to_string(appointmentID)+";";
	db.exec(appointmentRemoval);
}

int countMatchingWorkers(std::string DBFilePath, std::string workerName)
{
	int matchingRecords = 0;
	SQLite::Database db(DBFilePath,SQLite::OPEN_READONLY);
	SQLite::Statement workerSearch(db,"SELECT * FROM WORKERS WHERE NAME LIKE ?");
	std::string searchParam = "'%"+workerName+"%'";
	workerSearch.bind(1,searchParam);
	while(workerSearch.executeStep())
	{
		matchingRecords++;
	}
	return matchingRecords;
}

Worker* getWorker(std::string DBFilePath, int workerID)
{
	SQLite::Database db(DBFilePath,SQLite::OPEN_READONLY);
	SQLite::Statement workerSelect(db,"SELECT * FROM WORKERS WHERE ID=?");
	workerSelect.bind(1,workerID);
	int id;
	const char *name;
	const char *daysAvailable;
	int startTime;
	int stopTime;
	while(workerSelect.executeStep())
	{
		id = std::stoi(workerSelect.getColumn("ID"));
		name = workerSelect.getColumn("NAME");
		daysAvailable = workerSelect.getColumn("DAYS_AVAILABLE");
		startTime = std::stoi(workerSelect.getColumn("START_TIME"));
		stopTime = std::stoi(workerSelect.getColumn("STOP_TIME"));
	}
	std::string str(name);
	Worker * returner = new Worker(name,id); //TODO: Fix this once Worker class gets corrected.
	return returner;
}

void findMatchingWorkers(std::string DBFilePath, std::string workerName, Worker *resultsArray[])
{
	int matches = countMatchingWorkers(DBFilePath,workerName);
	int currentIndex = 0;
	SQLite::Database db(DBFilePath,SQLite::OPEN_READONLY);
	SQLite::Statement workerSearch(db,"SELECT * FROM WORKERS WHERE NAME LIKE ?");
	std::string searchParam = "'%"+workerName+"%'";
	workerSearch.bind(1,searchParam);
	while(workerSearch.executeStep() && currentIndex < matches)
	{
		int id = workerSearch.getColumn("ID");
		const char *name = workerSearch.getColumn("NAME");
		std::string str(name);
		resultsArray[currentIndex] = new Worker(name,id);
		currentIndex++;
	}
}

int countMatchingVisitors(std::string DBFilePath, std::string visitorName)
{
	int matchingRecords = 0;
	SQLite::Database db(DBFilePath,SQLite::OPEN_READONLY);
	SQLite::Statement visitorSearch(db,"SELECT * FROM VISITORS WHERE NAME LIKE ?");
	std::string searchParam = "'%"+visitorName+"%'";
	visitorSearch.bind(1,searchParam);
	while(visitorSearch.executeStep())
	{
		matchingRecords++;
	}
	return matchingRecords;
}

std::string getVisitorName(std::string DBFilePath,int visitorID)
{
	SQLite::Database db(DBFilePath,SQLite::OPEN_READONLY);
	SQLite::Statement visitorSelect(db,"SELECT * FROM VISITORS WHERE ID=?");
	const char* name;
	visitorSelect.bind(1,visitorID);
	while(visitorSelect.executeStep())
	{
		name = visitorSelect.getColumn("NAME");
	}
	std::string str(name);
	return name;
}

void findMatchingVisitors(std::string DBFilePath, std::string visitorName, Visitor *resultsArray[])
{
	int matches = countMatchingVisitors(DBFilePath,visitorName);
	int currentIndex = 0;
	SQLite::Database db(DBFilePath,SQLite::OPEN_READONLY);
	SQLite::Statement visitorSearch(db,"SELECT * FROM VISITORS WHERE NAME LIKE ?");
	std::string searchParam = "'%"+visitorName+"%'";
	visitorSearch.bind(1,searchParam);
	while(visitorSearch.executeStep() && currentIndex < matches)
	{
		int id = visitorSearch.getColumn("ID");
		const char *name = visitorSearch.getColumn("NAME");
		std::string str(name);
		resultsArray[currentIndex] = new Visitor(id,name);
		currentIndex++;
	}
}

int countMatchingAppointments(std::string DBFilePath,int workerID, std::string appointmentDate)
{
	int matchingRecords = 0;
	SQLite::Database db(DBFilePath,SQLite::OPEN_READONLY);
	SQLite::Statement appointmentSearch(db,"SELECT * FROM APPOINTMENTS WHERE WORKER_ID=? AND APPOINTMENT_DATE=?");
	appointmentSearch.bind(1,workerID);
	std::string date = "\""+appointmentDate+"\"";
	appointmentSearch.bind(2,date);
	while(appointmentSearch.executeStep())
	{
		matchingRecords++;
	}
	return matchingRecords;
}

int countMatchingAppointments(std::string DBFilePath, int entityID, bool searchByWorker)
{
	int matchingRecords = 0;
	SQLite::Database db(DBFilePath,SQLite::OPEN_READONLY);
	if(searchByWorker == true)
	{
		SQLite::Statement appointmentSearch(db,"SELECT * FROM APPOINTMENTS WHERE WORKER_ID=?");
		appointmentSearch.bind(1,entityID);
		while(appointmentSearch.executeStep())
		{
			matchingRecords++;
		}
	}
	else
	{
		SQLite::Statement appointmentSearch(db,"SELECT * FROM APPOINTMENTS WHERE VISITOR_ID=?");
		appointmentSearch.bind(1,entityID);
		while(appointmentSearch.executeStep())
		{
			matchingRecords++;
		}
	}
	return matchingRecords;
}

int countMatchingAppointments(std::string DBFilePath, int workerID, std::string appointmentDate, int appointmentTime)
{
	int matchingRecords = 0;
	SQLite::Database db(DBFilePath,SQLite::OPEN_READONLY);
	SQLite::Statement appointmentSearch(db,"SELECT * FROM APPOINTMENTS WHERE WORKER_ID=? AND APPOINTMENT_DATE=? AND APPOINTMENT_TIME=?");
	appointmentSearch.bind(1,workerID);
	std::string date = "\""+appointmentDate+"\"";
	appointmentSearch.bind(2,date);
	appointmentSearch.bind(3,appointmentTime);
	while(appointmentSearch.executeStep())
	{
		matchingRecords++;
	}
	return matchingRecords;
}

int countMatchingAppointments(std::string DBFilePath, std::string appointmentDate)
{
	int matchingRecords = 0;
	SQLite::Database db(DBFilePath,SQLite::OPEN_READONLY);
	SQLite::Statement appointmentSearch(db,"SELECT * FROM APPOINTMENTS WHERE APPOINTMENT_DATE=?");
	std::string date = "\""+appointmentDate+"\"";
	appointmentSearch.bind(1,date);
	while(appointmentSearch.executeStep())
	{
		matchingRecords++;
	}
	return matchingRecords;
}

int countMatchingAppointments(std::string DBFilePath, std::string appointmentDate, int appointmentTime)
{
	int matchingRecords = 0;
	SQLite::Database db(DBFilePath,SQLite::OPEN_READONLY);
	SQLite::Statement appointmentSearch(db,"SELECT * FROM APPOINTMENTS WHERE APPOINTMENT_DATE=? AND APPOINTMENT_TIME=?");
	std::string date = "\""+appointmentDate+"\"";
	appointmentSearch.bind(1,date);
	int time = appointmentTime;
	appointmentSearch.bind(2,time);
	while(appointmentSearch.executeStep())
	{
		matchingRecords++;
	}
	return matchingRecords;
}

void findAppointments(std::string DBFilePath, int workerID, std::string appointmentDate, Appointment *resultsArray[])
{
	int matches = countMatchingAppointments(DBFilePath,workerID,appointmentDate);
	int currentIndex = 0;
	SQLite::Database db(DBFilePath,SQLite::OPEN_READONLY);
	SQLite::Statement appointmentSearch(db,"SELECT * FROM APPOINTMENTS WHERE WORKER_ID=? AND APPOINTMENT_DATE=?");
	appointmentSearch.bind(1,workerID);
	std::string date = "\""+appointmentDate+"\"";
	appointmentSearch.bind(2,date);
	while(appointmentSearch.executeStep() && currentIndex<matches)
	{
		bool isDayOff;
		const char *tempBoolStorer;
		std::string date = appointmentSearch.getColumn("APPOINTMENT_DATE");
		int month = std::stoi(date.substr(0,2));
		int day = std::stoi(date.substr(2,2));
		int year = std::stoi(date.substr(5,4));
		int appointmentTime = appointmentSearch.getColumn("APPOINTMENT_TIME");
		int visitorID = appointmentSearch.getColumn("VISITOR_ID");
		std::string visitorName = getVisitorName(DBFilePath,visitorID);
		int workerID = appointmentSearch.getColumn("WORKER_ID");
		Worker * linkedWorker = getWorker(DBFilePath,workerID);
		tempBoolStorer = appointmentSearch.getColumn("IS_DAY_OFF");
		std::string str(tempBoolStorer);
		if(str.compare("TRUE") || str.compare("1"))
		{
			isDayOff = true;
		}
		else
		{
			isDayOff = false;
		}
		resultsArray[currentIndex] = new Appointment(isDayOff,day,month,year,appointmentTime,linkedWorker,visitorName,visitorID);
		currentIndex++;
	}
}

void findAppointments(std::string DBFilePath, int workerID, std::string appointmentDate, int appointmentTime, Appointment *resultsArray[])
{
	int matches = countMatchingAppointments(DBFilePath,workerID,appointmentDate,appointmentTime);
	int currentIndex = 0;
	SQLite::Database db(DBFilePath,SQLite::OPEN_READONLY);
	SQLite::Statement appointmentSearch(db,"SELECT * FROM APPOINTMENTS WHERE WORKER_ID=? AND APPOINTMENT_DATE=? AND APPOINTMENT_TIME=?");
	appointmentSearch.bind(1,workerID);
	std::string date = "\""+appointmentDate+"\"";
	appointmentSearch.bind(2,date);
	int time = appointmentTime;
	appointmentSearch.bind(3,time);
	while(appointmentSearch.executeStep() && currentIndex < matches)
	{
		bool isDayOff;
		const char *tempBoolStorer;
		std::string date = appointmentSearch.getColumn("APPOINTMENT_DATE");
		int month = std::stoi(date.substr(0,2));
		int day = std::stoi(date.substr(2,2));
		int year = std::stoi(date.substr(5,4));
		int appointmentTime = appointmentSearch.getColumn("APPOINTMENT_TIME");
		int visitorID = appointmentSearch.getColumn("VISITOR_ID");
		std::string visitorName = getVisitorName(DBFilePath,visitorID);
		int workerID = appointmentSearch.getColumn("WORKER_ID");
		tempBoolStorer = appointmentSearch.getColumn("IS_DAY_OFF");
		std::string str(tempBoolStorer);
		if(str.compare("TRUE") || str.compare("1"))
		{
			isDayOff = true;
		}
		else
		{
			isDayOff = false;
		}
		Worker * linkedWorker = getWorker(DBFilePath,workerID);
		resultsArray[currentIndex] = new Appointment(isDayOff,day,month,year,appointmentTime,linkedWorker,visitorName,visitorID);
		currentIndex++;
	}
}

void findAppointments(std::string DBFilePath, int entityID, bool searchByWorker, Appointment *resultsArray[])
{
	int matches = countMatchingAppointments(DBFilePath,entityID,searchByWorker);
	int currentIndex = 0;
	SQLite::Database db(DBFilePath,SQLite::OPEN_READONLY);
	if(searchByWorker == true)
	{
		SQLite::Statement appointmentSearch(db,"SELECT * FROM APPOINTMENTS WHERE WORKER_ID=?");
		appointmentSearch.bind(1,entityID);
		while(appointmentSearch.executeStep() && currentIndex<matches)
		{
			const char *tempBoolStorer;
			bool isDayOff;
			std::string date = appointmentSearch.getColumn("APPOINTMENT_DATE");
			int month = std::stoi(date.substr(0,2));
			int day = std::stoi(date.substr(2,2));
			int year = std::stoi(date.substr(5,4));
			int appointmentTime = appointmentSearch.getColumn("APPOINTMENT_TIME");
			int visitorID = appointmentSearch.getColumn("VISITOR_ID");
			std::string visitorName = getVisitorName(DBFilePath,visitorID);
			int workerID = appointmentSearch.getColumn("WORKER_ID");
			tempBoolStorer = appointmentSearch.getColumn("IS_DAY_OFF");
			std::string str(tempBoolStorer);
			if(str.compare("TRUE") || str.compare("1"))
			{
				isDayOff = true;
			}
			else
			{
				isDayOff = false;
			}
			Worker * linkedWorker = getWorker(DBFilePath,workerID);
			resultsArray[currentIndex] = new Appointment(isDayOff,day,month,year,appointmentTime,linkedWorker,visitorName,visitorID);
			currentIndex++;
		}
	}
	else
	{
		SQLite::Statement appointmentSearch(db,"SELECT * FROM APPOINTMENTS WHERE VISITOR_ID=?");
		appointmentSearch.bind(1,entityID);
		while(appointmentSearch.executeStep() && currentIndex < matches)
		{
			const char *tempBoolStorer;
			bool isDayOff;
			std::string date = appointmentSearch.getColumn("APPOINTMENT_DATE");
			int month = std::stoi(date.substr(0,2));
			int day = std::stoi(date.substr(2,2));
			int year = std::stoi(date.substr(5,4));
			int appointmentTime = appointmentSearch.getColumn("APPOINTMENT_TIME");
			int visitorID = appointmentSearch.getColumn("VISITOR_ID");
			std::string visitorName = getVisitorName(DBFilePath,visitorID);
			int workerID = appointmentSearch.getColumn("WORKER_ID");
			tempBoolStorer = appointmentSearch.getColumn("IS_DAY_OFF");
			std::string str(tempBoolStorer);
			if(str.compare("TRUE") || str.compare("1"))
			{
				isDayOff = true;
			}
			else
			{
				isDayOff = false;
			}
			Worker * linkedWorker = getWorker(DBFilePath,workerID);
			resultsArray[currentIndex] = new Appointment(isDayOff,day,month,year,appointmentTime,linkedWorker,visitorName,visitorID);
			currentIndex++;
		}
	}
}
void findAppointments(std::string DBFilePath, std::string appointmentDate, int appointmentTime, Appointment *resultsArray[])
{
	int matches = countMatchingAppointments(DBFilePath,appointmentDate, appointmentTime);
	int currentIndex = 0;
	SQLite::Database db(DBFilePath,SQLite::OPEN_READONLY);
	SQLite::Statement appointmentSearch(db,"SELECT * FROM APPOINTMENTS WHERE APPOINTMENT_DATE=? AND APPOINTMENT_TIME=?");
	std::string date = "\""+appointmentDate+"\"";
	appointmentSearch.bind(1,date);
	int time = appointmentTime;
	appointmentSearch.bind(2,time);
	while(appointmentSearch.executeStep() && currentIndex<matches)
	{
		const char *tempBoolStorer;
		bool isDayOff;
		std::string date = appointmentSearch.getColumn("APPOINTMENT_DATE");
		int month = std::stoi(date.substr(0,2));
		int day = std::stoi(date.substr(2,2));
		int year = std::stoi(date.substr(5,4));
		int appointmentTime = appointmentSearch.getColumn("APPOINTMENT_TIME");
		int visitorID = appointmentSearch.getColumn("VISITOR_ID");
		std::string visitorName = getVisitorName(DBFilePath,visitorID);
		int workerID = appointmentSearch.getColumn("WORKER_ID");
		tempBoolStorer = appointmentSearch.getColumn("IS_DAY_OFF");
		std::string str(tempBoolStorer);
		if(str.compare("TRUE") || str.compare("1"))
		{
			isDayOff = true;
		}
		else
		{
			isDayOff = false;
		}
		Worker * linkedWorker = getWorker(DBFilePath,workerID);
		resultsArray[currentIndex] = new Appointment(isDayOff,day,month,year,appointmentTime,linkedWorker,visitorName,visitorID);
		currentIndex++;
	}
}
#endif /* INFO_MANAGEMENT_INFORMATION_MANAGEMENT_H_ */
