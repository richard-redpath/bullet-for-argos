//
// Created by rar500 on 12/11/15.
//

#ifndef ARGOS3_BULLET_LUA_CCLOSURE_HELPERS_H
#define ARGOS3_BULLET_LUA_CCLOSURE_HELPERS_H

#include <argos3/core/wrappers/lua/lua_utility.h>

#ifdef ARGOS_WITH_LUA

/**
 * Template method to get an argument which has been bound to a closure
 * in the type we want it
 */
template<class T>
T LUA_GetClosureArgument(lua_State* state, int argumentNo)
{
	return reinterpret_cast<T>(lua_touserdata(state, lua_upvalueindex(argumentNo)));
}

/**
 * Push a float to the LUA state
 */
inline void LUA_PushArg(lua_State* state, float arg)
{
	lua_pushnumber(state, arg);
}

/**
 * Push an int to the LUA state
 */
inline void LUA_PushArg(lua_State* state, int arg)
{
	lua_pushinteger(state, arg);
}

/**
 * Push an unsigned int to the LUA state
 */
inline void LUA_PushArg(lua_State* state, unsigned int arg)
{
	lua_pushunsigned(state, arg);
}

/**
 * Push a (C style) string to the LUA state
 */
inline void LUA_PushArg(lua_State* state, const char* arg)
{
	lua_pushstring(state, arg);
}

/**
 * Push a boolean to the LUA state
 */
inline void LUA_PushArg(lua_State* state, bool arg)
{
	lua_pushboolean(state, arg);
}

/**
 * Push a pointer to any type of argument to the LUA state
 */
template<class T>
void LUA_PushArg(lua_State* state, T* arg)
{
	lua_pushlightuserdata(state, arg);
}

/**
 * Pushes a single closure argument to the state before recursively adding the others.
 * Also increments the number of objects which have been bound
 */
template<class T, class... S>
void LUA_PushCClosureWorker(lua_State* state, lua_CFunction func, int& count, T arg, S... args)
{
	count++;
	LUA_PushArg(state, arg);
	LUA_PushCClosureWorker(state, func, count, args...);
}

/**
 * Pushes the final closure argument to the state and returns. Also increments the number of bound objects.
 */
template<class T>
void LUA_PushCClosureWorker(lua_State* state, lua_CFunction func, int& count, T arg)
{
	count++;
	LUA_PushArg(state, arg);
}

/**
 * Push a full LUA closure to the state.
 *
 * @param state The LUA state table that this closure should be added to
 * @param key The name that this function should be accessible by
 * @param func A C function pointer to the function to bind
 * @param args A collection of values which should be bound when this closure is invoked
 */
template<class... S>
void LUA_PushCClosure(lua_State* state, const std::string key, lua_CFunction func, S... args)
{
	// Push the function name
	lua_pushstring(state, key.c_str());

	// Initialise the number of closure arguments to 0
	int count = 0;

	// Initiate the pushing of arguments
	LUA_PushCClosureWorker(state, func, count, args...);

	// We now have all arguments on the stack and count tells us how many have been added
	lua_pushcclosure(state, func, count);
	lua_settable(state, -3);
}

/**
 * Convenience function to get the first argument that was bound to the closure that had been called.
 * The most frequent use case for this will likely be to bind C++ objects to invocations of C functions
 * which act as wrappers to call methods on the C++ object. As such the first (and possibly only) bound
 * argument will be the C++ object we wish to act on.
 */
template<typename C>
C* LUA_GetCallingInstance(lua_State* state)
{
	return LUA_GetClosureArgument<C*>(state, 1);
}

#endif

#endif //ARGOS3_BULLET_LUA_CCLOSURE_HELPERS_H
