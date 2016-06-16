//
// Created by rar500 on 03/11/15.
//

#ifndef ARGOS3_BULLET_STRINGFUNCS_H
#define ARGOS3_BULLET_STRINGFUNCS_H

#include <string>
#include <sstream>

/*
 * Template definition to parse space separated strings into any number of parameters of mixed type
 * Used here primarily to extract numeric values from XML attributes
 */
template<typename T>
inline void extractFromStringStream(std::stringstream* ss, T &a)
{
	// Extract the final value we are interested in
	(*ss) >> a;
}

/*
 * Specific overrides for floats to handle "NaN" values
 * This one is for float as a final argument
 */
template<>
inline void extractFromStringStream<float>(std::stringstream* ss, float &a)
{
	// Extract as a string
	std::string tmp;
	(*ss)>>tmp;

	// Is string "NaN"?
	if(tmp.compare("NaN") == 0)
		a = std::numeric_limits<float>::quiet_NaN();	// Set to NaN
	else
		std::stringstream{tmp} >> a;					// Otherwise get the value
}

/*
 * As above for floats in the middle of argument lists
 */
template<typename T, typename... S>
inline void extractFromStringStream(std::stringstream* ss, float &a, S&... args)
{
	std::string tmp;
	(*ss)>>tmp;
	if(tmp.compare("NaN") == 0)
		a = std::numeric_limits<float>::quiet_NaN();
	else
		std::stringstream{tmp} >> a;
	extractFromStringStream(ss, args...);
}

/*
 * Generic case which will extract a value from the stream and call recursively
 * to get the remaining values
 */
template<typename T, typename... S>
inline void extractFromStringStream(std::stringstream* ss, T &a, S&... args)
{
	(*ss) >> a;
	extractFromStringStream(ss, args...);
}

/*
 * This is the one which will most likely be invoked
 */
template<typename... T>
inline void extractFromString(std::string str, T &... args)
{
	std::stringstream ss{str};
	extractFromStringStream(&ss, args...);
}

/**
 * Returns true iff the first string ends with the second
 */
inline bool endsWith(const std::string& mainString, const std::string& end)
{
	int mainLength = mainString.length();
	int endLength = end.length();

	// Impossible to be true now
	if(mainLength < endLength)
		return false;

	// End matches
	if(mainString.compare(mainLength-endLength, endLength, end) == 0)
		return true;

	// Could have been true but isn't
	return false;
}

#endif //ARGOS3_BULLET_STRINGFUNCS_H
