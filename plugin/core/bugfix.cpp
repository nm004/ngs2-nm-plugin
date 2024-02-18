/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 *
 * This module is bug fixes for the game.
 */

#include "util.hpp"
#include <cstdint>

using namespace std;
using namespace ngs2::nm::util;

namespace ngs2::nm::plugin::core::bugfix {
  void
  init ()
  {
    uintptr_t monitor_in_game_setting_menu_func;
  
    switch (binary_kind)
      {
      case NGS2_BINARY_KIND::STEAM_JP:
	monitor_in_game_setting_menu_func = base_of_image + 0x13ddad0;
	break;
      case NGS2_BINARY_KIND::STEAM_AE:
	monitor_in_game_setting_menu_func = base_of_image + 0x13ddd00;
	break;
      }

    // Fix for "Return to main menu" hang up bug.
    // It sees out of the index in the official implementation.
    WriteMemory(monitor_in_game_setting_menu_func + 0x2a6 + 3, static_cast<uint8_t>(7));
  }
}
