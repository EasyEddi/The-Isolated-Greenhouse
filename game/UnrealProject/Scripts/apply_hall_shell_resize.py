import os
import sys

import unreal


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)

import place_furniture_models


def main():
    unreal.EditorLevelLibrary.load_level(place_furniture_models.PROJECT_MAP)
    place_furniture_models.resize_hall_shell()
    unreal.EditorLevelLibrary.save_current_level()
    unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")


if __name__ == "__main__":
    main()
