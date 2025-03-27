#include "socket_client.hpp"
#include "csv_sender.hpp"
#include "csv_receiver.hpp"
#include <iostream>
#include <string>
#include <vector>

class EarthControl {
private:
    SocketClient moon_connection;
    
public:
    EarthControl(const std::string& moon_ip, int moon_port) 
        : moon_connection(moon_ip, moon_port) {}

    bool connectToMoon() {
        return moon_connection.connectToServer();
    }

    bool commandRover(float degrees, float meters) {
        CsvSender sender(moon_connection.getSocket());
        std::string cmd = "MOVE," 
                        + std::to_string(degrees) + "," 
                        + std::to_string(meters);
        return sender.sendCsvData(cmd);
    }

    bool requestSensorData(const std::vector<std::string>& sensors) {
        CsvSender sender(moon_connection.getSocket());
        std::string request = "SENSOR_REQ,";
        
        for (const auto& sensor : sensors) {
            request += "," + sensor;
        }
        
        return sender.sendCsvData(request);
    }

    std::string receiveData() {
        CsvReceiver receiver(moon_connection.getSocket());
        return receiver.receiveCsvData();
    }

    void disconnect() {
        moon_connection.closeConnection();
    }
};
