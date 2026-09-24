/* Copyright (C) 2019 Mr Goldberg
   This file is part of the Goldberg Emulator

   The Goldberg Emulator is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   The Goldberg Emulator is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Goldberg Emulator; if not, see
   <http://www.gnu.org/licenses/>.  */

/*
* docs. ref.
* 
* steamworks sdk
* 
* https://partner.steamgames.com/doc/sdk/api#callbacks
* https://partner.steamgames.com/doc/sdk/api#callresults
* 
*/

#ifndef __INCLUDED_CALLBACK_SYSTEM_H__
#define __INCLUDED_CALLBACK_SYSTEM_H__

#ifdef _WIN32
#pragma once
#endif

#include <vector>
#include <chrono>
#include <map>
#include "revCommon.h"

#define DEFAULT_CALLBACK_TIMEOUT 0.002
#define STEAM_CALLRESULT_TIMEOUT 120.0
#define STEAM_CALLRESULT_WAIT_FOR_CB 0.01

/*
* CallbackMgr
*/
class CCallbackMgr
{
public:
    static void SetRegister(class CCallbackBase* pCallback, int iCallback);
    static void SetUnregister(class CCallbackBase* pCallback);

    static bool IsServer(class CCallbackBase* pCallback);
};

/*
* SteamCallResult
*/
struct SteamCallResult 
{
    SteamAPICall_t api_call{};
    std::vector<class CCallbackBase*> callbacks{};
    std::vector<char> result{};
    std::chrono::high_resolution_clock::time_point created{};

    bool    to_delete = false;
    bool    reserved = false;
    double  run_in{};
    bool    run_call_completed_cb{};
    int     iCallback{};

    SteamCallResult(SteamAPICall_t a, int icb, void* r, unsigned int s, double r_in, bool run_cc_cb);

    bool operator==(const struct SteamCallResult& other) const;

    bool timed_out() const;
    bool call_completed() const;
    bool can_execute() const;
    bool has_cb() const;
};

/*
* Contains all SteamCallResults
*/
class SteamCallResults 
{
    std::vector<struct SteamCallResult> callresults{};
    std::vector<class CCallbackBase*> completed_callbacks{};
    void (*cb_all)(std::vector<char> result, int callback) = nullptr;

public:
    void AddCallCompleted(class CCallbackBase* cb);
    void RemoveCallCompleted(class CCallbackBase* cb);
    void AddCallback(SteamAPICall_t api_call, class CCallbackBase* cb);
    bool Exists(SteamAPICall_t api_call) const;
    bool CallbackResult(SteamAPICall_t api_call, void* copy_to, unsigned int size);
    void RemoveCallback(SteamAPICall_t api_call, class CCallbackBase* cb);
    void RemoveCallback(class CCallbackBase* cb);
    SteamAPICall_t AddCallResult(SteamAPICall_t api_call, int iCallback, void* result, unsigned int size, double timeout = DEFAULT_CALLBACK_TIMEOUT, bool run_call_completed_cb = true);
    SteamAPICall_t ReserveCallResult();
    SteamAPICall_t AddCallResult(int iCallback, void* result, unsigned int size, double timeout = DEFAULT_CALLBACK_TIMEOUT, bool run_call_completed_cb = true);
    void SetCbAll(void (*cb_all)(std::vector<char> result, int callback));
    void Clear();
    void RunCallResults();
};

/*
* SteamCallback
*/
struct SteamCallback 
{
    std::vector<class CCallbackBase*>   callbacks{};
    std::vector<std::vector<char>>      results{};
};

/*
* Contains all SteamCallbacks
*/
class SteamCallbacks 
{
    std::map<int, struct SteamCallback> callbacks{};
    SteamCallResults* results{};

public:
    SteamCallbacks(SteamCallResults* results);

    void AddCallback(int iCallback, class CCallbackBase* cb);
    void AddCallbackResult(int iCallback, void* result, unsigned int size, double timeout, bool dont_post_if_already);
    void AddCallbackResult(int iCallback, void* result, unsigned int size);
    void AddCallbackResult(int iCallback, void* result, unsigned int size, bool dont_post_if_already);
    void AddCallbackResult(int iCallback, void* result, unsigned int size, double timeout);

    void RemoveCallback(int iCallback, class CCallbackBase* cb);

    void RunCallbacks();
};

/*
* RunCBs
*/
struct RunCBs 
{
    void (*function)(void* object) = nullptr;
    void* object{};
};

class RunEveryRunCB 
{
    std::vector<struct RunCBs> cbs{};

public:
    void Add(void (*cb)(void* object), void* object);
    void Remove(void (*cb)(void* object), void* object);
    void Run() const;
};

#endif // __INCLUDED_CALLBACK_SYSTEM_H__