#pragma once
#include <Geode/Geode.hpp>
#include <managers/RPCManager.hpp>
#include <Geode/loader/Dispatch.hpp>

using namespace geode::prelude;

using EditorCollabListener = EventListener<DispatchFilter<>>;

class Observer {
public:
    static Observer& get() {
        static Observer instance;
        return instance;
    }

    void updateRPC();
    bool isRPCOverridden = false;

protected:
    Observer();
    
    bool isConnectedToEC = false;

    EditorCollabListener ecSocketConnectedListener = DispatchFilter<>("alk.editor-collab/socket-connected");
    EditorCollabListener ecSocketDisconnectedListener = DispatchFilter<>("alk.editor-collab/socket-disconnected");
    EditorCollabListener ecSocketAbnormallyDisconnectedListener = DispatchFilter<>("alk.editor-collab/socket-abnormally-disconnected");

    template <typename T, typename F>
    void callIf(const F&& f) const {
        if (auto arg = CCScene::get()->getChildByType<T>(0)) {
            f(arg);
        }
    }

    template <typename T>
    void setIf(RPCOptions rpc, RPCOptions& options) {
        callIf<T>([rpc, &options](T* layer) {
            options.copyFrom(rpc);
        });
    }

    static std::string stringForSearchType(SearchType type);
    static GJFeatureState getFeaturedState(GJGameLevel* level);
    static std::string stringForFeaturedState(GJFeatureState state);
    static std::string stringForFeaturedState(GJGameLevel* level);
};
