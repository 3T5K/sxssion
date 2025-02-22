#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iterator>
#include <optional>
#include <vector>
#include <string_view>
#include <print>
#include <unistd.h>
#include <getopt.h>
#include <string>
#include <format>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

constexpr std::string_view HELP =
R"(Starts an Xorg session with the specified environment.

Usage: sxssion [options] [desktop]

Options:
    -h, --help          show this message
    -c, --config <path> specify alternate config file

The json configuration file should contain an object or
array of keys to arrays of strings. The keys correspond
to desktop names and the arrays to commands for startx
to execute.
The configuration file will by default be looked up in
'$XDG_CONFIG_HOME/sxssion/desktops.json'.
)";

constexpr std::string_view 
    XINITRC_TMP = "/tmp/sxssion-tmp-xinitrc-7NeVeIYBH08OiJuwl5D4",
    XINITRC_DEF = "/etc/X11/xinit/xinitrc",
    XINITRC_DEF_EXCL_FROM = "$twm &";

const auto CONFIG_PATH_DEFAULT = fs::path(getenv("HOME"))/".config"/"sxssion"/"desktops.json";

[[noreturn]] auto die(const std::string_view msg = "", const int exit = 1) -> void {
	if (!msg.empty()) { std::println(stderr, "{}", msg); }
	std::println(stderr, "Run `sxssion --help` for more info.");
	std::exit(exit);
}

[[nodiscard]] auto parse_config(std::ifstream &cs) noexcept -> std::optional<json> {
	auto rv = [&] -> std::optional<json> {
        try         { return json::parse(cs); }
        catch (...) { return std::nullopt;    }
    }();

    if (!rv) { return rv; }

	constexpr auto check_structure = [](const auto &obj) {
		if (!obj.value().is_array()) { return false; }
		return std::ranges::all_of(obj.value().items(), [](const auto &inner) {
			return inner.value().is_string();
		});
	};

	if (!std::ranges::all_of(rv->items(), check_structure)) {
		rv = std::nullopt;
	}

	return rv;
}

auto dump_tmp_xinitrc(json::reference commands) -> void {
	std::ifstream def_xrc(fs::path{XINITRC_DEF});
	if (!def_xrc) {
		die(std::format("Failed to open default xinitrc at '{}'", XINITRC_DEF));
	}
	
	std::ofstream tmp_xrc(fs::path{XINITRC_TMP});
	if (!tmp_xrc) {
		die(std::format("Failed to open tmp xinitrc at '{}' for writing.", XINITRC_TMP));
	}

	bool line_found = false;
	for (std::string line; !std::getline(def_xrc, line).eof();) {
		if (!def_xrc) {
			die(std::format("An error occurred reading '{}'.", XINITRC_DEF));
		}

		if (line == XINITRC_DEF_EXCL_FROM) {
			line_found = true;
			break;
		}

		if (!(tmp_xrc << line << '\n')) {
			die(std::format("Failed to write to tmp file at '{}'.", XINITRC_TMP));
		}
	}

	if (!line_found) {
		die(std::format("Failed to find line '{}' to replace in the default xinitrc '{}'",
			XINITRC_DEF_EXCL_FROM, XINITRC_DEF));
	}

	for (const auto &c : commands.items()) {
		if (!(tmp_xrc << std::string{c.value()} << '\n')) {
			die(std::format("Failed to write to tmp file at '{}'.", XINITRC_TMP));
		}
	}
}

auto main(const int argc, /* const */ char *const *const argv) -> int {
	int list_flag = 0;
	std::array opts{
		option{"help",   no_argument,       nullptr, 'h'},
		option{"config", required_argument, nullptr, 'c'},
		option{"list",   no_argument,		nullptr, 'l'},
		option{0, 		 0,                 0,       0  }
	};

	fs::path config_path(CONFIG_PATH_DEFAULT);
	for (int opt; (opt = getopt_long(argc, argv, "hlc:", opts.data(), nullptr)) != -1;) {
		switch (opt) {
			case 'h': die(HELP, 0);
			case 'c': config_path = optarg; break;
			case 'l': list_flag = 1; break;
			case '?': die();
		}
	}

	const auto desktop = [argc, argv, list_flag] -> std::string_view {
		if (list_flag)          { return ""; }
		if (optind != argc - 1) { die("Expected exactly one positional argument."); }
		return argv[optind];
	}();

	if (!fs::exists(config_path)) {
		die(std::format("Config path '{}' doesn't exist.", config_path.c_str()));
	} else if (!(fs::is_regular_file(config_path) || fs::is_symlink(config_path))) {
		die(std::format("Config path '{}' is neither a regular file nor a link.", config_path.c_str()));
	}
	
	std::ifstream ifs_conf(config_path);
	if (!ifs_conf) {
		die(std::format("Failed to open config at '{}'.", config_path.c_str()));
	}

	auto conf = parse_config(ifs_conf);
	if (!conf) {
		die("Invalid config format.");
	}

	if (list_flag) {
		for (const auto &item : conf->items()) {
			std::println("{}", item.key());
		}

		std::exit(0);
	}

	if (std::ranges::find_if(conf->items(), [desktop](const auto &obj) { return obj.key() == desktop; })
		== std::end(conf->items())) {
		die(std::format("Desktop '{}' isn't in the config file.", desktop));
	}

	dump_tmp_xinitrc(conf->at(desktop));

	if (execl("/usr/bin/startx", "startx", XINITRC_TMP.data(), nullptr) == -1) {
		perror("execl");
		std::exit(1);
	}

	return 0;
}
