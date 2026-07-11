//ADXL345와 통신하는 클래스
struct Acceleration{
    double x{};
    double y{};
    double z{};
};

class Adxl345Driver
{
public:

    bool init();

    Acceleration read();

    void shutdown();
};