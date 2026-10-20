#include <RemoteControl.h>

RemoteControl::RemoteControl(UltraSounds& sensorReference, int period):
    sensor(sensorReference),
    sampleTimePeriod(period)
{  
}

float RemoteControl::getMedian() {
    int N = 5;

    for (int i = 0; i < N - 1; i++) {
    int minIndex = i;

    for (int j = i + 1; j < N; j++) {
        if (this->values[j] < this->values[minIndex]) {
        minIndex = j;
        }
    }

    if (minIndex != i) {
        float temp = this->values[i];
        this->values[i] = this->values[minIndex];
        this->values[minIndex] = temp;
    }
    }

    return this->values[N / 2];
}

void RemoteControl::calculateDistance() {
    unsigned long now = millis();

    if (!this->distanceReady && now - this->lastSampleTime >= this->sampleTimePeriod) {
      this->lastSampleTime = now;
      this->values[this->sampleIndex++] = this->sensor.getFilteredDistance();
      
      if (this->sampleIndex >= 5) {
        this->distanceToGraph = getMedian();
        this->sampleIndex = 0;
        this->distanceReady = true;
      } 
    }
  
}

float RemoteControl::getDistanceToGraph() {
    if (this->distanceReady) {
        this->distanceReady = false;
        return this->distanceToGraph;
    } else {
        return -1;
    }
}