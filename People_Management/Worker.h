/*
 * Worker.h
 *
 *  Created on: Nov 3, 2017
 *      Author: David
 */
#include <iostream>
#include <string>
using namespace std;
#ifndef WORKER_H
#define WORKER_H

class Worker {
private:
	string workerName;
	int workerID;
	string workingDays;
	int startTime;
	int stopTime;
public:
	string getWorkerName();
	string getWorkingDays();
	int getWorkerID();
	Worker(string name, int id, int startTime, int stopTime, string workingDays);
};
#endif /* WORKER_H */
