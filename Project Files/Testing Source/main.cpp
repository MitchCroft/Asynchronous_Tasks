#include <iostream>

#include "../../AsyncTasks.h"

#include "BasicInput.h"
#include "Random.h"

/*
    statusToName - Convert a AsynchTask status value into a printable string
    Author: Mitchell Croft
    Created: 10/01/2017
    Modified: 10/01/2017

    param[in] pStatus - The AsynchTasks::ETaskStatus to convert to a string

    return const char* - Returns a c-string describing the status
*/
inline const char* statusToName(const AsynchTasks::ETaskStatus& pStatus) {
    switch (pStatus) {
    case AsynchTasks::ETaskStatus::Error: return "ERROR";
    case AsynchTasks::ETaskStatus::Setup: return "SETUP";
    case AsynchTasks::ETaskStatus::Pending: return "PENDING";
    case AsynchTasks::ETaskStatus::In_Progress: return "IN_PROGRESS";
    case AsynchTasks::ETaskStatus::Callback_On_Update: return "CALLBACK_ON_UPDATE";
    case AsynchTasks::ETaskStatus::Completed: return "COMPLETE";
    default: return "Unknown Status";
    }
}

/*
    normalisingVectors - Normalise a large number of randomly sized vectors to test speed
    Author: Mitchell Croft
    Created: 24/08/2016
    Modified: 10/01/2017
*/
void normalisingVectors() {
    //Define basic Vec3 struct
    struct Vec3 { float x, y, z; };

    //Store the number of worker threads to create
    int threadCount;

    //Loop until valid input
    do {
        //Clear the screen
        system("CLS");

        //Display prompt to the user
        printf("Enter the number of Worker threads to create (32 maximum): ");

        //Request the number from the user
        std::cin >> threadCount;

        //Clear input stream
        std::cin.clear();
        std::cin.ignore(2048, '\n');
    } while (threadCount >= 32);

    //Add some space on screen
    printf("\n\n\n");

    //Create the Task Manager
    if (AsynchTasks::TaskManager::create(threadCount)) {
        //Create the basic input manager
        BasicInput input = BasicInput(VK_ESCAPE, VK_SPACE);

        //Display initial instructions
        printf("Press 'SPACE' to normalise 3,000,000 Vector 3 objects (Multiple Task Test)\n\n");

        //Loop until the input breaks the loop
        while (true) {
            //Update the input
            input.update();

            //Check if the loop should exit
            if (input.keyPressed(VK_ESCAPE)) break;

            //Update the Task Manager
            AsynchTasks::TaskManager::update();

            //Output a single '.'
            printf(".");

            //Determine if space was pressed
            if (input.keyPressed(VK_SPACE)) {
                //Create a new Task
                AsynchTasks::Task<std::pair<unsigned int, float>> newTask = AsynchTasks::TaskManager::createTask<std::pair<unsigned int, float>>();

                //Force callback to print on main
                newTask->callbackOnUpdate = true;

                //Set the functions
                newTask->process = [&]() -> std::pair<unsigned int, float> {
                    //Define the number of vectors to create
                    const unsigned int ARRAY_SIZE = 3000000;

                    //Create an array of 1 mill vectors
                    Vec3* vectors = new Vec3[ARRAY_SIZE];

                    //Store the number of vectors that were successfully normalised
                    unsigned int totalNormal = 0;
                    float averageNonNormal = 0.f;
                    
                    //Give the vectors random values
                    for (unsigned int i = 0; i < ARRAY_SIZE; i++) {
                        //Assign random axis values
                        vectors[i].x = randomRange(-500.f, 500.f);
                        vectors[i].y = randomRange(-500.f, 500.f);
                        vectors[i].z = randomRange(-500.f, 500.f);

                        //Get the magnitude of the vector
                        float mag = sqrtf(SQU(vectors[i].x) + SQU(vectors[i].y) + SQU(vectors[i].z));

                        //Check the magnitude is valid
                        if (mag) {
                            //Divide along the axis
                            vectors[i].x /= mag;
                            vectors[i].y /= mag;
                            vectors[i].z /= mag;
                        }

                        //Check if the magnitude has been normalised
                        mag = sqrtf(SQU(vectors[i].x) + SQU(vectors[i].y) + SQU(vectors[i].z));
                        if (mag == 0.f || mag == 1.f)
                            totalNormal++;
                        else averageNonNormal += mag;
                    }

                    //Delete the vector objects
                    delete[] vectors;

                    //Return the final count
                    return std::pair<unsigned int, float>(totalNormal, averageNonNormal /= (float)(ARRAY_SIZE - totalNormal));
                };
                newTask->callback = [&](std::pair<unsigned int, float>& pVal) {
                    printf("\nNormalized %u of 3,000,000 Vector 3 objects. The average non-normalised magnitude was %f (Floating point error)\n", pVal.first, pVal.second);
                };

                //Add the task to the Task Manager
                if (AsynchTasks::TaskManager::addTask(newTask))
                    printf("\n\nAdded new Task to the Manager. Processing...\n");
                else printf("\n\nFailed to add the new task the Manager.\n");
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
    reusableTasks - Test reusing Task objects for similar tasks repeatedly
    Author: Mitchell Croft
    Created: 24/08/2016
    Modified: 10/01/2017
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
        stringTask->callbackOnUpdate = true;

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
            printf("\n\nCounted to: %u\n\n", pNum);
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
        std::cin.ignore(2048, '\n');

        //Switch on the received character
        switch (usrChoice) {
        case '1': normalisingVectors(); break;
        case '2': reusableTasks(); break;
        default: running = false; break;
        }
    } while (running);
    return EXIT_SUCCESS;
}