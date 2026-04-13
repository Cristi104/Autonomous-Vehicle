/usr/bin/cmake -DCMAKE_BUILD_TYPE=Debug -S . -B ./cmake-build-debug-remote-host
/usr/bin/cmake --build ./cmake-build-debug-remote-host --target controller -- -j 14
chmod +x run.sh
cp ./cmake-build-debug-remote-host/controller.cpython-313-aarch64-linux-gnu.so .
