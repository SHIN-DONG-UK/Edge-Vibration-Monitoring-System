#include "I2CBus.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <stdexcept>
#include <iostream>

// 1. 생성자: system call open
I2CBus::I2CBus(const std::string& device_path) 
{
    m_fd = ::open(device_path.c_str(), O_RDWR);

    if (m_fd == -1)
    {
        throw std::runtime_error("I2CBus: failed to open I2C device");
    }
}

I2CBus::~I2CBus() 
{
    // 유효한 파일 디스크립터가 없으면 그냥 리턴
    if (m_fd < 0)
    {
        return;
    }

    const int fd = m_fd;
    m_fd = -1;
    
    if(::close(fd) == -1)
    {
        // 소멸자는 예외 처리 없이 종료해야 함
        std::cerr << "Failed to close I2C device\n";
    }
}

void I2CBus::write_register(
    std::uint8_t device_address,
    std::uint8_t register_address,
    std::uint8_t value
)
{

    std::uint8_t data[2]{
        register_address,
        value
    };

    // 1. i2c messgae 생성
    i2c_msg message{};
    message.addr = device_address;
    message.flags = 0; //write
    message.len = sizeof(data);
    message.buf = data;

    // 2. transaction 생성
    i2c_rdwr_ioctl_data transaction{};
    transaction.msgs = &message;
    transaction.nmsgs = 1;

    // 3. request: I2C_RDWR
    if(::ioctl(m_fd, I2C_RDWR, &transaction) == -1)
    {
        // 예외 처리
        throw std::runtime_error("I2CBus::write_register: I2C transaction failed");
    }
}

// I2CBus::read_registers 재활용
std::uint8_t I2CBus::read_register(
    std::uint8_t device_address,
    std::uint8_t register_address
)
{
    std::uint8_t rst{};

    read_registers(
        device_address,
        register_address,
        &rst,
        1
    );

    return rst;
}

void I2CBus::read_registers(
    std::uint8_t device_address,
    std::uint8_t start_register,
    std::uint8_t* buffer,
    std::size_t length
)
{
    // 사용자 방어 코드
    if (buffer == nullptr)
    {
        // 예외 처리
        throw std::invalid_argument("I2CBus::read_registers: buffer is nullptr");
    }

    if (length == 0)
    {
        // 예외 처리
        throw std::invalid_argument("I2CBus::read_registers: length must be greater than zero");
    }

    i2c_msg messages[2] {};

    // 첫 번째 메시지: 읽기 시작할 레지스터 주소 전송
    messages[0].addr = device_address;
    messages[0].flags = 0; //write
    messages[0].len = 1;
    messages[0].buf = &start_register;

    // 두 번째 메시지: 해당 레지스터부터 데이터 읽기
    messages[1].addr = device_address;
    messages[1].flags = I2C_M_RD; // read
    messages[1].len = static_cast<__u16>(length);
    messages[1].buf = buffer;

    i2c_rdwr_ioctl_data transaction{};
    transaction.msgs = messages;
    transaction.nmsgs = 2;

    if(::ioctl(m_fd, I2C_RDWR, &transaction) == -1)
    {
        // 예외 처리
        throw std::runtime_error("I2CBus::read_registers: I2C transaction failed");
    }

}