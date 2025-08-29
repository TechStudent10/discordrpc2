// INCREDIBLY HACKY WORKAROUND
// to get Champions support working
// until Champions releases

#pragma once

#include <Geode/loader/Dispatch.hpp>

// required for GEODE_EVENT_EXPORT
#define MY_MOD_ID "ninxout.champions"

namespace champions {
    inline geode::Result<bool> inChampionsGame() GEODE_EVENT_EXPORT(&inChampionsGame, ());
    inline geode::Result<bool> isQueueing() GEODE_EVENT_EXPORT(&isQueueing, ());
    inline geode::Result<int> getLastQueue() GEODE_EVENT_EXPORT(&getLastQueue, ());
    inline geode::Result<GJGameLevel*> getChampionsLevel() GEODE_EVENT_EXPORT(&getChampionsLevel, ());
}

#undef MY_MOD_ID
