# ModeManagerService

This project is for creating ModeManager service, Command line(cli) and share library(for Unity). The following functions are available

1. show
   The show API will show application.
2. hide
   The hide API will hide application.
3. changeLayout
   It will pass action data to Launcher(RN) onLayoutChangeRequest function.
4. launchApp
   The launchApp API will record component id in JSON file.(../app_user/packages/com.stellantis.service.modemanager/data/key_Recent_app.json)
5. closeApp
   The closeApp API to remove component id in JSON file.(../app_user/packages/com.stellantis.service.modemanager/data/key_Recent_app.json)

## Folder Structure

```
.
└── ModeManagerService/
    ├── common/
    │   └── ipc/
    │       ├── ModeManagerService.acs
    │       └── CMakeLists.txt
    ├── service/
    │   ├── include/
    │   │   ├── ModeManagerService.h
    │   │   └── ModeManagerServiceApp.h
    │   ├── src/
    │   │   ├── App.cpp
    │   │   ├── ModeManagerService.cpp
    │   │   └── ModeManagerServiceApp.cpp
    │   ├── CMakeLists.txt
    │   └── manifest.toml
    ├── build.sh
    ├── unity-vera-build.sh
    ├── CMakeLists.txt
    └── README.md
```


## Build

For x86_64

```
./build.sh x86_64
```

For aarch64

```
./build.sh aarch64
```

unity-vera-build.sh is for jenkis build
