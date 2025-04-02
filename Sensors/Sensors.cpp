#include "Sensors.hpp"
#include "/Users/marcocurran/Downloads/UNIVERSITY/3D3.2/Lunar-Project/Location_Map/location_map.hpp"  // Include the shared map
#include <chrono>
#include <bitset>

Sensor::Sensor(const std::string& name, int minValue, int maxValue)
    : sensor_name(name), min_value(minValue), max_value(maxValue),
      rng(std::chrono::steady_clock::now().time_since_epoch().count()),
      dist(min_value, max_value),
      location_index_dist(0, location_map.size() - 1)
{
    location_id = location_index_dist(rng);  // Randomly pick a location ID
}

int Sensor::generateReading() {
    last_reading = dist(rng);  // Update the last reading
    return last_reading;
}

std::string Sensor::getName() const {
    return sensor_name;
}

int Sensor::getLocationID() const {
    return location_id;
}

std::string Sensor::getLocationName() const {
    return location_map.at(location_id);
}

std::string Sensor::getBinaryPayload() const {
    std::bitset<4> loc_bits(location_id);
    std::bitset<16> moisture_bits(last_reading);  // Use the stored reading
    std::bitset<12> padding(0);  // 12-bit zero padding
    return loc_bits.to_string() + moisture_bits.to_string() + padding.to_string();
}
int Sensor::getLastValue() const {
    return last_value;
}
