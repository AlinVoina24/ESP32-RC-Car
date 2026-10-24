#pragma once

class DistanceSensor {
    public:
        virtual bool begin() = 0;
        virtual float getDistance() = 0;

        virtual ~DistanceSensor() = default;
};