#include <UltraSounds.h>

class RemoteControl {
    private:
        UltraSounds& sensor;
        unsigned long lastSampleTime = 0;
        int sampleTimePeriod;
        int sampleIndex = 0;
        float values[5];
        bool distanceReady = false;
        float distanceToGraph = 0;

    public:
        RemoteControl(UltraSounds&, int);

        void calculateDistance();
        float getMedian();
        float getDistanceToGraph();

};