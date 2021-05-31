Simple flutter embedder in SDL2 

to run this you need to put libflutter_engine.so from your flutter engine.

You can download your `libflutter_engine.so` from:

> curl -O https://storage.googleapis.com/flutter_infra/flutter/FLUTTER_ENGINE/linux-x64/linux-x64-embedder

replace FLUTTER_ENGINE with hash from your flutter versions:

> <path to flutter>/bin/internal/engine.version

ex:

> cat /home/charafau/Utils/flutterSdk/flutterDev/bin/internal/engine.version

> 26e217e6c3d487bb3c0cbf8a7edb378952a51d33

> curl -O https://storage.googleapis.com/flutter_infra/flutter/26e217e6c3d487bb3c0cbf8a7edb378952a51d33/linux-x64/linux-x64-embedder


after unpacking zip file simple run `run.sh` script

Dependencies:

- SDL2
- cmake

