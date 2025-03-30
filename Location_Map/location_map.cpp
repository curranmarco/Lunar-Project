#include "location_map.hpp"
#include <bitset>
const std::unordered_map<int, std::string> location_map = {
    {0,  "Mare Imbrium"},
    {1,  "Mare Serenitatis"},
    {2,  "Mare Tranquillitatis"},
    {3,  "Mare Crisium"},
    {4,  "Mare Nectaris"},
    {5,  "Tycho Crater"},
    {6,  "Copernicus Crater"},
    {7,  "Aristarchus Plateau"},
    {8,  "South Pole-Aitken Basin"},
    {9,  "Oceanus Procellarum"},
    {10, "Plato Crater"},
    {11, "Clavius Crater"},
    {12, "Kepler Crater"},
    {13, "Schrödinger Basin"},
    {14, "Hadley Rille"},
    {15, "Apollo 11 Landing Site"},
    {16, "Apollo 17 Taurus–Littrow"},
    {17, "Lacus Somniorum"},
    {18, "Mare Humboldtianum"},
    {19, "Marius Hills"}
};
std::string getLocationNameFromBinary(const std::string& binary_id) {
    // Validate input size (assuming 4-bit ID)
    if (binary_id.size() != 4) {
        return "Invalid ID size";
    }

    // Convert binary string to integer
    int id = std::bitset<4>(binary_id).to_ulong();

    // Try to find the ID in the map
    auto it = location_map.find(id);
    if (it != location_map.end()) {
        return it->second;
    }

    // Fallback if ID is not mapped
    return "Unknown";
}
