
#include <stdio.h>
#include "simulation/simulator_ui.h"

#include "flight_software/controllers/generic_controller.h"
#include "flight_software/drivers/generic_driver.h"
#include "simulation/mounts/generic_mount.h"

int main(void)
{

    generic_driver_t generic_driver;

    driver_operations_t ops = {
        .init = NULL,
        .sample = generic_sample,
        .calibrate = NULL,
        .emergency = NULL,
        .cleanup = NULL,
        .sample_size = sizeof(double)};

    generic_driver_init(
        &generic_driver,
        "Generic Driver",
        1,
        &ops,
        NULL,
        &instrument_generic
    );

    printf("\nInitializing UI...\n");
    initialize_UI();
    main_menu();
    return 1;
}