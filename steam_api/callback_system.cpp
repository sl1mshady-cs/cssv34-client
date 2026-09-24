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

#include "callback_system.h"
#include "tier0/dbg.h"
#include "logging.h"

CLoggingFile g_CallbackLog("rev-callbacks.log");
extern std::recursive_mutex global_mutex;

#define PRINT_DEBUG(msg, ...) g_CallbackLog.Write(msg "\n", __VA_ARGS__)

void randombytes(char* buf, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        buf[i] = (unsigned char)(rand() & 0xFF);
    }
}

SteamAPICall_t generate_steam_api_call_id() {
    static SteamAPICall_t a;
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    randombytes((char*)&a, sizeof(a));
    ++a;
    if (a == 0) ++a;
    return a;
}

void CCallbackMgr::SetRegister(class CCallbackBase* pCallback, int iCallback)
{
    pCallback->m_nCallbackFlags |= CCallbackBase::k_ECallbackFlagsRegistered;
    pCallback->m_iCallback = iCallback;
};

void CCallbackMgr::SetUnregister(class CCallbackBase* pCallback)
{
    if (pCallback)
        pCallback->m_nCallbackFlags &= !CCallbackBase::k_ECallbackFlagsRegistered;
};

bool CCallbackMgr::IsServer(class CCallbackBase* pCallback)
{
    return (pCallback->m_nCallbackFlags & CCallbackBase::k_ECallbackFlagsGameServer) != 0;
};



SteamCallResult::SteamCallResult(SteamAPICall_t a, int icb, void* r, unsigned int s, double r_in, bool run_cc_cb)
{
    api_call = a;
    result.resize(s);
    if (s > 0 && r != NULL) {
        memcpy(&(result[0]), r, s);
    }
    run_in = r_in;
    run_call_completed_cb = run_cc_cb;
    iCallback = icb;
    created = std::chrono::high_resolution_clock::now();
}

bool SteamCallResult::operator==(const struct SteamCallResult& other) const
{
    return other.api_call == api_call && other.callbacks == callbacks;
}

bool SteamCallResult::timed_out() const
{
    return check_timedout(created, STEAM_CALLRESULT_TIMEOUT);
}

bool SteamCallResult::call_completed() const
{
    return (!reserved) && check_timedout(created, run_in);
}

bool SteamCallResult::can_execute() const
{
    return (!to_delete) && call_completed() && (has_cb() || check_timedout(created, STEAM_CALLRESULT_WAIT_FOR_CB));
}

bool SteamCallResult::has_cb() const
{
    return callbacks.size() > 0;
}



void SteamCallResults::AddCallCompleted(class CCallbackBase* cb)
{
    if (std::find(completed_callbacks.begin(), completed_callbacks.end(), cb) == completed_callbacks.end()) {
        completed_callbacks.push_back(cb);
        PRINT_DEBUG("new cb for call complete notification [result k_iCallback=%i] %p", cb ? (cb->GetICallback()) : -1, cb);
    }
}

void SteamCallResults::RemoveCallCompleted(class CCallbackBase* cb)
{
    auto c = std::find(completed_callbacks.begin(), completed_callbacks.end(), cb);
    if (c != completed_callbacks.end()) {
        completed_callbacks.erase(c);
        PRINT_DEBUG("removed cb for call complete notification [result k_iCallback=%i] %p", cb ? (cb->GetICallback()) : -1, cb);
    }
}

void SteamCallResults::AddCallback(SteamAPICall_t api_call, class CCallbackBase* cb)
{
    auto cb_result = std::find_if(callresults.begin(), callresults.end(), [api_call](struct SteamCallResult const& item) { return item.api_call == api_call; });
    if (cb_result != callresults.end()) {
        cb_result->callbacks.push_back(cb);
        CCallbackMgr::SetRegister(cb, cb->GetICallback());
        PRINT_DEBUG("new cb for call result [api id=%llu, result k_iCallback=%i] %p", api_call, cb ? (cb->GetICallback()) : -1, cb);
    }
}

