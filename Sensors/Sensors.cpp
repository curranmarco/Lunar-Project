#include "Sensors.hpp" // Include the header file for the Sensor class
#include "/Users/marcocurran/Downloads/UNIVERSITY/3D3.2/Lunar-Project/Location_Map/location_map.hpp" // Include the shared location map
#include <chrono> // For generating random seeds based on time
#include <bitset> // For converting integers to binary strings

// Constructor: Initializes the sensor with a name, minimum and maximum values
Sensor::Sensor(const std::string& name, int minValue, int maxValue)
    : sensor_name(name), // Set the sensor's name
      min_value(minValue), // Set the minimum value for readings
      max_value(maxValue), // Set the maximum value for readings
      rng(std::chrono::steady_clock::now().time_since_epoch().count()), // Seed the random number generator with the current time
      dist(min_value, max_value), // Create a uniform distribution for sensor readings
      location_index_dist(0, location_map.size() - 1) // Create a uniform distribution for location IDs
{
    // Randomly assign a location ID to the sensor
    location_id = location_index_dist(rng);
}

// Function to generate a new sensor reading
int Sensor::generateReading() {
    last_reading = dist(rng); // Generate a random value within the specified range
    return last_reading; // Return the generated reading
}

// Function to get the name of the sensor
std::string Sensor::getName() const {
    return sensor_name; // Return the sensor's name
}

// Function to get the location ID of the sensor
int Sensor::getLocationID() const {
    return location_id; // Return the location ID
}

// Function to get the name of the location where the sensor is placed
std::string Sensor::getLocationName() const {
    return location_map.at(location_id); // Look up the location name in the shared location map
}

// Function to generate a binary payload representing the sensor's data
std::string Sensor::getBinaryPayload() const {
    std::bitset<4> loc_bits(location_id); // Convert the location ID to a 4-bit binary string
    std::bitset<16> moisture_bits(last_reading); // Convert the last reading to a 16-bit binary string
    std::bitset<12> padding(0); // Add 12 bits of zero padding
    return loc_bits.to_string() + moisture_bits.to_string() + padding.to_string(); // Concatenate the binary strings
}

// Function to get the last generated sensor reading
int Sensor::getLastValue() const {
    return last_value; // Return the last value
}
