#include "Observer.hpp"
#include <managers/RPCManager.hpp>

#include <champions.hpp>

#ifndef GEODE_IS_WINDOWS
#include <cxxabi.h>
#endif

static std::string getNodeName(cocos2d::CCObject* node) {
#ifdef GEODE_IS_WINDOWS
    return typeid(*node).name() + 6;
#else 
    std::string ret;

    int status = 0;
    auto demangle = abi::__cxa_demangle(typeid(*node).name(), 0, 0, &status);
    if (status == 0) {
        ret = demangle;
    }
    free(demangle);

    return ret;
#endif
}

std::string splitByCapitals(const std::string& input) {
    std::string result;
    std::string current;

    for (auto character : input) {
        if (std::isupper(character) || std::isdigit(character)) {
            if (!current.empty()) {
                result += " " + current;
                current.clear();
            }
        }
        current += character;
    }

    if (!current.empty()) {
        result += " " + current;
    }

    return result;
}

Observer::Observer() {
    auto thread = std::thread([this]() {
        while (true) {
            updateRPC();

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
        }
    });
    thread.detach();

    ecSocketConnectedListener.bind([this]() {
        isConnectedToEC = true;
        return ListenerResult::Propagate;
    });
    ecSocketDisconnectedListener.bind([this]() {
        isConnectedToEC = false;
        return ListenerResult::Propagate;
    });
    ecSocketAbnormallyDisconnectedListener.bind([this]() {
        isConnectedToEC = false;
        return ListenerResult::Propagate;
    });
}

// stolen, er, borrowed from BetterInfo (https://github.com/Cvolton/betterinfo-geode/blob/master/src/utils/TimeUtils.cpp)
std::string workingTime(int value){
    if(value < 0) return fmt::format("NA ({})", value);
    if(value == 0) return "NA";

    int hours = value / 3600;
    int minutes = (value % 3600) / 60;
    int seconds = value % 60;

    std::ostringstream stream;
    if(hours > 0) stream << hours << "h ";
    if(minutes > 0) stream << minutes << "m ";
    stream << seconds << "s";

    return stream.str();
}

