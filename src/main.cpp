#include <Geode/Geode.hpp>
#include <managers/RPCManager.hpp>
#include <managers/Observer.hpp>

std::string getSysName() {
#ifdef GEODE_IS_WINDOWS
	if (!geode::utils::platform::isWine()) {
    	return "Windows";
    }
    else {
    	return "Linux";
    }
#else
    return "macOS";
#endif
}

$on_mod(Loaded) {
    auto& rpcManager = RPCManager::get();
    rpcManager.initRPC();
    rpcManager.defaultState = "Browsing menus";
    rpcManager.defaultLargeImage = "gd-large";
    rpcManager.defaultLargeImageText = fmt::format("Playing Geometry Dash on {}", getSysName());
    rpcManager.startTime = time(0);

    // start doing things
    Observer::get();
}