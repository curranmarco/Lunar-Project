#include "Sensors.hpp"
#include "../location_map/location_map.hpp"
#include <iostream>

int main() {
    // Create a Sensor instance
    Sensor lunarSensor("Lunar-M1");

    // Test getName()
    std::cout << "[Sensor Name]       " << lunarSensor.getName() << "\n";

    // Test getLocationID() and getLocationName()
    int loc_id = lunarSensor.getLocationID();
    std::string loc_name = lunarSensor.getLocationName();
    std::cout << "[Location ID]       " << loc_id << "\n";
    std::cout << "[Location Name]     " << loc_name << "\n";

    // Test generateReading()
    int moisture = lunarSensor.generateReading();
    std::cout << "[Moisture Reading]  " << moisture << "\n";

    // Test getBinaryPayload() (after generateReading so value is stored)
    std::string binaryPayload = lunarSensor.getBinaryPayload();
    std::cout << "[Binary Payload]    " << binaryPayload << "\n";

  /*  // Decode just to confirm (optional)
    std::bitset<4> decodedLoc(binaryPayload.substr(0, 4));
    std::bitset<16> decodedMoisture(binaryPayload.substr(4, 16));

    std::cout << "[Decoded Loc ID]    " << decodedLoc.to_ulong() << "\n";
    std::cout << "[Decoded Moisture]  " << decodedMoisture.to_ulong() << "\n";*/

    return 0;
}
