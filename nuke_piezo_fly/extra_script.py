Import("env")

env.AddCustomTarget(
    name="format-fix",
    dependencies=None,
    actions=["cmake --build .pio/build/esp32 --target format-fix"]
)

env.AddCustomTarget(
    name="format-check",
    dependencies=None,
    actions=["cmake --build .pio/build/esp32 --target format-check"]
)
