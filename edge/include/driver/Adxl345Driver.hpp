// ADXL345를 제어하는 인터페이스 드라이버
#include "I2CBus.hpp"

#include <cstdint>
#include <string>
#include <array>

// 가속도 데이터 형식
struct Acceleration
{
    double x{};
    double y{};
    double z{};
};

class Adxl345Driver
{
public:
    enum class OutputDataRate : std::uint8_t 
    {
        Hz0_10 = 0x00,  // 0.10 Hz
        Hz0_20 = 0x01,  // 0.20 Hz
        Hz0_39 = 0x02,  // 0.39 Hz
        Hz0_78 = 0x03,  // 0.78 Hz
        Hz1_56 = 0x04,  // 1.56 Hz
        Hz3_13 = 0x05,  // 3.13 Hz
        Hz6_25 = 0x06,  // 6.25 Hz
        Hz12_5 = 0x07,  // 12.5 Hz
        Hz25   = 0x08,  // 25 Hz
        Hz50   = 0x09,  // 50 Hz
        Hz100  = 0x0A,  // 100 Hz
        Hz200  = 0x0B,  // 200 Hz
    };

    enum class Range : std::uint8_t
    {
        G2  = 0x00,     // +-2 g
        G4  = 0x01,     // +-4 g
        G8  = 0x02,     // +-8 g
        G16 = 0x03      // +-16 g
    };
    
    struct Config
    {
        OutputDataRate odr{OutputDataRate::Hz200};
        Range range{Range::G4};
        bool fullResolution{true};
    };

    // 생성자: ADXL345 장치 초기화 및 측정 시작
    Adxl345Driver(
        const std::string& device_path,
        const Config& config
    );

    // 소멸자: 자원 반납
    ~Adxl345Driver();

    Adxl345Driver(const Adxl345Driver&) = delete;               // 복사 생성자 삭제
    Adxl345Driver& operator=(const Adxl345Driver&) = delete;    // 복사 대입 생성자 삭제

    // read acceleration data
    Acceleration read_acceleration();

private:
    static constexpr std::uint8_t DEVICE_ADDRESS  = 0x53;

    static constexpr std::uint8_t REG_DEVID       = 0x00;
    static constexpr std::uint8_t REG_BW_RATE     = 0x2C;
    static constexpr std::uint8_t REG_POWER_CTL   = 0x2D;
    static constexpr std::uint8_t REG_DATA_FORMAT = 0x31;
    static constexpr std::uint8_t REG_DATAX0      = 0x32;

    static constexpr std::uint8_t EXPECTED_DEVICE_ID = 0xE5;
    static constexpr std::uint8_t MEASURE_BIT        = 0x08;
    static constexpr std::uint8_t FULL_RES_BIT       = 0x08;

    void verify_device();
    void set_odr();
    void set_range_and_resolution();

    void set_measurement_mode();
    void set_standby_mode();

    // util function
    Acceleration convert_raw_value(const std::array<uint8_t, 6>& raw_arr) const;
    static std::int16_t combine_bytes(std::uint8_t low, std::uint8_t high);
    double get_scale_factor() const;

    I2CBus m_bus;
    Config m_config;
};