bool SteamCallResults::Exists(SteamAPICall_t api_call) const
{
    auto cr = std::find_if(callresults.begin(), callresults.end(), [api_call](struct SteamCallResult const& item) {
        return item.api_call == api_call;
        });
    if (callresults.end() == cr) return false;
    if (!cr->call_completed()) return false;
    return true;
}

bool SteamCallResults::CallbackResult(SteamAPICall_t api_call, void* copy_to, unsigned int size)
{
    auto cb_result = std::find_if(callresults.begin(), callresults.end(), [api_call](struct SteamCallResult const& item) {
        return item.api_call == api_call;
        });
    if (cb_result != callresults.end()) {
        if (!cb_result->call_completed()) return false;

        // Real Steam just copies nothing and returns true if nullptr is passed.
        if (!copy_to) {
            cb_result->to_delete = true;
            return true;
        }

        memset(copy_to, 0, size);

        size_t resultsize = cb_result->result.size();
        if (size > resultsize) {
            PRINT_DEBUG("provided buffer is larger than callback data (%u > %u), code should possibly be updated to new sdk",
                size,
                resultsize);
        }

        // Output buffer being too small is not a failure condition.
        // This behavior is needed for RemoteStorageFileShareResult_t which was made larger in sdk v1.28x
        // without even the interface version being bumped.
        size_t outsize = min(resultsize, static_cast<size_t>(size));
        memcpy(copy_to, cb_result->result.data(), outsize);
        cb_result->to_delete = true;
        return true;
    }
    else {
        return false;
    }
}

void SteamCallResults::RemoveCallback(SteamAPICall_t api_call, class CCallbackBase* cb)
{
    auto cb_result = std::find_if(callresults.begin(), callresults.end(), [api_call](struct SteamCallResult const& item) { return item.api_call == api_call; });
    if (cb_result != callresults.end()) {
        auto it = std::find(cb_result->callbacks.begin(), cb_result->callbacks.end(), cb);
        if (it != cb_result->callbacks.end()) {
            cb_result->callbacks.erase(it);
            CCallbackMgr::SetUnregister(cb);
            PRINT_DEBUG("removed cb for call result [api id=%llu, result k_iCallback=%i] %p", api_call, cb ? (cb->GetICallback()) : -1, cb);
        }
    }
}

void SteamCallResults::RemoveCallback(class CCallbackBase* cb)
{
    //TODO: check if callback is callback or call result?
    for (auto& cr : callresults) {
        auto it = std::find(cr.callbacks.begin(), cr.callbacks.end(), cb);
        if (it != cr.callbacks.end()) {
            cr.callbacks.erase(it);
            PRINT_DEBUG("removed cb %p, kind=%i (0=callback, 1=call result)", cb, (int)cr.run_call_completed_cb);
        }

        if (cr.callbacks.size() == 0) {
            cr.to_delete = true;
        }
    }
}

SteamAPICall_t SteamCallResults::AddCallResult(SteamAPICall_t api_call, int iCallback, void* result, unsigned int size, double timeout, bool run_call_completed_cb)
{
    PRINT_DEBUG("%i", iCallback);
    auto cb_result = std::find_if(callresults.begin(), callresults.end(), [api_call](struct SteamCallResult const& item) { return item.api_call == api_call; });
    if (cb_result != callresults.end()) {
        // only change the data if this is a previously reserved callresult
        if (cb_result->reserved) {
            std::chrono::high_resolution_clock::time_point created = cb_result->created;
            std::vector<class CCallbackBase*> temp_cbs = cb_result->callbacks;
            *cb_result = SteamCallResult(api_call, iCallback, result, size, timeout, run_call_completed_cb);
            cb_result->callbacks = temp_cbs;
            cb_result->created = created;
            return cb_result->api_call;
        }
    }
    else {
        struct SteamCallResult res = SteamCallResult(api_call, iCallback, result, size, timeout, run_call_completed_cb);
        callresults.push_back(res);
        return callresults.back().api_call;
    }

    PRINT_DEBUG("ERROR");
    return k_uAPICallInvalid;
}

