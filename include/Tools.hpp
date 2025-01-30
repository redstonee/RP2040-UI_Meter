#include <cstdint>

namespace Tools
{
    /**
     * @brief Calculate the sum of an byte (uint8_t) array by XOR
     *
     * @param data A pointer to the byte array
     * @param len The length of the array
     * @return uint8_t sum
     */
    uint8_t calcSum(const uint8_t *data, const std::size_t len)
    {
        uint8_t sum = 0;
        for (std::size_t i = 0; i < len; i++)
        {
            sum ^= *(data++);
        }
        return sum;
    }

} // namespace Tools
