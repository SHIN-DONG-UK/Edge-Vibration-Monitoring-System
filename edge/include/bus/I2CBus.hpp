#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
using namespace std;

class I2CBus{
    public:
        // 생성자
        explicit I2CBus(const std::string& device_path);
        // 소멸자
        ~I2CBus();

        I2CBus(const I2CBus&) = delete;             // 복사 생성 금지
        I2CBus& operator=(const I2CBus&&) = delete; // 복사 대입 금지
        
        void write_register(
            uint8_t device_address,
            uint8_t register_address,
            uint8_t value
        );

        uint8_t read_register(
            uint8_t device_address,
            uint8_t register_address
        );

        void read_registers(
            uint8_t device_address,
            uint8_t start_address,
            uint8_t* buffer,
            size_t length
        );

    private:
        int m_fd{-1};   // member var: file descriptor
}