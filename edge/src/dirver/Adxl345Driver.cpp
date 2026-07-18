#include "Adxl345Driver.hpp"

#include <string>
#include <stdexcept>
#include <array>
#include <cstdint>

Adxl345Driver::Adxl345Driver(
    const std::string& device_path,
    const Config& config
)
    : m_bus{device_path},
    m_config{config}
{
    verify_device();

    set_standby_mode();
    set_odr();
    set_range_and_resolution();
    set_measurement_mode();
};

void Adxl345Driver::verify_device()
{
    const std::uint8_t device_id{
        m_bus.read_register(
            DEVICE_ADDRESS,
            REG_DEVID
        )
    };

    if (device_id != EXPECTED_DEVICE_ID)
    {
        throw std::runtime_error(
            "Adxl345Driver::verify_device: unexpected device ID" +
            std::to_string(device_id)
        );
    }
}

void Adxl345Driver::set_odr()
{
    m_bus.write_register(
        DEVICE_ADDRESS,
        REG_BW_RATE,
        static_cast<std::uint8_t>(m_config.odr)
    );
}

void Adxl345Driver::set_range_and_resolution()
{
    std::uint8_t value{
        static_cast<std::uint8_t>(m_config.range)
    };

    if (m_config.fullResolution)
    {
        // OR 연산 및 대입
        value |= FULL_RES_BIT;
    }
    
    m_bus.write_register(
        DEVICE_ADDRESS,
        REG_DATA_FORMAT,
        value
    );
}

void Adxl345Driver::set_measurement_mode()
{
    const std::uint8_t current_value{
        m_bus.read_register(
            DEVICE_ADDRESS,
            REG_POWER_CTL
        )
    };

    const std::uint8_t new_value{
        static_cast<std::uint8_t>(
            current_value | MEASURE_BIT
        )
    };

    m_bus.write_register(
        DEVICE_ADDRESS,
        REG_POWER_CTL,
        new_value
    );
}

void Adxl345Driver::set_standby_mode()
{
    const std::uint8_t current_value{
        m_bus.read_register(
            DEVICE_ADDRESS,
            REG_POWER_CTL
        )
    };

    const std::uint8_t new_value{
        static_cast<std::uint8_t>(
            current_value & 
            static_cast<uint8_t>(~MEASURE_BIT)
        )
    };

    m_bus.write_register(
        DEVICE_ADDRESS,
        REG_POWER_CTL,
        new_value
    );
}

Acceleration Adxl345Driver::read_acceleration()
{
    std::array<std::uint8_t, 6> raw_data{};

    m_bus.read_registers(
        DEVICE_ADDRESS,
        REG_DATAX0,
        raw_data.data(),
        raw_data.size()
    );

    return convert_raw_value(raw_data);
}

Acceleration Adxl345Driver::convert_raw_value(
    const std::array<uint8_t, 6>& raw_arr
) const
{
    // 1. X, Y, Z 각각 상위 8비트 하위 8비트 합치기
    const std::int16_t raw_x{
        combine_bytes(raw_arr[0], raw_arr[1])
    };

    const std::int16_t raw_y{
        combine_bytes(raw_arr[2], raw_arr[3])
    };

    const std::int16_t raw_z{
        combine_bytes(raw_arr[4], raw_arr[5])
    };

    const double scale_factor{
        get_scale_factor()
    };

    return Acceleration{
        static_cast<double>(raw_x) * scale_factor,
        static_cast<double>(raw_y) * scale_factor,
        static_cast<double>(raw_z) * scale_factor
    };
}

double Adxl345Driver::get_scale_factor() const
{
    if(m_config.fullResolution)
    {
        return 0.0039;
    }

    switch (m_config.range)
    {
        case Range::G2:
            return 0.0039;

        case Range::G4:
            return 0.0078;

        case Range::G8:
            return 0.0156;

        case Range::G16:
            return 0.0312;
    }

    throw std::logic_error(
        "Adxl345Driver::get_scale_factor: invalid range"
    );
}