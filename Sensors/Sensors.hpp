#ifndef SENSOR_HPP
#define SENSOR_HPP

#include <string>
#include <random>

class Sensor {
public:
    Sensor(const std::string& name, int minValue = 10, int maxValue = 1000);
    int last_reading = 0; 
    int generateReading();
    std::string getName() const;
    int getLocationID() const;
    std::string getLocationName() const;
    std::string getBinaryPayload() const;
    int getLastValue() const;
    int last_value;
private:
    std::string sensor_name;
    int min_value;
    int max_value;
    int location_id;

    std::mt19937 rng;
    std::uniform_int_distribution<int> dist;
    std::uniform_int_distribution<size_t> location_index_dist;
};

#endif // SENSOR_HPP
