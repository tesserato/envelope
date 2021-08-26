# Updates the Dll and the exe from the compiled Cpp files
Copy-Item -Path "Cpp/Execs/DLL.dll" -Destination "signal_envelope/envelope.dll"

# Updates the executable and necessary dll
Copy-Item -Path "Cpp/Execs/libsndfile-1.dll" -Destination "executable/libsndfile-1.dll"
Copy-Item -Path "Cpp/Execs/x64_Release.exe" -Destination "executable/envelope.exe"