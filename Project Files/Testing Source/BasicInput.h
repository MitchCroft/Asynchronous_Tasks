#pragma once

#include <windows.h>
#include <unordered_map>
#include <utility>

/*
 *		Name: BasicInput
 *		Author: Mitchell Croft
 *		Created: 24/08/2016
 *		Modified: 24/08/2016
 *		
 *		Purpose:
 *		Provide a simple way of testing various specific virtual
 *		keys which are defined at creation of the input object
**/
class BasicInput {
	/*----------Variables----------*/
	//! Maintain a map of the different keys to check
	std::unordered_map<int, std::pair<bool, bool>> mKeyStates;

	/*----------Functions----------*/
	//! Key extraction functions
	template<typename ... TKeys>
	inline void extractKeys(int pKey, TKeys ... pKeys);
	inline void extractKeys(int pKey);

public:
	//! Constructor allowing for multiple key checks
	template<typename ... TKeys>
	inline BasicInput(TKeys ... pKeys);
	
	//! Used to update input values
	inline void update();

	//! Input checking functions
	inline bool keyDown(int pKey);
	inline bool keyUp(int pKey);
	inline bool keyPressed(int pKey);
	inline bool keyReleased(int pKey);
};

#pragma region Function Definitions
#pragma region Private Functions
/*
	BasicInput : extractKeys - Extract the key values from variadic parameter pack
	Author: Mitchell Croft
	Created: 24/08/2016
	Modified: 24/08/2016

	param[in] pKey - The next key extracted from the parameter pack
	param[in] pKeys - The parameter pack containing the remaining keys to process
*/
template<typename ...TKeys>
inline void BasicInput::extractKeys(int pKey, TKeys ...pKeys) {
	//Add the key to the map if it does not already exist
	if (mKeyStates.find(pKey) == mKeyStates.end()) 
		mKeyStates[pKey] = std::pair<bool, bool>(false, false);

	//Move through the parameter pack
	extractKeys(pKeys ...);
}

/*
	BasicInput : extractKeys - Extract the final key value from a parameter pack
	Author: Mitchell Croft
	Created: 24/08/2016
	Modified: 24/08/2016

	param[in] pKey - The final key to add the monitored map
*/
inline void BasicInput::extractKeys(int pKey) {
	//Add the key to the map if it does not already exist
	if (mKeyStates.find(pKey) == mKeyStates.end())
		mKeyStates[pKey] = std::pair<bool, bool>(false, false);
}
#pragma endregion

#pragma region Main Functions
/*
	BasicInput : Constructor - Initialise the BasicInput object with a 
							   set of virtual keys to monitor
	Author: Mitchell Croft
	Created: 24/08/2016
	Modified: 24/08/2016

	param[in] pKeys - A parameter pack containing the virtual keys to check
*/
template<typename ...TKeys>
inline BasicInput::BasicInput(TKeys ...pKeys) {
	//Pass the keys to the extraction functions
	extractKeys(pKeys ...);
}

/*
	BasicInput : update - Update the contained virtual key states
	Author: Mitchell Croft
	Created: 24/08/2016
	Modified: 24/08/2016
*/
inline void BasicInput::update() {
	//Loop through the map elements
	for (auto& iter : mKeyStates) {
		//Save the previous state
		iter.second.first = iter.second.second;

		//Get the current state
		iter.second.second = (GetKeyState(iter.first) < 0);
	}
}
#pragma endregion

#pragma region Input State Functions
/*
	BasicInput : keyDown - Checks to see if the specified key is down
	Author: Mitchell Croft
	Created: 24/08/2016
	Modified: 24/08/2016

	param[in] pKey - The virtual key to check the state of

	return bool - Returns true if the specified virtual key is down
*/
inline bool BasicInput::keyDown(int pKey) { return mKeyStates[pKey].second; }

/*
	BasicInput : keyUp - Checks to see if the specified key is up
	Author: Mitchell Croft
	Created: 24/08/2016
	Modified: 24/08/2016

	param[in] pKey - The virtual key to check the state of

	return bool - Returns true if the specified virtual key is up
*/
inline bool BasicInput::keyUp(int pKey) { return !mKeyStates[pKey].second; }

/*
	BasicInput : keyPressed - Checks to see if the specified key was pressed
							  this update cycle
	Author: Mitchell Croft
	Created: 24/08/2016
	Modified: 24/08/2016

	param[in] pKey - The virtual key to check the state of

	return bool - Returns true if the virtual key was just pressed
*/
inline bool BasicInput::keyPressed(int pKey) { return mKeyStates[pKey].second && !mKeyStates[pKey].first; }

/*
	BasicInput : keyReleased - Checks to see if the specified key was released
							   this update cycle
	Author: Mitchell Croft
	Created: 24/08/2016
	Modified: 24/08/2016

	param[in] pKey - The virtual key to check the state of

	return bool - Returns true if the virtual key was just released
*/
inline bool BasicInput::keyReleased(int pKey) { return !mKeyStates[pKey].second && mKeyStates[pKey].first; }
#pragma endregion
#pragma endregion