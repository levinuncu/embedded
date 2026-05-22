import struct

data = bytearray([
    0x01, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x04, 0x05, 0x06, 0x00,
    0x07, 0x00, 0x08, 0x00,
    0x09, 0x00, 0x0a, 0x0b,
    0x00, 0x00, 0x00, 0x00,
    0x0c, 0x00, 0x00, 0x00,
    0x0d, 0x00, 0x00, 0x00,
    0x0e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x0f, 0x10, 0x11, 0x00,
    0x12, 0x00, 0x13, 0x00,
    0x14, 0x00, 0x15, 0x16,
    0x00, 0x00, 0x00, 0x00
])

def main():
    if (len(data) < 32) or (len(data) % 32 != 0) or (len(data) > 224):
        print(f"Ungueltiges Paket ({len(data)} Bytes): {data.hex(",")}")
        return

    number_of_readings = int(len(data) / 32);

    for i in range(number_of_readings):
        reading = data[i*32:]

        # GNSS
        longitude, latitude, timestamp = struct.unpack("<LLQ", reading[:16])
        # IMU
        acc_x, acc_y, acc_z, _, gyro_x, gyro_y, gyro_z = struct.unpack("<bbbbhhh", reading[16:26])
        # Temperature
        humidity, temperature = struct.unpack("<Bb", reading[26:28])

        print(f"Reading {i+1}")
        print(f"longitude={longitude}, latitude={latitude}, timestamp={timestamp}")
        print(f"acc_x={acc_x}, acc_y={acc_y}, acc_z={acc_z}")
        print(f"gyro_x={gyro_x}, gyro_y={gyro_y}, gyro_z={gyro_z}")
        print(f"humidity={humidity}, temperature={temperature}")
        print("-------------------------------------")

main();

