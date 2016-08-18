#include <iostream>

#include "../../AsyncTasks.h"

#include <windows.h>

/*
	main - Test the current implementation of the AsynchTasks namespace
	Author: Mitchell Croft
	Created: 18/08/2016
	Modified: 18/08/2016
*/
int main() {
	//Create the Task Manager
	if (AsynchTasks::TaskManager::create(1)) {
		//Save the current state of the escape and space keys
		bool escapeState = GetKeyState(VK_ESCAPE) < 0;
		bool spaceKey[] = { GetKeyState(VK_SPACE) < 0, GetKeyState(VK_SPACE) < 0 };

		//Create the base job
		AsynchTasks::Task<unsigned int> stringTask = AsynchTasks::TaskManager::createTask<unsigned int>();

		//Set the callback to occur on main
		stringTask->callbackOnMain = true;

		//Set the process function
		stringTask->process = [&]() -> unsigned int {
			//Create a random character string
			unsigned int sum = 0;

			//Loop through and add the totals from 0 to UINT_MAX
			for (unsigned int i = 0; i < UINT_MAX; i++)
				sum++;

			//Return the sum
			return sum;
		};

		//Attach the callback function
		stringTask->callback = [&](unsigned int& pNum) {
			printf("\n\nCounted to: %i\n\n", pNum);
		};

		//Display initial instructions
		printf("Press 'SPACE' to count to UINT_MAX (Reusable Task Test)\n\n");

		//Loop till escape is pressed
		while (GetKeyState(VK_ESCAPE) < 0 == escapeState) {																			 
			//Update the input
			spaceKey[0] = spaceKey[1];
			spaceKey[1] = GetKeyState(VK_SPACE) < 0;

			//Update the Task Manager
			AsynchTasks::TaskManager::update();

			//Output a single '.'
			if (stringTask->status != AsynchTasks::ETaskStatus::Completed && stringTask->status != AsynchTasks::ETaskStatus::Setup)
				printf(".");

			//Determine if the user has pressed space
			if (spaceKey[1] && !spaceKey[0]) {
				//Add the task to the Task Manager
				if (AsynchTasks::TaskManager::addTask(stringTask))
					printf("Starting to count now:\n");
			}

			//Slow main down to viewable pace
			Sleep(100);
		}
	}

	//Destroy the the Task Manager
	AsynchTasks::TaskManager::destroy();

	//Allow the user to see the final output
	printf("\n\n\n\n\n");
	system("PAUSE");
	return EXIT_SUCCESS;
}