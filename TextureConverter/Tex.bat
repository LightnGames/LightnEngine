@echo off

for %%f in (%*) do (
  "%~dp0\texconv" -f BC7_UNORM %%f -m 0
)
pause