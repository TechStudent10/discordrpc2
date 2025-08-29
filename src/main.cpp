#include <Geode/Geode.hpp>
#include <managers/RPCManager.hpp>
#include <managers/Observer.hpp>

using namespace geode::prelude;

std::string getSysName() {
#ifdef GEODE_WINDOWS
    return "Windows";
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

// this is here solely to keep backwards compatability; mods should NOT use this API
$execute {
	using NewRPCFilter = geode::DispatchFilter<std::string>;
	new EventListener<NewRPCFilter>(+[](std::string const& newRPCStr) {
		auto newRPCRes = matjson::parse(newRPCStr);
		if (newRPCRes.isErr()) {
			log::error("err updating rpc: {}", newRPCRes.err()->message);
			return ListenerResult::Propagate;
		}
        auto& rpcManager = RPCManager::get();
		auto newRPC = newRPCRes.unwrap();
        rpcManager.setRPC({
            .state = newRPC["state"].asString().unwrapOr(""),
            .details = newRPC["details"].asString().unwrapOr(""),
            .maxPartySize = static_cast<int>(newRPC["partyMax"].asInt().unwrapOr(1)),
            .largeImage = newRPC["largeImageKey"].asString().unwrapOr(""),
            .largeImageText = newRPC["largeImageText"].asString().unwrapOr(""),
            .smallImage = newRPC["smallImageKey"].asString().unwrapOr(""),
            .smallImageText = newRPC["smallImageText"].asString().unwrapOr(""),

        });
		// gdrpc::GDRPC::getSharedInstance()->updateDiscordRP(
		// 	newRPC["modID"].asString().unwrapOr(""),
		// 	,
		// 	,
		// 	,
		// 	,
		// 	newRPC["useTime"].asBool().unwrapOr(false),
		// 	newRPC["shouldResetTime"].asBool().unwrapOr(false),
		// 	,
		// 	0,
		// 	newRPC["joinSecret"].asString().unwrapOr(""),
		// 	,
			
		// );
		return ListenerResult::Propagate;
	}, NewRPCFilter("update_rpc"_spr));

	using ToggleRPCFilter = geode::DispatchFilter<bool>;
	new EventListener(+[](bool enabled) {
		Observer::get().isRPCOverridden = !enabled;
		return ListenerResult::Propagate;
	}, ToggleRPCFilter("set_default_rpc_enabled"_spr));
};
