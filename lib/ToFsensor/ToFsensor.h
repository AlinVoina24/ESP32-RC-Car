#pragma once

#include <DistanceSensor.h>

#include <VL53L1X.h>

class ToFsensor : public DistanceSensor {
    private:
        VL53L1X sensor;
    
    public:
    
        bool begin() override;
        float getDistance() override;

};