void Observer::updateRPC() {
    if (isRPCOverridden) return;
    Loader::get()->queueInMainThread([this]() {
        RPCOptions options;
        RPCOptions fallback;

        // fallback in case we forget to add a layer
        callIf<CCLayer>([&fallback](CCLayer* layer) {
            auto nodeName = getNodeName(layer);
            if (geode::utils::string::contains(nodeName, "cocos2d")) {
                // no fallback here
                return;
            } 
            fallback.copyFrom({
                .details = fmt::format("On the {}", splitByCapitals(
                    geode::utils::string::replaceIP(
                        geode::utils::string::replaceIP(nodeName, "GJ", ""), "Layer", "Screen"
                    )
                )),
            });
        });

        setIf<MenuLayer>({
            .details = "Main Menu"
        }, options);
        setIf<CreatorLayer>({
            .details = "Playing online"
        }, options);
        setIf<LevelSearchLayer>({
            .details = "Searching for levels"
        }, options);
        setIf<LeaderboardsLayer>({
            .details = "Browsing the leaderboards"
        }, options);
        setIf<GauntletSelectLayer>({
            .details = "Selecting a gauntlet"
        }, options);
        setIf<LevelSelectLayer>({
            .details = "Exploring the main levels"
        }, options);
        setIf<GJGarageLayer>({
            .details = "Customizing their look"
        }, options);
        setIf<SecretRewardsLayer>({
            .details = "In the Treasure Room"
        }, options);
        setIf<RewardsPage>({
            .details = "Opening Chests"
        }, options);

        setIf<SecretLayer>({
            .details = "In the Vault"
        }, options);
        setIf<SecretLayer2>({
            .details = "Uncovering Secrets in the Vault"
        }, options);
        setIf<SecretLayer3>({
            .details = "In the basement"
        }, options);
        setIf<SecretLayer4>({
            .details = "In the Chamber of Time"
        }, options);
        setIf<SecretLayer5>({
            .details = "Trying codes with the Wraith"
        }, options);

        setIf<LevelAreaLayer>({
            .details = "Outside the Tower"
        }, options);
        setIf<LevelAreaInnerLayer>({
            .details = "Exploring the Tower"
        }, options);

        // these all need to take priority over CreatorLayer, so they need to be called after
        setIf<ChallengesPage>({
            .details = "Checking out the Quests"
        }, options);
        setIf<GJPathsLayer>({
            .details = "Unlocking Paths"
        }, options);
        callIf<DailyLevelPage>([&options](DailyLevelPage* layer) {
            std::string levelType;
            switch (layer->m_type) {
                case GJTimedLevelType::Daily:
                    levelType = "daily";
                    break;
                case GJTimedLevelType::Weekly:
                    levelType = "weekly";
                    break;
                case GJTimedLevelType::Event:
                    levelType = "event";
                    break;
            }
            options.copyFrom({
                .details = fmt::format("Checking out the {} level", levelType),
            });
        });

        callIf<LevelInfoLayer>([&options](LevelInfoLayer* layer) {
            auto level = layer->m_level;
            auto starCount = level->m_stars.value();
            options.copyFrom({
                .state = "Viewing level",
                .details = fmt::format("{} by {}", level->m_levelName, level->m_creatorName),
                .smallImage = RPCManager::getAssetKey(level),
                .smallImageText = starCount != 0 ?
                    fmt::format("{} Stars | {}", starCount, stringForFeaturedState(level)) : "Unrated"
            });
        });
        callIf<LevelEditorLayer>([this, &options](LevelEditorLayer* layer) {
            // TODO: hide sensitive info if user says to
            auto level = layer->m_level;
            auto sensitiveMode = Mod::get()->getSettingValue<bool>("private-info");
            options.copyFrom({
                .state = isConnectedToEC ? "Editing a level in EditorCollab" : "Editing a level",
                .details = fmt::format("Working on {}", sensitiveMode ? level->m_levelName : "a level"),
                .largeImage = isConnectedToEC ? "https://github.com/editor-collab/ClientUI/blob/main/logo.png?raw=true" : "gd-large",
                .smallImage = "editor",
                .smallImageText = sensitiveMode ? (isConnectedToEC ?
                    fmt::format("{} objects", layer->m_objectCount.value()) :
                    fmt::format("Worked on for {} | {} objects", workingTime(level->m_workingTime), layer->m_objectCount.value())) : ""
            });
        });
        callIf<LevelBrowserLayer>([&options](LevelBrowserLayer* layer) {
            options.copyFrom({
                .details = stringForSearchType(layer->m_searchObject->m_searchType)
            });
        });
        callIf<GauntletLayer>([&options](GauntletLayer* layer) {
            auto name = GauntletNode::nameForType(layer->m_gauntletType);

            options.copyFrom({
                .details = fmt::format("Playing the {} Gauntlet", name)
            });
        });
        callIf<GJShopLayer>([&options](GJShopLayer* layer) {
            std::unordered_map<ShopType, std::string> shopkeepers = {
                { ShopType::Normal, "The Shopkeeper" },
                { ShopType::Secret, "Scratch" },
                { ShopType::Community, "Potbor" },
                { ShopType::Mechanic, "The Mechanic" },
                { ShopType::Diamond, "The Diamond Shopkeeper"}
            };
            std::string shopkeeper = shopkeepers[layer->m_type];
            options.copyFrom({
                .details = fmt::format("In {}'s shop", shopkeeper)
            });
        });
        callIf<PlayLayer>([&options](PlayLayer* layer) {
            auto level = layer->m_level;

            std::string details = fmt::format(
                "{} by {}",
                level->m_levelName,
                level->m_levelType == GJLevelType::Local ? "RobTopGames" : level->m_creatorName
            );
            if (level->m_levelType == GJLevelType::Editor) {
                details = "Playtesting a created level";
            }

            std::string bestString;
            if (level->isPlatformer()) {
                int sec = round(level->m_bestTime / 1000);
                bestString = std::to_string(sec) + "s";

                if (level->m_bestTime == 0) {
                    bestString = "No Best Time";
                }
            } else {
                bestString = std::to_string(level->m_normalPercent.value()) + "%";
            }

            std::string state = (layer->m_isPracticeMode ? "Practicing" : "Playing");
            if (level->isPlatformer()) {
                state = state + " a platformer";
            }

            bool isDaily = level->m_dailyID.value() != 0;
            bool isDemon = level->m_demon.value() != 0;

            if (isDaily) {
                if (isDemon) {
                    state = state + " the weekly";
                }
                else {
                    state = state + " the daily";
                }
            }

            if (state == "Playing") {
                state = "Playing a";
            }

            state = fmt::format("{} level (best: {})", state, bestString);
        
            auto inChampions = champions::inChampionsGame().unwrapOr(false);
            if (inChampions) {
                state = "Playing in a Champions game";
            }

            auto sensitiveMode = Mod::get()->getSettingValue<bool>("private-info");
            auto starCount = level->m_stars.value();

            if (sensitiveMode && level->m_unlisted) {
                state = "Playing a level";
                details = "Unlisted.";
                starCount = 0;
            }

            std::vector<RPCButton> buttons({
                {
                    .title = "Open level in browser",
                    .url = fmt::format("https://gdbrowser.com/{}", level->m_levelID)
                }
            });
            std::vector<RPCButton> noButtons;

            options.copyFrom({
                .state = state,
                .details = details,
                .buttons = sensitiveMode ? buttons : noButtons,
                .largeImage = inChampions ? "https://cdn.discordapp.com/icons/1398898354145984612/872e7357735435a48e24d04a557e4467.png?size=480" : "gd-large",
                .smallImage = RPCManager::getAssetKey(level),
                .smallImageText = starCount != 0 ?
                    fmt::format("{} Stars | {}", starCount, stringForFeaturedState(level)) : "Unrated",
            });
        });

        if (champions::isQueueing().unwrapOr(false)) {
            std::unordered_map<int, std::string> queueStrings = {
                { 0, "Easy | Medium" },
                { 1, "Hard | Insane" },
                { 2, "Extreme" }
            };
            std::unordered_map<int, std::string> queueImages = {
                { 0, "easy_demon" },
                { 1, "hard_demon" },
                { 2, "extreme_demon" }
            };

            int lastQueue = champions::getLastQueue().unwrapOr(0);
            options = {
                .state = "Queueing into Champions",
                .details = fmt::format("{} queue", queueStrings[lastQueue]),
                .largeImage = "https://cdn.discordapp.com/icons/1398898354145984612/872e7357735435a48e24d04a557e4467.png?size=480",
                .smallImage = queueImages[lastQueue],
            };
        }

        RPCManager::get().setRPC(options.details == "" ? fallback : options);
    });
}

