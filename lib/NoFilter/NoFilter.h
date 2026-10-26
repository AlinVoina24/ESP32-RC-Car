#pragma once

#include <DistanceFilter.h>

class NoFitler : public DistanceFilter {
    public:
        float filterMeasurement(float measurement) override {
            return measurement;
        }

        void resetFilter() override {
            
        }
};