#include <ToFsensor.h>
#include <Wire.h>

bool ToFsensor::begin() {
    Wire.begin();
    Wire.setClock(400000);

    this->sensor.setTimeout(100);

    if (!this->sensor.init()) {
        return false;
    }

    this->sensor.setDistanceMode(VL53L1X::Short);

    if (!this->sensor.setMeasurementTimingBudget(35000)) {
        return false;
    }

    this->sensor.startContinuous(35);

    return true;
}

float ToFsensor::getDistance() {
    if (!sensor.dataReady()) {
        return -1;
    }

    this->sensor.read(false);

    if(this->sensor.timeoutOccurred()) {
        return -1;
    }

    if (this->sensor.ranging_data.range_status != VL53L1X::RangeValid) {
        return -1;
    }

    return this->sensor.ranging_data.range_mm;
}