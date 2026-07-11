//ADXL345와 통신하는 클래스
#include "./common/Msgs.hpp"

class Adxl345Driver
{
public:

    bool init();

    Acceleration read();

    void shutdown();
};