SteamAPICall_t SteamCallResults::ReserveCallResult()
{
    struct SteamCallResult res = SteamCallResult(generate_steam_api_call_id(), 0, NULL, 0, 0.0, true);
    res.reserved = true;
    callresults.push_back(res);
    return callresults.back().api_call;
}

SteamAPICall_t SteamCallResults::AddCallResult(int iCallback, void* result, unsigned int size, double timeout, bool run_call_completed_cb)
{
    return AddCallResult(generate_steam_api_call_id(), iCallback, result, size, timeout, run_call_completed_cb);
}

void SteamCallResults::SetCbAll(void (*cb_all)(std::vector<char> result, int callback))
{
    this->cb_all = cb_all;
}

void SteamCallResults::Clear()
{
    callresults.clear();
}

void SteamCallResults::RunCallResults()
{
    unsigned long current_size = static_cast<unsigned long>(callresults.size());
    for (unsigned i = 0; i < current_size; ++i) {
        unsigned index = i;

        if (!callresults[index].to_delete) {
            if (callresults[index].can_execute()) {
                std::vector<char> result = callresults[index].result;
                SteamAPICall_t api_call = callresults[index].api_call;
                bool run_call_completed_cb = callresults[index].run_call_completed_cb;
                int iCallback = callresults[index].iCallback;
                if (run_call_completed_cb) {
                    callresults[index].run_call_completed_cb = false;
                }

                callresults[index].to_delete = true;
                if (callresults[index].has_cb()) {
                    std::vector<class CCallbackBase*> temp_cbs = callresults[index].callbacks;
                    for (auto& cb : temp_cbs) {
                        PRINT_DEBUG("Calling callresult %p %i, kind=%i (0=callback, 1=call result)", cb, cb->GetICallback(), (int)run_call_completed_cb);
                        global_mutex.unlock();

                        //TODO: unlock relock doesn't work if mutex was locked more than once.
                        if (run_call_completed_cb) { //run the right function depending on if it's a callback or a call result.
                            cb->Run(&(result[0]), false, api_call);
                        }
                        else { // if this is a callback
                            cb->Run(&(result[0]));
                        }

                        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                        //COULD BE DELETED SO DON'T TOUCH CB
                        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                        global_mutex.lock();
                        PRINT_DEBUG("callresult done");
                    }
                }

                if (run_call_completed_cb) {
                    //can it happen that one is removed during the callback?
                    std::vector<class CCallbackBase*> callbacks = completed_callbacks;
                    SteamAPICallCompleted_t data{};
                    data.m_hAsyncCall = api_call;
                    data.m_iCallback = iCallback;
                    data.m_cubParam = (uint32)result.size();

                    for (auto& cb : callbacks) {
                        PRINT_DEBUG("Calling complete cb %p %i %llu", cb, iCallback, api_call);
                        //TODO: check if this is a problem or not.
                        SteamAPICallCompleted_t temp = data;
                        global_mutex.unlock();
                        cb->Run(&temp);
                        global_mutex.lock();
                    }

                    if (cb_all) {
                        std::vector<char> res{};
                        res.resize(sizeof(data));
                        memcpy(&(res[0]), &data, sizeof(data));
                        cb_all(res, data.k_iCallback);
                    }
                }
                else {
                    if (cb_all) {
                        cb_all(result, iCallback);
                    }
                }
            }
            else {
                if (callresults[index].timed_out()) {
                    callresults[index].to_delete = true;
                }
            }
        }
    }

    // PRINT_DEBUG("erase to_delete");
    auto c = std::begin(callresults);
    while (c != std::end(callresults)) {
        if (c->to_delete) {
            if (c->timed_out()) {
                PRINT_DEBUG("removed callresult %i", c->iCallback);
                c = callresults.erase(c);
            }
            else {
                ++c;
            }
        }
        else {
            ++c;
        }
    }
}



SteamCallbacks::SteamCallbacks(SteamCallResults* results)
{
    this->results = results;
}

