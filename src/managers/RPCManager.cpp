#include "RPCManager.hpp"
#include <discord-rpc.hpp>
#include <Geode/loader/Log.hpp>

using namespace geode::log;

#define APPLICATION_ID "1178492879627366472"

void RPCManager::initRPC() {
    if (hasInitializedRPC) return;
    discord::RPCManager::get()
        .setClientID(APPLICATION_ID)
        .onReady([](discord::User const& user) {
            geode::log::info("connected to user {}#{} (user id: {})", user.username, user.discriminator, user.id);
        })
        .onDisconnected([](int errcode, std::string_view message) {
            geode::log::error("disconnected with error code {} - message: {}", errcode, message);
        })
        .initialize();
}

void RPCManager::clearRPC() {
    discord::RPCManager::get().clearPresence();
}

void RPCManager::setRPC(RPCOptions options) {
    if (options == currentOptions) return;

    currentOptions = options;

    auto& presence = discord::Presence::get();
    presence.clear();

    presence
        .setState(options.state.empty() ? defaultState : options.state)
        .setDetails(options.details)
        .setLargeImageKey(
            options.largeImage.empty() ? defaultLargeImage : options.largeImage
        )
        .setLargeImageText(
            options.largeImage.empty() ? defaultLargeImageText : options.largeImageText
        )
        .setSmallImageKey(options.smallImage)
        .setSmallImageText(options.smallImageText)
        .setStartTimestamp(options.startTimestamp != 0 ? options.startTimestamp : startTime)
        .setEndTimestamp(options.endTimestamp)
        .setPartyID(options.partyID)
        .setPartySize(options.partySize)
        .setPartyMax(options.maxPartySize)
        .setInstance(false);

    if (options.buttons.size() >= 1) {
        if (!options.buttons[0].title.empty()) {
            presence.setButton1(options.buttons[0].title, options.buttons[0].url);
        }
    }
    if (options.buttons.size() >= 2) {
        if (!options.buttons[1].title.empty()) {
            presence.setButton2(options.buttons[1].title, options.buttons[1].url);
        }
    }

    presence.refresh();
}

LevelDifficulty RPCManager::getDifficultyFromLevel(GJGameLevel* level) {
    if (level->m_autoLevel) {
        return Auto;
    }
    if (level->m_demon == 1) {
        switch (level->m_demonDifficulty) {
            case 3:
                return EasyDemon;
            case 4:
                return MediumDemon;
            case 0:
                return HardDemon;
            case 5:
                return InsaneDemon;
            case 6:
                return ExtremeDemon;
        }
    }
    #define DIFF_INT(diff) static_cast<int>(GJDifficulty::diff)
    auto diff = level->getAverageDifficulty();
    if (level->m_levelType == GJLevelType::Local) {
        diff = static_cast<int>(level->m_difficulty);
    }
    switch (diff) {
        case 0:
            return NA;
        case DIFF_INT(Easy):
            return Easy;
        case DIFF_INT(Normal):
            return Normal;
        case DIFF_INT(Hard):
            return Hard;
        case DIFF_INT(Harder):
            return Harder;
        case DIFF_INT(Insane):
            return Insane;
    }
    #undef DIFF_INT
    return NA;
}

std::string RPCManager::getAssetKey(LevelDifficulty difficulty) {
    switch (difficulty) {
        case NA:            return "na";
        case Auto:          return "auto";
        case Easy:          return "easy";
        case Normal:        return "normal";
        case Hard:          return "hard";
        case Harder:        return "harder";
        case Insane:        return "insane";
        case EasyDemon:     return "easy_demon";
        case MediumDemon:   return "medium_demon";
        case HardDemon:     return "hard_demon";
        case InsaneDemon:   return "insane_demon";
        case ExtremeDemon:  return "extreme_demon";
    }
}
