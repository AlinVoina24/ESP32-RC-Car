#pragma once

class DistanceFilter {
    public:
        virtual float filterMeasurement(float) = 0;
        virtual void resetFilter() = 0;

        virtual ~DistanceFilter() = default;
};