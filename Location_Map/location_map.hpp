#ifndef LOCATION_MAP_HPP
#define LOCATION_MAP_HPP

#include <unordered_map>
#include <string>

// Shared mapping from numeric location IDs to actual lunar location names
extern const std::unordered_map<int, std::string> location_map;

#endif // LOCATION_MAP_HPP
