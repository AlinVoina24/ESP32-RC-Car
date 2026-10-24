#pragma once

#include <DistanceFilter.h>

class KalmanFilter : public DistanceFilter {
private:
    float estimate;
    float errorCovariance;

    float Q;
    float R;

    bool initialized = false;

public:
    KalmanFilter(float Q, float R)
        : Q(Q), R(R), errorCovariance(1.0f)
    {
    }

    float filterMeasurement(float measurement) {

        if (!initialized) {
            estimate = measurement;
            initialized = true;

            return estimate;
        }

        // Prediction
        errorCovariance =
            errorCovariance + Q;

        // Kalman gain
        float K =
            errorCovariance /
            (errorCovariance + R);

        // Correction
        estimate =
            estimate +
            K * (measurement - estimate);

        errorCovariance =
            (1.0f - K) *
            errorCovariance;

        return estimate;
    }

    void resetFilter() {
        this->initialized = false;
    }
};