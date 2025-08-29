#pragma once
#include <Geode/Geode.hpp>

enum LevelDifficulty {
    NA,
    Auto,
    Easy,
    Normal,
    Hard,
    Harder,
    Insane,
    EasyDemon,
    MediumDemon,
    HardDemon,
    InsaneDemon,
    ExtremeDemon
};

struct RPCButton {
    std::string title = "";
    std::string url = "";

    bool operator==(const RPCButton&) const = default;
};

struct RPCOptions {
    /* Required */
    std::string state;

    /* Optional */
    std::string details = "";

    bool shouldDisplayTime = false;
    int64_t startTimestamp = 0;
    int64_t endTimestamp = 0;

    std::string partyID = "";
    int partySize = 0;
    int maxPartySize = 0;

    std::vector<RPCButton> buttons = {};

    std::string largeImage = "";
    std::string largeImageText = "";

    std::string smallImage = "";
    std::string smallImageText = "";

    bool operator==(const RPCOptions&) const = default;
    void copyFrom(RPCOptions options) {
#define COPY(arg) this->arg = options.arg;
        COPY(state)
        COPY(details)
        COPY(shouldDisplayTime)
        COPY(startTimestamp)
        COPY(endTimestamp)
        COPY(partyID)
        COPY(partySize)
        COPY(maxPartySize)
        COPY(buttons)
        COPY(largeImage)
        COPY(largeImageText)
        COPY(smallImage)
        COPY(smallImageText)
#undef COPY
    }
};

class RPCManager {
protected:
    bool hasInitializedRPC = false;
    RPCOptions currentOptions;
    RPCManager() {}
public:
    static RPCManager& get() {
        static RPCManager instance;
        return instance;
    }

    std::string defaultLargeImage = "";
    std::string defaultLargeImageText = "";
    std::string defaultState = "";

    time_t startTime;

    void initRPC();
    void clearRPC();
    void setRPC(RPCOptions options);

    RPCOptions getCurrentRPC() {
        return currentOptions;
    }

    static LevelDifficulty getDifficultyFromLevel(GJGameLevel* level);
    static std::string getAssetKey(LevelDifficulty difficulty);
    static std::string getAssetKey(GJGameLevel* level) {
        return getAssetKey(
            getDifficultyFromLevel(
                level
            )
        );
    }
};