void SteamCallbacks::AddCallback(int iCallback, class CCallbackBase* cb)
{
    PRINT_DEBUG("%i", iCallback);
    if (iCallback == SteamAPICallCompleted_t::k_iCallback) { // if this is a call result "call completed cb"
        results->AddCallCompleted(cb);
        CCallbackMgr::SetRegister(cb, iCallback);
        return;
    }

    if (std::find(callbacks[iCallback].callbacks.begin(), callbacks[iCallback].callbacks.end(), cb) == callbacks[iCallback].callbacks.end()) {
        callbacks[iCallback].callbacks.push_back(cb);
        PRINT_DEBUG("new cb for callback [result k_iCallback=%i] %p", iCallback, cb);
        CCallbackMgr::SetRegister(cb, iCallback);
        for (auto& res : callbacks[iCallback].results) {
            //TODO: timeout?
            SteamAPICall_t api_id = results->AddCallResult(iCallback, &(res[0]), static_cast<unsigned long>(res.size()), 0.0, false);
            results->AddCallback(api_id, cb);
        }
    }
}

void SteamCallbacks::AddCallbackResult(int iCallback, void* result, unsigned int size, double timeout, bool dont_post_if_already)
{
    if (dont_post_if_already) {
        for (auto& r : callbacks[iCallback].results) {
            if (r.size() == size) {
                if (memcmp(&(r[0]), result, size) == 0) {
                    //cb already posted
                    return;
                }
            }
        }
    }

    std::vector<char> temp{};
    temp.resize(size);
    memcpy(&(temp[0]), result, size);
    callbacks[iCallback].results.push_back(temp);
    for (auto cb : callbacks[iCallback].callbacks) {
        SteamAPICall_t api_id = results->AddCallResult(iCallback, result, size, timeout, false);
        results->AddCallback(api_id, cb);
    }

    if (callbacks[iCallback].callbacks.empty()) {
        results->AddCallResult(iCallback, result, size, timeout, false);
    }
}

void SteamCallbacks::AddCallbackResult(int iCallback, void* result, unsigned int size)
{
    AddCallbackResult(iCallback, result, size, DEFAULT_CALLBACK_TIMEOUT, false);
}

void SteamCallbacks::AddCallbackResult(int iCallback, void* result, unsigned int size, bool dont_post_if_already)
{
    AddCallbackResult(iCallback, result, size, DEFAULT_CALLBACK_TIMEOUT, dont_post_if_already);
}

void SteamCallbacks::AddCallbackResult(int iCallback, void* result, unsigned int size, double timeout)
{
    AddCallbackResult(iCallback, result, size, timeout, false);
}

void SteamCallbacks::RemoveCallback(int iCallback, class CCallbackBase* cb)
{
    if (iCallback == SteamAPICallCompleted_t::k_iCallback) {
        results->RemoveCallCompleted(cb);
        CCallbackMgr::SetUnregister(cb);
        return;
    }

    auto c = std::find(callbacks[iCallback].callbacks.begin(), callbacks[iCallback].callbacks.end(), cb);
    if (c != callbacks[iCallback].callbacks.end()) {
        callbacks[iCallback].callbacks.erase(c);
        CCallbackMgr::SetUnregister(cb);
        PRINT_DEBUG("removed cb for callback [result k_iCallback=%i] %p", iCallback, cb);
        results->RemoveCallback(cb);
    }
}

void SteamCallbacks::RunCallbacks()
{
    for (auto& c : callbacks) {
        c.second.results.clear();
    }
}



void RunEveryRunCB::Add(void (*cb)(void* object), void* object)
{
    Remove(cb, object);
    RunCBs rcb{};
    rcb.function = cb;
    rcb.object = object;
    cbs.push_back(rcb);
}

void RunEveryRunCB::Remove(void (*cb)(void* object), void* object)
{
    auto c = std::begin(cbs);
    while (c != std::end(cbs)) {
        if (c->function == cb && c->object == object) {
            c = cbs.erase(c);
        }
        else {
            ++c;
        }
    }
}

void RunEveryRunCB::Run() const
{
    std::vector<struct RunCBs> temp_cbs = cbs;
    for (auto c : temp_cbs) {
        c.function(c.object);
    }
}