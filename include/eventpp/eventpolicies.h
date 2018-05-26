// eventpp library
// Copyright (C) 2018 Wang Qi (wqking)
// Github: https://github.com/wqking/eventpp
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//   http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef EVENTOPTIONS_H_730367862613
#define EVENTOPTIONS_H_730367862613

#include <mutex>
#include <atomic>
#include <condition_variable>
#include <map>
#include <unordered_map>

namespace eventpp {

namespace internal_ {

struct DummyMutex
{
	void lock() {}
	void unlock() {}
};

} //namespace internal_

struct MultipleThreading
{
	using Mutex = std::mutex;

	template <typename T>
	using Atomic = std::atomic<T>;

	using ConditionVariable = std::condition_variable;
};

struct SingleThreading
{
	using Mutex = internal_::DummyMutex;

	// May replace Atomic with dummy atomic later.
	template <typename T>
	using Atomic = std::atomic<T>;

	// May replace ConditionVariable with dummy condition variable later.
	using ConditionVariable = std::condition_variable;
};

struct ArgumentPassingAutoDetect
{
	enum {
		canIncludeEventType = true,
		canExcludeEventType = true
	};
};

struct ArgumentPassingIncludeEvent
{
	enum {
		canIncludeEventType = true,
		canExcludeEventType = false
	};
};

struct ArgumentPassingExcludeEvent
{
	enum {
		canIncludeEventType = false,
		canExcludeEventType = true
	};
};

struct DefaultPolicies
{
	/* default types for CallbackList
	using Callback = std::function<blah, blah>;
	using Threading = MultipleThreading;
	*/

	/* default types/implements for EventDispatch and EventQueue
	template <typename U, typename ...Args>
	static E getEvent(U && e, const Args &...) {
		return e;
	}

	using Callback = std::function<blah, blah>;
	using Threading = MultipleThreading;
	using ArgumentPassingMode = ArgumentPassingAutoDetect;

	template <typename Key, typename T>
	using Map = std::map <Key, T>;
	*/
};

namespace internal_ {

template <typename T>
struct HasTypeArgumentPassingMode
{
	template <typename C> static std::true_type test(typename C::ArgumentPassingMode *) ;
	template <typename C> static std::false_type test(...);    

	enum { value = !! decltype(test<T>(0))() };
};
template <typename T, bool> struct SelectArgumentPassingMode;
template <typename T> struct SelectArgumentPassingMode <T, true> { using Type = typename T::ArgumentPassingMode; };
template <typename T> struct SelectArgumentPassingMode <T, false> { using Type = ArgumentPassingAutoDetect; };

template <typename T>
struct HasTypeThreading
{
	template <typename C> static std::true_type test(typename C::Threading *) ;
	template <typename C> static std::false_type test(...);    

	enum { value = !! decltype(test<T>(0))() };
};
template <typename T, bool> struct SelectThreading;
template <typename T> struct SelectThreading <T, true> { using Type = typename T::Threading; };
template <typename T> struct SelectThreading <T, false> { using Type = MultipleThreading; };

template <typename T>
struct HasTypeCallback
{
	template <typename C> static std::true_type test(typename C::Callback *) ;
	template <typename C> static std::false_type test(...);    

	enum { value = !! decltype(test<T>(0))() };
};
template <typename T, bool, typename D> struct SelectCallback;
template <typename T, typename D> struct SelectCallback<T, true, D> { using Type = typename T::Callback; };
template <typename T, typename D> struct SelectCallback<T, false, D> { using Type = D; };

template <typename T>
class HasFunctionGetEvent
{
	template <typename C> static std::true_type test(decltype(&C::getEvent) *) ;
	template <typename C> static std::false_type test(...);    

public:
	enum { value = !! decltype(test<T>(0))() };
};
template <typename E>
struct DefaultGetEvent
{
	template <typename U, typename ...Args>
	static E getEvent(U && e, const Args &...) {
		return e;
	}
};
template <typename T, typename Key, bool> struct SelectGetEvent;
template <typename T, typename Key> struct SelectGetEvent<T, Key, true> { using Type = T; };
template <typename T, typename Key> struct SelectGetEvent<T, Key, false> { using Type = DefaultGetEvent<Key>; };

template <typename T>
struct HasTemplateMap
{
	template <typename C> static std::true_type test(typename C::template Map<int, int> *) ;
	template <typename C> static std::false_type test(...);    

	enum { value = !! decltype(test<T>(0))() };
};
template <typename T>
class HasHash
{
	template <typename C> static std::true_type test(decltype(std::hash<C>()(std::declval<C>())) *) ;
	template <typename C> static std::false_type test(...);    

public:
	enum { value = !! decltype(test<T>(0))() };
};
template <typename Key, typename Value, typename T, bool>
struct SelectMap;
template <typename Key, typename Value, typename T>
struct SelectMap<Key, Value, T, true>
{
	using Type = typename T::template Map<Key, Value>;
};
template <typename Key, typename Value, typename T>
struct SelectMap<Key, Value, T, false> {
	using Type = typename std::conditional<
		HasHash<Key>::value,
		std::unordered_map<Key, Value>,
		std::map<Key, Value>
	>::type;
};


} //namespace internal_


} //namespace eventpp


#endif
