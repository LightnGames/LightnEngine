@echo off

for %%f in (%*) do (
  L:\LightnEngine\tools\python\FBXConverter.exe %%f
)
pause