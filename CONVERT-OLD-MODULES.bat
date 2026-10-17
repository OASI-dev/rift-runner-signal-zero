@echo off
setlocal
pushd "%~dp0"
if not exist "main.c" (
  echo ERROR: main.c was not found. Run this file from the project folder after copying the update files.
  pause
  popd
  exit /b 1
)
if not exist "CMakeLists.txt" (
  echo ERROR: CMakeLists.txt was not found. Run this file from the project root.
  pause
  popd
  exit /b 1
)
for %%F in (audio.c background.c bullets.c colors.c effects.c enemies.c game.c helpers.c leaderboard.c pickups.c player.c ui.c world.c) do if exist "%%F" del /q "%%F"
for %%F in (audio.h background.h bullets.h colors.h effects.h enemy.h game.h pickups.h player.h) do if exist "%%F" del /q "%%F"
echo.
echo Removed old modular source files. All game code is now in main.c.
echo GitHub Desktop should show main.c changed and the obsolete files deleted.
echo Review the changes before committing.
echo.
pause
popd
