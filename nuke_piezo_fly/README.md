## Project setup

```sh
components/ # Third party libraries which are not available throug platformio/esp idf.
modules/    # Implementation of our libraries.
        sensor/  # Library used to connect to all of our sensors and read data.
        storage/ # Library used to read and write memory inside the flash memory.
include/    # Header files for the application.
src/        # Application
```