GJFeatureState Observer::getFeaturedState(GJGameLevel* level) {
    if (level->m_isEpic == 3) {
        return GJFeatureState::Mythic;
    } else if (level->m_isEpic == 2) {
        return GJFeatureState::Legendary;
    } else if (level->m_isEpic == 1) {
        return GJFeatureState::Epic;
    } else if (level->m_featured > 0) {
        return GJFeatureState::Featured;
    } else {
        return GJFeatureState::None;
    }
}

std::string Observer::stringForFeaturedState(GJFeatureState state) {
    switch (state) {
        case GJFeatureState::None: return "Rate-only";
        case GJFeatureState::Featured: return "Featured";
        case GJFeatureState::Epic: return "Epic";
        case GJFeatureState::Legendary: return "Legendary";
        case GJFeatureState::Mythic: return "Mythic";
    }
}

std::string Observer::stringForFeaturedState(GJGameLevel* level) {
    return stringForFeaturedState(
        getFeaturedState(
            level
        )
    );
}

std::string Observer::stringForSearchType(SearchType type) {
    std::string res = "Looking at online levels";
    // generated with the help of chatgpt
    // (i am NOT writing all this by hand)
    switch (type) {
        case SearchType::Search:
            res = "Looking for new levels to play";
            break;
        case SearchType::Downloaded:
            res = "Looking at downloaded levels";
            break;
        case SearchType::MostLiked:
            res = "Browsing most liked levels";
            break;
        case SearchType::Trending:
            res = "Browsing trending levels";
            break;
        case SearchType::Recent:
            res = "Browsing recently added levels";
            break;
        case SearchType::UsersLevels:
            res = "Viewing user custom levels";
            break;
        case SearchType::Featured:
            res = "Browsing featured levels";
            break;
        case SearchType::Magic:
            res = "Exploring magic levels";
            break;
        case SearchType::Sends:
            res = "Looking at levels to rate (hi RubRub)";
            break;
        case SearchType::Sent:
            res = "Looking at recent sends";
            break;
        case SearchType::MapPack:
            res = "Browsing map packs";
            break;
        case SearchType::MapPackOnClick:
            res = "Viewing a specific map pack";
            break;
        case SearchType::Awarded:
            res = "Viewing awarded levels";
            break;
        case SearchType::Followed:
            res = "Viewing levels from followed users";
            break;
        case SearchType::Friends:
            res = "Viewing friends' levels";
            break;
        case SearchType::Users:
            res = "Searching for users";
            break;
        case SearchType::LikedGDW:
            res = "Viewing liked levels (GDW)";
            break;
        case SearchType::HallOfFame:
            res = "Browsing the Hall of Fame";
            break;
        case SearchType::FeaturedGDW:
            res = "Browsing featured levels (GDW)";
            break;
        case SearchType::Similar:
            res = "Browsing similar levels";
            break;
        case SearchType::Type19:
            res = "Viewing unknown type (19)";
            break;
        case SearchType::DailySafe:
            res = "Viewing safe daily levels";
            break;
        case SearchType::WeeklySafe:
            res = "Viewing safe weekly levels";
            break;
        case SearchType::EventSafe:
            res = "Viewing safe event levels";
            break;
        case SearchType::Reported:
            res = "Browsing reported levels";
            break;
        case SearchType::LevelListsOnClick:
            res = "Viewing selected level list";
            break;
        case SearchType::Type26:
            res = "Viewing unknown type (26)";
            break;
        case SearchType::FeaturedLite:
            res = "Browsing featured levels (Lite)";
            break;
        case SearchType::Bonus:
            res = "Exploring bonus levels";
            break;
        case SearchType::MyLevels:
            res = "Looking at created levels";
            break;
        case SearchType::SavedLevels:
            res = "Looking at saved levels";
            break;
        case SearchType::FavouriteLevels:
            res = "Viewing favourite levels";
            break;
        case SearchType::SmartTemplates:
            res = "Browsing smart templates";
            break;
        case SearchType::MyLists:
            res = "Viewing level lists";
            break;
        case SearchType::FavouriteLists:
            res = "Viewing favourite level lists";
            break;
        default:
            break;
    }

    return res;
}
