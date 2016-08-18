#include <iostream>

#include "../../AsyncTasks.h"

#include <windows.h>

/*
	printNumber - Output a single number to the screen
	Author: Mitchell Croft
	Created: 18/08/2016
	Modified: 18/08/2016
*/
inline void printNumber(int& pNum) { 
	printf("\n\n%i\n\n", pNum); 
}

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

		//Loop till escape is pressed
		while (GetKeyState(VK_ESCAPE) < 0 == escapeState) {
			//Update the input
			spaceKey[0] = spaceKey[1];
			spaceKey[1] = GetKeyState(VK_SPACE) < 0;

			//Update the Task Manager
			AsynchTasks::TaskManager::update();

			//Output a single '.'
			printf(".");

			//Determine if the user has pressed space
			if (spaceKey[1] && !spaceKey[0]) {
				//Create a new int Task
				AsynchTasks::Task<std::string> task = AsynchTasks::TaskManager::createTask<std::string>();

				//Set callback on main flag
				task->callbackOnMain = true;

				//Set the process function
				task->process = [&]() -> std::string {
					//Create a random character string
					std::string sum;

					//Loop through and add the totals from 0 to 1000000
					for (int i = 0, size = rand(); i < size; i++) {
						sum += char(((float)rand() / (float)RAND_MAX) * float(128 - 32) + 32.f);
					}

					//Return the sum
					return sum;
				};

				//Attach the callback function
				task->callback = [&](std::string& pWord) {
					printf("\n\n%s\n\n", pWord.c_str());
				};

				//Add the Task to the Task Manager
				AsynchTasks::TaskManager::addTask(task);
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