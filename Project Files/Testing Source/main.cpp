#include <iostream>

#include "../../AsyncTasks.h"
#include "BasicInput.h"

/*
	normalisingVectors - Normalise a large number of randomly sized vectors to test speed
	Author: Mitchell Croft
	Created: ##/08/2016
	Modified: ##/08/2016
*/
void normalisingVectors() {

}

/*
	reusableTasks - Test reusing Task objects for similar tasks repeatedly
	Author: Mitchell Croft
	Created: 24/08/2016
	Modified: 24/08/2016
*/
void reusableTasks() {
	//Clear the screen
	system("CLS");

	//Create the Task Manager
	if (AsynchTasks::TaskManager::create(1)) {
		//Create the basic input manager
		BasicInput input = BasicInput(VK_ESCAPE, VK_SPACE);

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
		printf("Press 'SPACE' to count to UINT_MAX when Task object is available (Reusable Task Test)\n\n");

		//Loop till escape is pressed
		while (true) {
			//Update the input
			input.update();

			//Check if the loop should exit
			if (input.keyPressed(VK_ESCAPE)) break;

			//Update the Task Manager
			AsynchTasks::TaskManager::update();

			//Output a single '.'
			if (stringTask->status != AsynchTasks::ETaskStatus::Completed && stringTask->status != AsynchTasks::ETaskStatus::Setup)
				printf(".");

			//Determine if the user has pressed space
			if (input.keyPressed(VK_SPACE)) {
				//Add the task to the Task Manager
				if (AsynchTasks::TaskManager::addTask(stringTask))
					printf("Starting to count now:\n");
			}

			//Slow main down to viewable pace
			Sleep(100);
		}
	}

	//Display error message
	else printf("Failed to create the Asynchronous Task Manager\n");

	//Destroy the the Task Manager
	AsynchTasks::TaskManager::destroy();

	//Allow the user to see the final output
	printf("\n\n\n\n\n");
	system("PAUSE");
}

/*
	main - Test the current implementation of the AsynchTasks namespace
	Author: Mitchell Croft
	Created: 18/08/2016
	Modified: 18/08/2016
*/
int main() {
	//Store the user input
	char usrChoice;

	//Flag if test program is running
	bool running = true;

	//Loop so the user can choose the different tests
	do {
		//Clear the screen
		system("CLS");

		//Display the options
		printf("Implemented Test:\n" \
			   "1. Normalising Vectors\n" \
			   "2. Reusable Tasks\n\n");

		//Prompt user for choice
		printf("Enter the desired test (Invalid character to quit): ");

		//Receive input selection from the user
		std::cin >> usrChoice;

		//Clear the input
		std::cin.clear();
		std::cin.ignore(256, '\n');

		//Switch on the received character
		switch (usrChoice) {
		case '1': normalisingVectors(); break;
		case '2': reusableTasks(); break;
		default: running = false; break;
		}
	} while (running);
	return EXIT_SUCCESS;
}